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

/// The state the client is in the update and rendering pipe
/// Used to determine what message is next send to that client
/// Moving from one to the next state is done when server sends the message
/// for the current state.
/// TODO state change should be done only after client ACKs
enum CLIENT_STATE
{
	CS_UNDEFINED,
	CS_REQ,		// Client has requested updates
	CS_ENV,		// Environment settings have been sent
	CS_PROJ,	// Project settings have been sent
	CS_INIT,	// Initial SceneGraph has been sent
	CS_UPDATE,	// Rendering loop : Update has been sent
	CS_DRAW,	// Rendering loop : Draw has been sent
	CS_SWAP,	// Rendering loop : Swap has been sent
};

class Server
{
public:
	Server( uint16_t const port );

	~Server();

	void receiveMessages( void );

	/// Synchronious method that blocks till all the clients have done
	/// update, draw and swap
	void render( void );

	/// Store the Environment message for further use
	// TODO change to use dynamically allocated memory so that the message
	// can be stored without copiying
	void sendEnvironment( Message const &msg );

	/// Store the project message for further use
	// Can be updated by a another call and will be sent again to all clients
	void sendProject( Message const &msg );

	/// Send an Initial SceneGraph
	void sendInit( Message const &msg );

	/// Send an SceneGraph update
	void sendUpdate( Message const &msg );

	/// Returns true if some client needs an Initial SceneGraph
	/// @todo This always returns true for now
	/// the architecture should rather use functors/callbacks to be called 
	/// when they are needed
	bool needsInit( void ) const;

	/// @brief Has the Server unprocessed Input Messages.
	/// @return true if the server has input messages, false otherwise
	/// @todo change to use functors/callbacks rather than this has + get
	/// method
	bool inputMessages( void )
	{ return !_input_msgs.empty(); }

	/**	@brief receive an input message from a stack.
	 *	@todo same as above replace with callbacks
	 *	All messages are dynamically allocated so the user needs to delete
	 *	the message after use.
	 */
	Message *popInputMessage( void );

	typedef std::vector< std::pair<boost::udp::endpoint, CLIENT_STATE> > ClientList;

private :
	void _addClient( boost::udp::endpoint const &endpoint );

	void _sendProject( boost::udp::endpoint const &endpoint );

	void _sendEnvironment( std::vector<char> const &msg );

	void _sendEnvironment( boost::udp::endpoint const &endpoint );

	void _sendInit( boost::udp::endpoint const &endpoint );

	void _sendUpdate( boost::udp::endpoint const &endpoint );

	void _sendDraw( boost::udp::endpoint const &endpoint );

	void _sendSwap( boost::udp::endpoint const &endpoint );

	void _handleAck( boost::udp::endpoint const &client, MSG_TYPES ack_to );

	/// Returns when all the clients are ready for an update message
	void _waitUpdate( void );

	/// Returns when all the clients are ready for an draw message
	void _waitDraw( void );

	/// Returns when all the clients are ready for an swap message
	void _waitSwap( void );

	/// Copying is forbidden
	// Something funcky with the io_service or socket, so we can not forbid copy
//	Server(const Server& other) {}
//	virtual Server& operator=(const Server& other) {}
//	virtual bool operator==(const Server& other) const {}

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;

	ClientList _clients;

	std::vector<Message *> _input_msgs;

	/// Messages stored for when new clients connect
	/// Environment message, set in the start
	std::vector<char> _env_msg;
	/// Project message, the latest one set
	std::vector<char> _proj_msg;
	/// Init message, the latest one set
	std::vector<char> _msg_init;
	/// Last update message? Might need a whole vector of these
	std::vector<char> _msg_update;

};	// class Server

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_SERVER_HPP
