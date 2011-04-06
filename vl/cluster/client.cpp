/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file cluster/client.cpp
 */

#include "client.hpp"

#include "base/exceptions.hpp"

#include "distrib_settings.hpp"

#include "logger.hpp"

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
	// TODO should set callbacks to Renderer
	vl::MsgCallback *cb = new ClientMsgCallback(this);
	_renderer->setSendMessageCB(cb);
	_callbacks.push_back(cb);

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
	Message msg( MSG_REG_UPDATES );
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
		std::vector<char> recv_buf( _socket.available() );
		boost::system::error_code error;

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_master, 0, error );

		/// TOOD when these do happen?
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
			Message msg( recv_buf );
			sendAck(msg.getType());
			_handleMessage(msg);
		}
	}

	// Request updates
	// Uses the timer so that the request is sent only so often
	// TODO replace this with a more general purpose send till received
	// blocking function
	if( !_state.environment && double(_request_timer.elapsed()) > 0.1 )
	{
		Message msg( MSG_REG_UPDATES );
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

void
vl::cluster::Client::sendAck(vl::cluster::MSG_TYPES type)
{
	Message msg( vl::cluster::MSG_ACK );
	msg.write(type);
	sendMessage(msg);
}

/// ------------------------ Private -------------------------------------------
void
vl::cluster::Client::_handleMessage(vl::cluster::Message &msg)
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
				vl::SettingsByteData data;
				data.copyFromMessage(&msg);
				vl::cluster::ByteStream stream(&data);
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
			Message msg(MSG_SG_UPDATE_READY);
			assert( !_state.rendering );
			assert( _state.wants_render );
			_state.rendering = true;
			_state.set_rendering_state(CS_UPDATE_READY);
			sendMessage(msg);
		}
		break;

		case vl::cluster::MSG_SG_UPDATE :
		{
			assert(_renderer.get());
			_state.set_rendering_state(CS_UPDATE);
			_renderer->handleMessage(msg);
			Message msg(MSG_DRAW_READY);
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
			Message msg(MSG_DRAW_DONE);
			sendMessage(msg);
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
				Message msg(MSG_REG_RENDERING);
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
