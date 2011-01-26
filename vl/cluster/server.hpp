#ifndef VL_CLUSTER_SERVER_HPP
#define VL_CLUSTER_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

class Message;

class Server
{
public:
	Server( uint16_t const port );

	~Server();

	void mainloop( void );

	// TODO not implemented
	void sendToAll( Message const &msg );

private :
	void handle( std::vector<char> msg );

	/// Copying is forbidden
	// Something funcky with the io_service or socket, so we can not forbid copy
//	Server(const Server& other) {}
//	virtual Server& operator=(const Server& other) {}
//	virtual bool operator==(const Server& other) const {}

	boost::asio::io_service io_service;
	boost::udp::socket socket;

	// TODO add a list of clients to whom updates are sent

};	// class Server

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_SERVER_HPP
