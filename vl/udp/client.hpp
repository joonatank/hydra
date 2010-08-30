#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>

#include <stdint.h>

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{
	
namespace udp
{

class Client
{
public:
	Client( char const *hostname, uint16_t port );

	virtual ~Client();

	void send( std::vector<double> const & msg );


private :
	/// Copying is forbidden
//	Client(const Client& other) {}
//	virtual Client& operator=(const Client& other) {}
//	virtual bool operator==(const Client& other) const {}


	boost::asio::io_service io_service;

	boost::udp::socket socket;

	boost::udp::endpoint receiver_endpoint;

};	// class Client

}	// namespace udp

}	// namespace vl

#endif // CLIENT_HPP
