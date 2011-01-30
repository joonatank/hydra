#ifndef VL_CLUSTER_SERVER_HPP
#define VL_CLUSTER_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "message.hpp"

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

class Server
{
public:
	Server( uint16_t const port );

	~Server();

	void mainloop( void );

	void sendToAll( Message const &msg );

	void sendToNewClients( Message const &msg );

	bool oldClients( void ) const
	{ return !_clients.empty(); }

	bool newClients( void ) const
	{ return !_new_clients.empty(); }

	bool inputMessages( void )
	{ return !_input_msgs.empty(); }

	Message *popInputMessage( void );

private :
	void _addClient( boost::udp::endpoint const &endpoint );

	/// Copying is forbidden
	// Something funcky with the io_service or socket, so we can not forbid copy
//	Server(const Server& other) {}
//	virtual Server& operator=(const Server& other) {}
//	virtual bool operator==(const Server& other) const {}

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;

	// TODO add a list of clients to whom updates are sent
	std::vector<boost::udp::endpoint> _clients;

	std::vector<boost::udp::endpoint> _new_clients;

	std::vector<Message *> _input_msgs;

};	// class Server

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_SERVER_HPP
