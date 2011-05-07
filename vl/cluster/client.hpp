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

#include "base/timer.hpp"

#include "base/report.hpp"
/// Necessary for the MeshLoader callback base
#include "mesh_manager.hpp"

/// Necessary for Callbacks that take ref ptrs
#include <boost/enable_shared_from_this.hpp>

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

};	// struct ClientMsgCallback

/// blocking mesh loader for slave
struct SlaveMeshLoaderCallback : public BlockingMeshLoaderCallback
{
	SlaveMeshLoaderCallback(vl::cluster::ClientRefPtr clien);

	/// Blocks till the mesh is loaded and returns a valid mesh
	virtual vl::MeshRefPtr loadMesh(std::string const &fileName);

	vl::cluster::ClientRefPtr client;

};	// class SlaveMeshLoaderCallback


/// @class Client
class Client : public boost::enable_shared_from_this<Client>
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

	/// @todo should probably use ref pointers
	Message waitForMessage(MSG_TYPES type);

private :
	void _handleMessage(vl::cluster::Message &msg);

	boost::asio::io_service _io_service;

	boost::udp::socket _socket;

	boost::udp::endpoint _master;

	std::vector<Message *> _messages;

	ClientState _state;

	vl::timer _request_timer;

	vl::RendererInterfacePtr _renderer;

	std::vector<vl::Callback *> _callbacks;

	// @todo this should save the simulation time and frame
	// uint32_t _frame
	// vl::time _sim_time;

	/// Rendering pipeline performance checks
	vl::timer _rend_timer;
	vl::Report<vl::time> _rend_report;
	vl::timer _print_timer;

};	// class Client

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_CLIENT_HPP
