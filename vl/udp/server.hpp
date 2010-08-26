#ifndef VL_UDP_SERVER_HPP
#define VL_UDP_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "command.hpp"

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

	void addCommand( vl::udp::CommandRefPtr cmd );

private :
	void handle( std::vector<double> msg );

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
	std::vector< vl::udp::CommandRefPtr > _commands;
};

}	// namespace udp

}	// namespace vl

#endif // VL_UDP_SERVER_HPP
