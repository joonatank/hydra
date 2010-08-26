#include "server.hpp"

#include <iostream>

vl::udp::Server::Server(  uint16_t const port  )
	: socket(io_service, boost::udp::endpoint(boost::udp::v4(), port))
{
}


vl::udp::Server::~Server()
{

}

void
vl::udp::Server::mainloop( void )
{
	std::vector<double> recv_buf(128);
	boost::udp::endpoint remote_endpoint;
	boost::system::error_code error;
	size_t n = socket.receive_from( boost::asio::buffer(recv_buf),
		remote_endpoint, 0, error );

	if (error && error != boost::asio::error::message_size)
	{ throw boost::system::system_error(error); }

	// Print the message gotten from client
	std::cout << "Received message from " << remote_endpoint
		<< " with " << n << " bytes." << std::endl;

	// Remove extra space from the message
	recv_buf.resize( n/sizeof(double) );
	
	if( !_handler )
	{
		return;
	}
	
	// Handler code
	_handler->handle(recv_buf);

	// TODO handle packets with IDs
}

void
vl::udp::Server::setHandler(boost::shared_ptr<udp::Handler> hand)
{
	_handler = hand;
}
