#ifndef VL_UDP_SERVER_HPP
#define VL_UDP_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "handler.hpp"

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{
	
namespace udp
{

class Server
{
public:
	Server( uint16_t const port );

	~Server();

	void mainloop( void );

	/// Adds a new handler to the handler stack
	/// Will not check if the packet ids are same so the first handler added
	/// for that id will get called.
	void setHandler( boost::shared_ptr<udp::Handler> hand );

private :
	/// Copying is forbidden
	// Something funcky with the io_service or socket, so we can not forbid copy
//	Server(const Server& other) {}
//	virtual Server& operator=(const Server& other) {}
//	virtual bool operator==(const Server& other) const {}

	boost::asio::io_service io_service;
	boost::udp::socket socket;

	/// Supports multiple handlers
	/// If you want multiple handlers all packets have to have an unique ID
	/// Else first handler should be the only one and should have packet ID = -1
	boost::shared_ptr<udp::Handler> _handler;
};

}	// namespace udp

}	// namespace vl

#endif // VL_UDP_SERVER_HPP
