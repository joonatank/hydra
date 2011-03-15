/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 */

#ifndef VL_CLUSTER_CLIENT_HPP
#define VL_CLUSTER_CLIENT_HPP

#include <boost/asio.hpp>

#include <stdint.h>

#include "message.hpp"
#include "states.hpp"

#include <OGRE/OgreTimer.h>

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

	/// Processes all the pending messages
	void mainloop( void );

	/// Register this client to receive updates in the Rendering context
	void registerForUpdates( void );

	void registerForOutput( void );

	bool messages() const
	{ return !_messages.empty(); }

	Message *popMessage( void );

	void sendMessage( Message *msg );

	void sendAck( vl::cluster::MSG_TYPES );

private :
	void _sentRequestUpdates( void );

	/// Copying is forbidden
//	Client(const Client& other) {}
//	virtual Client& operator=(const Client& other) {}
//	virtual bool operator==(const Client& other) const {}

	boost::asio::io_service _io_service;

	boost::udp::socket _socket;

	boost::udp::endpoint _master;

	std::vector<Message *> _messages;

	CLIENT_STATE _state;

	Ogre::Timer _request_timer;

};	// class Client

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_CLIENT_HPP
