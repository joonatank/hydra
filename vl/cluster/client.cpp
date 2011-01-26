/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "client.hpp"

#include <sstream>

#include <iostream>

vl::cluster::Client::Client( char const *hostname, uint16_t port )
	: _io_service(), _socket( _io_service ), _receiver_endpoint()
{
	std::stringstream ss;
	ss << port;
	boost::udp::resolver resolver( _io_service );
	boost::udp::resolver::query query( boost::udp::v4(), hostname, ss.str().c_str() );
	_receiver_endpoint = *resolver.resolve(query);

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
		// TODO handle large messages
		std::vector<char> recv_buf(512);
		boost::system::error_code error;

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_receiver_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		// TODO some constraints for the number of messages
		if( _messages.size() > 1024 )
		{
			std::cerr << "Message stack full, cleaning out the oldest." << std::endl;
			while( _messages.size() > 1024 )
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
	std::cout << "sending data" << std::endl;
	_socket.send_to( boost::asio::buffer(buf), _receiver_endpoint );
}

/// ------------------------ Private -------------------------------------------
