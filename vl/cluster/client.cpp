/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file cluster/client.cpp
 */

#include "client.hpp"

#include "base/exceptions.hpp"

#include "distrib_settings.hpp"

#include "logger.hpp"

/// Necessary because we create MeshManager here and set it to Renderer
#include "mesh_manager.hpp"

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

/// Blocks till the mesh is loaded and returns a valid mesh
vl::MeshRefPtr
vl::cluster::SlaveMeshLoaderCallback::loadMesh(std::string const &fileName)
{
	std::clog << "vl::cluster::SlaveMeshLoaderCallback::loadMesh : " << fileName << std::endl;
	if(!owner)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer());
	}

//	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/// @todo fix time and frame parameters
	Message reg_msg(MSG_REG_RESOURCE, 0, vl::time());
	reg_msg.write(RES_MESH);
	reg_msg.write(fileName);
	owner->sendMessage(reg_msg);

	/// @todo it's not enough that the type of the Message matches
	/// for blocking functions this might not be a problem but for non-blocking
	/// this is a disaster.
	MessageRefPtr msg = owner->waitForMessage(MSG_RESOURCE);
	assert(!msg->empty());

	vl::MeshRefPtr mesh(new vl::Mesh);
	MessageStream stream = msg->getStream();
	RESOURCE_TYPE type;
	std::string name;
	stream >> type >> name;
	std::cout << "Resource type = " << type << " : name = " << name << std::endl;
	assert(type == RES_MESH);
	assert(name == fileName);
	stream >> (*mesh);

	std::clog << "vl::cluster::SlaveMeshLoaderCallback::loadMesh : got mesh data" << std::endl;

	return mesh;
}

/// ------------------------------ Client -------------------------------------
vl::cluster::Client::Client( char const *hostname, uint16_t port,
							 vl::RendererInterfacePtr rend )
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

	std::stringstream ss;
	ss << port;
	boost::udp::resolver resolver( _io_service );
	boost::udp::resolver::query query( boost::udp::v4(), hostname, ss.str().c_str() );
	_master = *resolver.resolve(query);

	_socket.open( boost::udp::v4() );

	// @todo this is not a real solution but allowes us to receive large
	// messages
	// replace this by dividing the messages to multiple parts before
	// sending
	boost::asio::socket_base::receive_buffer_size buf_size(8*8192);
	_socket.set_option(buf_size);
	std::cout << "vl::cluster::Client::Client : Receive Buffer size = "
		<< buf_size.value() << "." << std::endl;

	// Needs to sent the request as soon as possible
	assert( !_state.environment );
	Message msg( MSG_REG_UPDATES, 0, 0 );
	sendMessage(msg);
	_request_timer.reset();
}

vl::cluster::Client::~Client( void )
{
	std::cout << vl::TRACE << "vl::cluster::Client::~Client" << std::endl;

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
	return _state.rendering;
}

void
vl::cluster::Client::mainloop(void)
{
	while(_socket.available())
	{
		MessageRefPtr msg = _receive();
		if(msg && MSG_UNDEFINED != msg->getType())
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
		Message msg( MSG_REG_UPDATES, 0, vl::time() );
		sendMessage(msg);
		_request_timer.reset();
	}
}

void
vl::cluster::Client::sendMessage(vl::cluster::Message const &msg)
{
	std::vector<char> buf;
	msg.dump(buf);
	_socket.send_to( boost::asio::buffer(buf), _master );
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
#ifndef _WIN32
		vl::msleep(0);
#endif
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
				_state.environment = true;
				assert(_renderer.get());

				// Deserialize the environment settings
				vl::EnvSettingsRefPtr env( new vl::EnvSettings );
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
			/// @todo add checking for the server timestamp
			Message msg(MSG_SG_UPDATE_READY, 0, vl::time());
			// Not necessary for anything, should restart the frame though
			// Oh might be also that we are getting messages in the wrong order, which would need timestamp support
			//assert( !_state.rendering );
			assert( _state.wants_render );
			_state.rendering = true;
			_rend_timer.reset();
			_state.set_rendering_state(CS_UPDATE_READY);
			sendMessage(msg);
		}
		break;

		case vl::cluster::MSG_SG_UPDATE :
		{
			assert(_renderer.get());
			_state.set_rendering_state(CS_UPDATE);
			_renderer->handleMessage(msg);
			Message msg(MSG_DRAW_READY, 0, vl::time());
			sendMessage(msg);
		}
		break;

		case vl::cluster::MSG_DRAW :
		{
			/// @todo move drawing to occure only if all the states required are
			/// set, or should we?
			_state.set_rendering_state(CS_DRAW);
			assert(_renderer.get());
			_renderer->draw();
			_renderer->swap();
			_renderer->capture();

			// Done
			_state.set_rendering_state(CS_DRAW_DONE);
			_state.rendering = false;

			/// Reply
			Message msg(MSG_DRAW_DONE, 0, vl::time());
			sendMessage(msg);
			
			/// Recond stats
			/// Before first recording should reset the print timer
			if( !_rend_report.has_number("Rendering ") )
			{ _print_timer.reset(); }
			_rend_report.get_number("Rendering ").push(_rend_timer.elapsed());

			/// Print stats every ten seconds
			if( _print_timer.elapsed() > vl::time(10) )
			{
				_rend_report.finish();
				std::clog << _rend_report << std::endl;
				_print_timer.reset();
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
				assert(_renderer.get());
				_renderer->handleMessage(msg);

				// request rendering messages when initialised
				Message msg(MSG_REG_RENDERING, 0, vl::time());
				_state.wants_render = true;
				sendMessage(msg);
			}
		}
		break;

		default:
		{
			assert(_renderer.get());
			_renderer->handleMessage(msg);
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

vl::cluster::MessageRefPtr 
vl::cluster::Client::_receive(void)
{
	MessageRefPtr msg;

	if(_socket.available())
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
			msg.reset(new Message(recv_buf));
			_send_ack(msg->getType());
		}
	}

	return msg;
}
