/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#ifndef VL_CLUSTER_CLIENT_HPP
#define VL_CLUSTER_CLIENT_HPP

#include <boost/asio.hpp>

#include <stdint.h>
#include "message.hpp"

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

class Client
{
public:
	Client( char const *hostname, uint16_t port );

	virtual ~Client( void );

// 	void send( std::vector<double> const & msg );

	/// Processes all the pending messages
	void mainloop( void );

	/// Register this client to receive updates in the Rendering context
	void registerForUpdates( void );

// 	void registerMessage( Message const &message, MessageCallback *callback );

	bool messages() const
	{ return !_messages.empty(); }

	Message *popMessage( void )
	{
		Message *tmp = _messages.front();
		_messages.erase( _messages.begin() );
		return tmp;
	}

private :
	/// Copying is forbidden
//	Client(const Client& other) {}
//	virtual Client& operator=(const Client& other) {}
//	virtual bool operator==(const Client& other) const {}

	/// Breaks a datagram down in to separate messages and processes them
	/// according to the Message stack
	void handle( std::vector<char> &datagram );

	boost::asio::io_service _io_service;

	boost::udp::socket _socket;

	boost::udp::endpoint _receiver_endpoint;

	std::vector<Message *> _messages;

};	// class Client

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_CLIENT_HPP
