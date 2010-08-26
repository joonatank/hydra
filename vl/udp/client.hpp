#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>

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
	Client( char const *hostname, char const *port );

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
