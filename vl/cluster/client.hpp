/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 */

#ifndef VL_CLUSTER_CLIENT_HPP
#define VL_CLUSTER_CLIENT_HPP

#include <boost/asio.hpp>

#include <stdint.h>

#include "message.hpp"
#include "states.hpp"

// Necessary for the Renderer RefPtr
#include "typedefs.hpp"

// Necessary for the Callback structs
#include "renderer_interface.hpp"

#include <OGRE/OgreTimer.h>

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

class Client;

/// Callbacks
struct ClientMsgCallback : public vl::MsgCallback
{
	ClientMsgCallback(Client *own);

	virtual void operator()(vl::cluster::Message const &msg);

	Client *owner;
};

class Client
{
public:
	Client( char const *hostname, uint16_t port, vl::RendererInterfacePtr rend );

	virtual ~Client(void);

	bool isRunning(void);

	bool isRendering(void);

	/// Processes all the pending messages
	void mainloop(void);

	void sendMessage(vl::cluster::Message const &msg);

	void sendAck( vl::cluster::MSG_TYPES );

private :
	void _handleMessage(vl::cluster::Message &msg);

	boost::asio::io_service _io_service;

	boost::udp::socket _socket;

	boost::udp::endpoint _master;

	std::vector<Message *> _messages;

	ClientState _state;

	Ogre::Timer _request_timer;

	vl::RendererInterfacePtr _renderer;

	std::vector<vl::Callback *> _callbacks;

};	// class Client

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_CLIENT_HPP
