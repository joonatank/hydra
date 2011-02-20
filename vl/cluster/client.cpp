/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "client.hpp"

#include "base/exceptions.hpp"

#include <sstream>

#include <iostream>

const size_t MSG_BUFFER_SIZE = 128;

vl::cluster::Client::Client( char const *hostname, uint16_t port )
	: _io_service()
	, _socket( _io_service )
	, _master()
	, _state(CS_UNDEFINED)
{
	std::cout << "vl::cluster::Client::Client : Connecting to host " 
		<< hostname << " at port " << port << "." << std::endl;

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
}

vl::cluster::Client::~Client( void )
{
	std::cout << "vl::cluster::Client::~Client" << std::endl;
}

void
vl::cluster::Client::mainloop( void )
{
	if( _socket.available() != 0 )
	{
		std::vector<char> recv_buf( _socket.available() );
		boost::system::error_code error;

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_master, 0, error );

		if( error && error == boost::asio::error::connection_refused )
		{
			std::cout << "Error : Connection refused" << std::endl;
		}
		else if( error && error == boost::asio::error::connection_aborted )
		{
			std::cout << "Error : Connection aborted" << std::endl;
		}
		else if( error && error == boost::asio::error::connection_reset )
		{
			// This is received if there is no service in the port we sent
		}
		else if( error && error == boost::asio::error::host_unreachable )
		{
			std::cout << "Error : Host unreachable" << std::endl;
		}
		else if( error && error != boost::asio::error::message_size )
		{ throw boost::system::system_error(error); }

		if( n > 0 )
		{
			// Some constraints for the number of messages
			if( _messages.size() > MSG_BUFFER_SIZE )
			{
				std::cerr << "Message stack full, cleaning out the oldest." << std::endl;
				while( _messages.size() > MSG_BUFFER_SIZE )
				{ delete popMessage(); }
			}

			Message *msg = new Message( recv_buf );
			_messages.push_back(msg);
			if( msg->getType() == vl::cluster::MSG_ENVIRONMENT )
			{
				std::cout << "Changing client state" << std::endl;
				assert( _state == CS_REQ );
				_state = CS_ENV;
			}
		}
	}

	// Request updates
	// Uses the timer so that the request is sent only so often
	if( _state == CS_REQ  && _request_timer.getMilliseconds() > 100 )
	{
		_sentRequestUpdates();
		_request_timer.reset();
	}
}

void
vl::cluster::Client::registerForUpdates( void )
{
	std::cout << "vl::cluster::Client::registerForUpdates" << std::endl;
	assert( _state == CS_UNDEFINED );
	if( _state == CS_UNDEFINED )
	{ _state = CS_REQ; }
}

vl::cluster::Message *
vl::cluster::Client::popMessage( void )
{
	Message *tmp = _messages.front();
	_messages.erase( _messages.begin() );
	return tmp;
}

void
vl::cluster::Client::sendMessage( vl::cluster::Message *msg )
{
	std::vector<char> buf;
	msg->dump(buf);
// 	std::cout << "sending data" << std::endl;
	_socket.send_to( boost::asio::buffer(buf), _master );
}

void
vl::cluster::Client::sendAck ( vl::cluster::MSG_TYPES type )
{
	Message msg( vl::cluster::MSG_ACK );
	msg.write(type);
	sendMessage(&msg);
}

/// ------------------------ Private -------------------------------------------
void 
vl::cluster::Client::_sentRequestUpdates( void )
{
	Message msg( MSG_REG_UPDATES );
	std::vector<char> buf;
	msg.dump(buf);
	_socket.connect( _master );
	_socket.send_to( boost::asio::buffer(buf), _master);
}
