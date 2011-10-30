/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file cluster/client.cpp
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
vl::cluster::ClientMsgCallback::ClientMsgCallback(vl::cluster::Client *own)
	: owner(own)
{
	assert(owner);
}

void
vl::cluster::ClientMsgCallback::operator()(vl::cluster::Message const &msg)
{
	assert(owner);
	owner->sendMessage(msg);
}

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
	vl::MsgCallback *cb = new ClientMsgCallback(this);
	_renderer->setSendMessageCB(cb);
	_callbacks.push_back(cb);

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
	boost::asio::socket_base::receive_buffer_size rec_buf_size(1024*1024);
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
		vl::msleep(0);
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
			std::cout << vl::TRACE << "vl::cluster::Client::_handleMessage : MSG_ENVIRONMENT received" << std::endl;
			/// Single environment supported
			if( !_state.environment )
			{
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
		}
		break;

		case vl::cluster::MSG_SHUTDOWN:
		{
			std::cout << vl::TRACE << "vl::cluster::Client::_handleMessage : MSG_SHUTDOWN received" << std::endl;
			_state.shutdown = true;
			_renderer.reset();
		}
		break;

		case vl::cluster::MSG_FRAME_START :
		{
			assert(_state.environment);
			assert(_state.project);
			/// @todo add checking for the server timestamp
			// Not necessary for anything, should restart the frame though
			// Oh might be also that we are getting messages in the wrong order, which would need timestamp support
			//assert( !_state.rendering );
			assert( _state.wants_render );
			//_state.rendering = true;
			_state.frame = msg.getFrame();
			_state.set_rendering_state(CS_UPDATE_READY);

			Message reply(MSG_SG_UPDATE_READY, msg.getFrame(), vl::time());
			sendMessage(reply);
		}
		break;

		case vl::cluster::MSG_SG_INIT :
			_renderer->initScene(msg);
			_state.has_init = true;
			/// @todo add reply
			break;
		
		case vl::cluster::MSG_SG_UPDATE :
		{
			uint32_t frame = 0;
			if(_state.is_rendering())
			{
				assert(_state.environment);
				assert(_state.project);
				assert(_renderer.get());
				// Send back either the current frame if valid or 0 for errors
				
				if(_state.frame == msg.getFrame())
				{
					if(!_state.has_rendering_state(CS_UPDATE_READY))
					{ std::cout << vl::CRITICAL << "Client should update though it has invalid state." << std::endl; }

					frame = _state.frame;
					_state.set_rendering_state(CS_UPDATE);
					_renderer->updateScene(msg);
				}
				else
				{
					std::cout << vl::TRACE << "Incorrect frame number to for MSG_SG_UPDATE : local frame = " 
						<< _state.frame << " : remote frame = " << msg.getFrame() << std::endl;
					_state.clear_rendering_state();
				}
			}
			else if(!_state.has_init)
			{
				_renderer->initScene(msg);
				_state.has_init = true;
			}

			Message reply(MSG_DRAW_READY, frame, vl::time());
			sendMessage(reply);
		}
		break;

		case vl::cluster::MSG_DRAW :
		{
			uint32_t frame = 0;
			if(_state.is_rendering())
			{
				assert(_state.environment);
				assert(_state.project);
				/// @todo move drawing to occure only if all the states required are
				/// set, or should we?
				assert(_renderer.get());
				
				if(_state.frame == msg.getFrame())
				{
					if(!_state.has_rendering_state(CS_UPDATE))
					{ std::cout << vl::CRITICAL << "Client should render though it has invalid state." << std::endl; }
					
					frame = _state.frame;
					_state.set_rendering_state(CS_DRAW);
				}
				else
				{
					std::cout << vl::TRACE << "Incorrect frame number for MSG_DRAW : local frame = " 
						<< _state.frame << " : remote frame = " << msg.getFrame() << std::endl;
					_state.clear_rendering_state();
				}
			}
			Message reply(MSG_DRAWING, frame, 0);
			sendMessage(reply);

			if(_state.is_rendering())
			{
				if(!_state.has_rendering_state(CS_DRAW))
				{ std::cout << vl::CRITICAL << "Client should render though it has invalid state." << std::endl; }

				_renderer->draw();
				_renderer->swap();
				_renderer->capture();

				// Done
				_state.clear_rendering_state();

				/// Removed printing of timing stats, they are incorrect anyway
				/// because they measure draw calls which are async functions.

				/// Reply
				reply = Message(MSG_DRAW_DONE, frame, 0);
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
				_state.project = true;

				/// @todo replace ByteDataStream with MessageStream, reduces copying
				vl::SettingsByteData data;
				data.copyFromMessage(&msg);
				vl::cluster::ByteDataStream stream(&data);
				vl::Settings settings;
				stream >> settings;

				assert(_renderer.get());
				_renderer->setProject(settings);

				// request rendering messages when initialised
				Message reply(MSG_REG_RENDERING, 0, vl::time());
				_state.wants_render = true;
				sendMessage(reply);
			}
		}
		break;

		case vl::cluster::MSG_UNDEFINED :
			std::cout << vl::CRITICAL << "Undefined message should never be processed." << std::endl;
			break;

		case vl::cluster::MSG_SG_CREATE :
			assert(_renderer.get());
			_renderer->createSceneObjects(msg);
			break;
		
		default:
		{
			std::cout << vl::CRITICAL << "Passing message with type = " << msg.getType()
				<< " to renderer because it has no handler." << std::endl;
//			assert(_renderer.get());
//			_renderer->handleMessage(msg);
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
			std::cout << vl::CRITICAL << "Error : Connection refused" << std::endl;
		}
		else if( error && error == boost::asio::error::connection_aborted )
		{
			std::cout << vl::CRITICAL << "Error : Connection aborted" << std::endl;
		}
		else if( error && error == boost::asio::error::connection_reset )
		{
			// This is received if there is no service in the port we sent
		}
		else if( error && error == boost::asio::error::host_unreachable )
		{
			std::cout << vl::CRITICAL << "Error : Host unreachable" << std::endl;
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
							std::cout << vl::TRACE << "Partial message with type = " 
								<< getTypeAsString(p_m->getType()) << " id = " << p_m->getID()
								<< " made whole." << std::endl;
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
			std::cout << vl::TRACE << "Callback found for message type : " << getTypeAsString(msg->getType())
				<< std::endl;
			iter->second->messageReceived(msg);
			msg.reset();
		}
	}

	return msg;
}
