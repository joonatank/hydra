/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluster/client.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "client.hpp"

#include "base/exceptions.hpp"
#include "base/sleep.hpp"

#include "distrib_settings.hpp"

#include "logger.hpp"

/// Necessary because we create MeshManager here and set it to Renderer
#include "mesh_manager.hpp"
/// Necessary for parsing the settings structure from a message
#include "settings.hpp"

#include <sstream>
#include <iostream>

const size_t MSG_BUFFER_SIZE = 128;

/// ------------------------------ Callbacks ---------------------------------

vl::cluster::SlaveMeshLoaderCallback::SlaveMeshLoaderCallback(vl::cluster::Client *own)
	: owner(own)
{}

void
vl::cluster::SlaveMeshLoaderCallback::loadMesh(std::string const &fileName, vl::MeshLoadedCallback *cb)
{
	std::cout << vl::TRACE << "vl::cluster::SlaveMeshLoaderCallback::loadMesh : " << fileName << std::endl;
	if(!owner)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer());
	}
	assert(cb);

	/// @todo fix time and frame parameters
	Message reg_msg(MSG_REG_RESOURCE, 0, vl::time());
	reg_msg.write(RES_MESH);
	reg_msg.write(fileName);
	owner->sendMessage(reg_msg);
}

vl::cluster::ClientMessageCallback::ClientMessageCallback(Client *c)
	: client(c)
{ assert(client); }

vl::cluster::ResourceMessageCallback::ResourceMessageCallback(Client *client)
	: ClientMessageCallback(client)
{}

void
vl::cluster::ResourceMessageCallback::messageReceived(MessageRefPtr msg)
{
	assert(!msg->empty());

	MessageDataStream stream = msg->getStream();
	RESOURCE_TYPE type;
	std::string name;
	stream >> type >> name;
	std::cout << vl::TRACE << "Resource type = " << type << " : name = " << name << std::endl;

	if(type == RES_MESH)
	{
		vl::MeshRefPtr mesh(new vl::Mesh(name));
		stream >> (*mesh);

		assert(client && client->getMeshManager());
		client->getMeshManager()->meshLoaded(name, mesh);
	}
	else
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Unknow Resource type.")); }
}

/// ------------------------------ Client -------------------------------------
vl::cluster::Client::Client( char const *hostname, uint16_t port,
							 vl::RendererUniquePtr rend )
	: _io_service()
	, _socket( _io_service )
	, _master()
	, _state()
	, _renderer(rend)
{
	std::cout << "vl::cluster::Client::Client : Connecting to host "
		<< hostname << " at port " << port << "." << std::endl;

	assert(_renderer.get());
	
	// set callbacks to Renderer
	vl::MeshLoaderCallback *mesh_cb = new SlaveMeshLoaderCallback(this);
	vl::MeshManagerRefPtr mesh_man(new MeshManager(mesh_cb));
	_renderer->setMeshManager(mesh_man);
	addMessageCallback(MSG_RESOURCE, new ResourceMessageCallback(this));

	std::stringstream ss;
	ss << port;
	boost::udp::resolver resolver( _io_service );
	boost::udp::resolver::query query( boost::udp::v4(), hostname, ss.str().c_str(), 
									   boost::udp::resolver_query::all_matching);
	_master = *resolver.resolve(query);

	_socket.open( boost::udp::v4() );

	// Large receive buffer is necessary for receiving multiple messages
	// have a look at Server for more information.
	// Really huge buffer (24Mbytes)
	// because if the server sends update messages back to back we would
	// overload the buffer.
	// @todo This needs a more elegant solution for clients that lag for more 
	// than few seconds e.g. if we are missing few hundred updates then we 
	// send the whole graph to the client not the individual update messages.
	boost::asio::socket_base::receive_buffer_size rec_buf_size(24*1024*1024);
	boost::asio::socket_base::send_buffer_size send_buf_size;
	_socket.set_option(rec_buf_size);
	_socket.get_option(send_buf_size);

	std::cout << "Receive Buffer size = " << rec_buf_size.value() << "." 
		<< " Send buffer size = " << send_buf_size.value() << "." << std::endl;

	// Needs to sent the request as soon as possible
	assert( !_state.environment );
	Message msg( MSG_REG_UPDATES, 0, 0 );
	sendMessage(msg);
	_request_timer.reset();
}

vl::cluster::Client::~Client( void )
{
	for( std::vector<vl::Callback *>::iterator iter = _callbacks.begin();
		iter != _callbacks.end(); ++iter )
	{ delete *iter; }
}

bool
vl::cluster::Client::isRunning(void)
{
	return !_state.shutdown;
}

bool
vl::cluster::Client::isRendering(void)
{
	return _state.is_rendering();
}

void
vl::cluster::Client::mainloop(void)
{
	while(_socket.available())
	{
		MessageRefPtr msg = _receive();
		if(msg)
		{
			_handle_message(*msg);
		}
	}

	// Request updates
	// Uses the timer so that the request is sent only so often
	// TODO replace this with a more general purpose send till received
	// blocking function
	if( !_state.environment && double(_request_timer.elapsed()) > 0.1 )
	{
		std::cout << vl::TRACE << "Sending another MSG_REG_UPDATES" << std::endl;
		Message msg( MSG_REG_UPDATES, 0, vl::time() );
		sendMessage(msg);
		_request_timer.reset();
	}
}

void
vl::cluster::Client::sendMessage(vl::cluster::Message const &msg)
{
	std::vector<char> buf;
	std::vector<MessagePart> parts = msg.createParts();

	for(size_t i = 0; i < parts.size(); ++i)
	{
		parts.at(i).dump(buf);
		_socket.send_to(boost::asio::buffer(buf), _master);
	}
}

vl::cluster::MessageRefPtr
vl::cluster::Client::waitForMessage(vl::cluster::MSG_TYPES type, vl::time timelimit)
{
	std::cout << vl::TRACE << "vl::cluster::Client::waitForMessage" << std::endl;

	MessageRefPtr msg;
	bool done = false;
	while(!done)
	{
		msg = _receive();
		if(msg && MSG_UNDEFINED != msg->getType())
		{
			if(msg->getType() == type)
			{
				done = true;
				/// Not calling handle because the message was requested from
				/// somewhere
			}
			else
			{
				_handle_message(*msg);
			}
		}
		/// @todo does this need sleeping between messages?
		/// probably necessary on Linux
		vl::msleep(uint32_t(0));
	}

	return msg;
}

/// ------------------------ Private -------------------------------------------
void
vl::cluster::Client::_handle_message(vl::cluster::Message &msg)
{
	switch( msg.getType() )
	{
		case vl::cluster::MSG_ENVIRONMENT:
		{
			/// Single environment supported
			if( !_state.environment )
			{
				std::clog << "vl::cluster::Client::_handleMessage : "
					<< "MSG_ENVIRONMENT received." << std::endl;
				_state.environment = true;
				assert(_renderer.get());

				// Deserialize the environment settings
				vl::config::EnvSettingsRefPtr env(new vl::config::EnvSettings);
				// TODO needs a ByteData object for Environment settings
				// @todo this should be changed to use MessageStream instead
				// so no copying is necessary.
				vl::SettingsByteData data;
				data.copyFromMessage(&msg);
				vl::cluster::ByteDataStream stream(&data);
				stream >> env;
				// Only single environment settings should be in the message
				assert( 0 == msg.size() );
				_renderer->init(env);
			}
			else
			{
				std::clog << "vl::cluster::Client::_handleMessage : "
					<< "MSG_ENVIRONMENT received more than once." << std::endl;
			}
		}
		break;

		case vl::cluster::MSG_SHUTDOWN:
		{
			std::clog << "vl::cluster::Client::_handleMessage : MSG_SHUTDOWN received" << std::endl;
			_state.shutdown = true;
			_renderer.reset();
		}
		break;

		case vl::cluster::MSG_INJECT_LAG :
		{
			vl::time lag;
			msg.read(lag);
			vl::sleep(lag);
		}
		break;

		case vl::cluster::MSG_FRAME_START :
		{
			assert(_state.environment);
			assert(_state.project);
			assert(_state.has_init);
			/// @todo add checking for the server timestamp
			// Not necessary for anything, should restart the frame though
			// Oh might be also that we are getting messages in the wrong order, which would need timestamp support
			assert( _state.wants_render );
			_state.frame = msg.getFrame();
			_state.set_rendering_state(CS_UPDATE_READY);

			Message reply(MSG_REQ_SG_UPDATE, _state.update_frame, vl::time());
			sendMessage(reply);
		}
		break;

		case vl::cluster::MSG_SG_INIT :
		{
			std::clog << "vl::cluster::Client::_handleMessage : "
					<< "MSG_SG_INIT received." << std::endl;
			assert(!_state.has_init);
			
			uint32_t frame = msg.getFrame();
			_renderer->updateScene(msg);

			_state.has_init = true;
			_state.update_frame = frame;

			// request rendering messages when initialised
			_state.wants_render = true;
			Message reply(MSG_REG_RENDERING, frame, vl::time());
			sendMessage(reply);
		}
		break;

		/// Add a new update to the stack to be applied
		/// If we have all updates required for the next frame proceed
		/// with the rendering loop by informing master that we are ready.
		/// Updates can come in any order but all of them are required for
		/// proceeding with the drawing of next frame.
		case vl::cluster::MSG_SG_UPDATE :
		{
			// @todo this is really error prone
			// Update shouldn't depend on rendering state.
			// @this can be removed later when we rearrange the rendering loop
			if(!_state.is_rendering())
			{
				std::stringstream ss;
				ss << "Client : Something really weird we are neither "
					<< "in the rendering loop nor waiting for init." << std::endl;
				BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
				
			}

			// @todo this client state should be removed
			if(!_state.has_rendering_state(CS_UPDATE_READY))
			{
				std::stringstream ss;
				ss << "Client should update though it has invalid state." << std::endl;
				BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
			}

			if(_update_messages.find(msg.getFrame()) != _update_messages.end())
			{
				std::stringstream ss;
				ss << "Already has a update message for " << msg.getFrame();
				BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
			}
			_update_messages[msg.getFrame()] = msg;

			// We only sent the DRAW_READY message if we are ready for draw
			// i.e. we have all messages between update_frame and frame (server/draw frame)
			//
			// @todo this is problematic if we lag for long while because we will
			// miss some of the update messages (the receive buffer is not large enough).
			// This has been somewhat worked around by using huge receive buffer.
			if(_state.update_frame + _update_messages.size() == _state.frame)
			{
				// Update state for the next message
				_state.update_frame = (--_update_messages.end())->first;

				_state.set_rendering_state(CS_UPDATE);
				/// Sending DRAW_READY from here always will cause multiple incorrect state 
				/// changes in the Server.
				/// This happens when we need more than one update message.
				/// This does not cause functional errors, but introduces lot of error printing.
				/// Fixing this should probably be done in the Server side or
				/// we need to separate update from drawing.
				Message reply(MSG_DRAW_READY, _state.frame, vl::time());
				sendMessage(reply);
			}
			/* @todo there should be a verbose argument in config (ini) for this print
			else
			{
				size_t msgs_missing = _state.frame - (_state.update_frame + _update_messages.size());
				std::clog << "Waiting for " << msgs_missing << "We have " 
					<< _update_messages.size() << " messages." << std::endl;
			}
			*/
		}
		break;

		case vl::cluster::MSG_DRAW :
		{
			uint32_t frame = 0;
			if(_state.is_rendering())
			{	
				if(_state.frame == msg.getFrame())
				{
					if(!_state.has_rendering_state(CS_UPDATE))
					{ std::clog << "Client should start draw though it has invalid state." << std::endl; }
					
					frame = _state.frame;
					_state.set_rendering_state(CS_DRAW);
				}
				else
				{
					std::clog << "Incorrect frame number for MSG_DRAW : local frame = " 
						<< _state.frame << " : remote frame = " << msg.getFrame() << std::endl;
					_state.clear_rendering_state();
				}

				Message reply(MSG_DRAWING, frame, 0);
				sendMessage(reply);
			}

			if(_state.is_rendering())
			{
				// Apply all updates in correct order
				assert(_state.environment);
				assert(_state.project);
				assert(_renderer.get());
				
				// Map has all the updates in correct order already
				// from the oldest to the newest.
				// Previous step already guaranties that we have all the update messages
				// because we only proceed here if we have them all.
				// @todo this also creates huge lag in start even with simple 
				// models which is rather odd.
				for(std::map<uint32_t, Message>::iterator iter = _update_messages.begin();
					iter != _update_messages.end(); ++iter)
				{
					_renderer->updateScene(iter->second);
				}
				// clear the update messages after applying them
				_update_messages.clear();
				


				// Start rendering

				if(!_state.has_rendering_state(CS_DRAW))
				{ std::clog << "Client should draw though it has invalid state." << std::endl; }

				_renderer->draw();
				_renderer->swap();
				_renderer->capture();

				// Done
				_state.clear_rendering_state();

				/// Removed printing of timing stats, they are incorrect anyway
				/// because they measure draw calls which are async functions.

				/// Reply
				Message reply = Message(MSG_DRAW_DONE, frame, 0);
				sendMessage(reply);
			}
		}
		break;

		case vl::cluster::MSG_ACK:
			break;

		case vl::cluster::MSG_PROJECT :
		{
			/// Only single project is supported, discards the rest
			if( !_state.project )
			{
				std::clog << "vl::cluster::Client::_handleMessage : "
					<< "MSG_PROJECT received." << std::endl;
				_state.project = true;

				/// @todo replace ByteDataStream with MessageStream, reduces copying
				vl::SettingsByteData data;
				data.copyFromMessage(&msg);
				vl::cluster::ByteDataStream stream(&data);
				vl::Settings settings;
				stream >> settings;

				assert(_renderer.get());
				_renderer->setProject(settings);
			}
			else
			{
				std::clog << "Client::_handle_message : Project received twice." << std::endl;
			}
		}
		break;

		case vl::cluster::MSG_UNDEFINED :
			std::cout << vl::CRITICAL << "Undefined message should never be processed." << std::endl;
			break;

		case vl::cluster::MSG_SG_CREATE :
			std::clog << "Client : MSG_SG_CREATE Received." << std::endl;
			assert(_renderer.get());
			_renderer->createSceneObjects(msg);
			
			break;
		
		default:
		{
			std::cout << vl::CRITICAL << "Passing message with type = " << msg.getType()
				<< " to renderer because it has no handler." << std::endl;
		}
		break;
	}
}

void
vl::cluster::Client::_send_ack(vl::cluster::MSG_TYPES type)
{
	Message msg( vl::cluster::MSG_ACK, 0, vl::time() );
	msg.write(type);
	sendMessage(msg);
}

void 
vl::cluster::Client::addMessageCallback(MSG_TYPES type, ClientMessageCallback *cb)
{
	/// Only Single callback per type is allowed
	assert(_msg_callbacks.find(type) == _msg_callbacks.end());

	_msg_callbacks[type] = cb;
}

vl::cluster::MessageRefPtr 
vl::cluster::Client::_receive(void)
{
	MessageRefPtr msg;

	/// @todo problematic here is that we can not receive more than one
	/// complete message at a time
	/// but if we don't get all the messages at once and add them to a
	/// received stack in the order they are received (or have been sent)
	/// we are using newer messages instead of the older.
	while(_socket.available() && !msg)
	{
		std::vector<char> recv_buf(_socket.available());
		boost::system::error_code error;

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_master, 0, error );

		/// @TODO when these do happen?
		if( error && error == boost::asio::error::connection_refused )
		{
			std::clog << "Error : Connection refused" << std::endl;
		}
		else if( error && error == boost::asio::error::connection_aborted )
		{
			std::clog << "Error : Connection aborted" << std::endl;
		}
		else if( error && error == boost::asio::error::connection_reset )
		{
			// This is received if there is no service in the port we sent
		}
		else if( error && error == boost::asio::error::host_unreachable )
		{
			std::clog << "Error : Host unreachable" << std::endl;
		}
		else if( error && error != boost::asio::error::message_size )
		{ throw boost::system::system_error(error); }

		if( n > 0 )
		{
			MessagePart part(recv_buf);
			// @todo send id and part number also
			_send_ack(part.type);
			if(part.parts == 1)
			{
				msg.reset(new Message(part));
			}
			else
			{
				bool consumed = false;
				for(size_t i = 0; i < _partial_messages.size(); ++i)
				{
					MessageRefPtr p_m = _partial_messages.at(i);
					if(p_m->getType() == part.type && p_m->getID() == part.id)
					{
						p_m->addPart(part);
						consumed = true;
						if( !p_m->partial() )
						{
							msg = p_m;
							_partial_messages.erase(_partial_messages.begin()+i);
						}
						break;
					}
				}
				if(!consumed)
				{
					MessageRefPtr p_m(new Message(part));
					_partial_messages.push_back(p_m);
				}
			}
		}
	}

	if(msg)
	{
		std::map<MSG_TYPES, ClientMessageCallback *>::iterator iter 
			= _msg_callbacks.find(msg->getType());
		if(iter != _msg_callbacks.end())
		{
			iter->second->messageReceived(msg);
			msg.reset();
		}
	}

	return msg;
}
