#include "client.hpp"

#include <iostream>

vl::udp::Client::Client( char const *hostname, char const *port )
	: io_service(), socket( io_service ), receiver_endpoint()
{
	boost::udp::resolver resolver(io_service);
	boost::udp::resolver::query query( boost::udp::v4(), hostname, port );
	receiver_endpoint = *resolver.resolve(query);

	socket.open( boost::udp::v4() );
}

vl::udp::Client::~Client()
{

}

void
vl::udp::Client::send(std::vector<double> const & msg)
{
	if( msg.size() == 0 )
	{ return; }

	size_t n = socket.send_to(boost::asio::buffer(msg), receiver_endpoint);
}
