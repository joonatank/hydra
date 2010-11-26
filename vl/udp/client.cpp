/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-09
 */

#include "client.hpp"

#include <sstream>

vl::udp::Client::Client( char const *hostname, uint16_t port )
	: io_service(), socket( io_service ), receiver_endpoint()
{
	std::stringstream ss;
	ss << port;
	boost::udp::resolver resolver(io_service);
	boost::udp::resolver::query query( boost::udp::v4(), hostname, ss.str().c_str() );
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
