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
	// TODO handle large messages
	if( _socket.available() != 0 )
	{
		std::cout << "vl::cluster::Client::mainloop has a message" << std::endl;
		std::vector<char> recv_buf(512);
		boost::system::error_code error;
		if( _socket.available() == 0 )
		{ return; }

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_receiver_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		handle( recv_buf );
	}
}

// void
// vl::cluster::Client::send(std::vector<double> const & msg)
// {
// 	if( msg.size() == 0 )
// 	{ return; }
//
// 	size_t n = socket.send_to(boost::asio::buffer(msg), receiver_endpoint);
// }

void
vl::cluster::Client::registerForUpdates( void )
{
// 	Message
// 	_receiver_endpoint.send_to( boost::asio::buffer(msg)
}

// void
// vl::cluster::Client::registerMessage( vl::cluster::Message const &message,
// 									  vl::MessageCallback *callback )
// {
//
// }

/// ------------------------ Private -------------------------------------------
void
vl::cluster::Client::handle( std::vector< char >& datagram )
{

}
