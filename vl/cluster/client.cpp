/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "client.hpp"

#include <sstream>

#include <iostream>

const size_t MSG_BUFFER_SIZE = 128;

vl::cluster::Client::Client( char const *hostname, uint16_t port )
	: _io_service(), _socket( _io_service ), _master()
{
	std::stringstream ss;
	ss << port;
	boost::udp::resolver resolver( _io_service );
	boost::udp::resolver::query query( boost::udp::v4(), hostname, ss.str().c_str() );
	_master = *resolver.resolve(query);

	_socket.open( boost::udp::v4() );
}

vl::cluster::Client::~Client( void )
{

}

void
vl::cluster::Client::mainloop( void )
{
// 	std::cout << "vl::cluster::Client::mainloop" << std::endl;
	if( _socket.available() != 0 )
	{
// 		std::cout << "vl::cluster::Client::mainloop has a message" << std::endl;
		std::vector<char> recv_buf( _socket.available() );
		boost::system::error_code error;

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_master, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		// Some constraints for the number of messages
		// TODO this should be configurable and be reasonable low
		if( _messages.size() > MSG_BUFFER_SIZE )
		{
			std::cerr << "Message stack full, cleaning out the oldest." << std::endl;
			while( _messages.size() > MSG_BUFFER_SIZE )
			{ delete popMessage(); }
		}

		Message *msg = new Message( recv_buf );
		_messages.push_back(msg);
	}
}

void
vl::cluster::Client::registerForUpdates( void )
{
	std::cout << "vl::cluster::Client::registerForUpdates" << std::endl;
	Message msg( MSG_REG_UPDATES );
	std::vector<char> buf;
	msg.dump(buf);
// 	std::cout << "sending data" << std::endl;
	_socket.send_to( boost::asio::buffer(buf), _master);
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

/// ------------------------ Private -------------------------------------------
