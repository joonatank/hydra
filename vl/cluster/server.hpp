/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file server.hpp
 */

#ifndef VL_CLUSTER_SERVER_HPP
#define VL_CLUSTER_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "message.hpp"
#include "states.hpp"

#include "stats.hpp"

#include "logger.hpp"

#include "base/timer.hpp"

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

class Server : public LogReceiver
{
public:

	struct ClientInfo
	{
		ClientInfo(boost::udp::endpoint const &p)
			: address(p)//, state(s), updates(0), frame(0), rendering(false), output(false)
		{}

		boost::udp::endpoint address;
		ClientState state;
	};

	Server( uint16_t const port );

	~Server();

	void poll(void);

	void block_till_initialised(vl::time const &limit = vl::time());

	/// Update the SceneGraph on slaves
	/// Blocks till they are updated
	void update(vl::Stats &stats);

	/// @brief Start drawing on all the slaves. Non blocking
	/// @return true if at least one slave is rendering, false otherwise
	bool start_draw(vl::Stats &stats);

	/// Finish drawing on every slave
	/// Blocks till done
	void finish_draw(vl::Stats &stats, vl::time const &limit = vl::time());

	// TODO this should block till all the clients have shutdown
	void shutdown( void );

	// New message interface
	void sendMessage(Message const &msg);

	/// Store the Environment message for further use
	void sendEnvironment( Message const &msg );

	/// Store the project message for further use
	// Can be updated by a another call and will be sent again to all clients
	void sendProject( Message const &msg );

	/// Send an Initial SceneGraph
	void sendInit( Message const &msg );

	/// Send an SceneGraph update
	void sendUpdate( Message const &msg );

	/// Send information on new SceneGraph elements created
	void sendCreate( Message const &msg );

	/// @brief Has the Server unprocessed Input Messages.
	/// @return true if the server has input messages, false otherwise
	/// @todo change to use functors/callbacks rather than this has + get
	/// method
	bool messages( void ) const
	{ return !_messages.empty(); }

	/**	@brief receive an input message from a stack.
	 *	@todo same as above replace with callbacks
	 *	All messages are dynamically allocated so the user needs to delete
	 *	the message after use.
	 */
	vl::cluster::Message popMessage(void);

	typedef std::vector<ClientInfo> ClientList;
	typedef std::vector<ClientInfo *> ClientRefList;

	/// LogReceiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

private :
	void _handle_message(Message &msg, ClientInfo &client);

	ClientInfo &_add_client( boost::udp::endpoint const &endpoint );

	bool _has_client(boost::udp::endpoint const &address) const;

	ClientInfo &_find_client(boost::udp::endpoint const &address);

	ClientInfo *_find_client_ptr(boost::udp::endpoint const &address);

	ClientInfo const *_find_client_ptr(boost::udp::endpoint const &address) const;

	void _sendEnvironment(ClientInfo &client);

	// Updates the clients update frame after sending
	// Which will ensure that no matter where and how many times
	// this is called the same messaage will never be sent more than once
	void _sendCreate(ClientInfo &client);

	void _sendUpdate(ClientInfo &client);

	void _sendOuput(ClientInfo &client);

	void _sendMessage(boost::udp::endpoint const &endpoint, vl::cluster::Message const &msg);

	void _handle_ack(ClientInfo &client, MSG_TYPES ack_to);

	/// @brief blocks till all the clients have are in state
	/// @param cs state which the clients should be in
	/// @param timelimit maximum time in ms to wait before returning
	/// 0 means no timelimit
	/// @return true if state was changed, false if timelimit expired
	bool _block_till_state(CLIENT_STATE cs, vl::time const &limit = vl::time());

	bool _rendering( void );

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;

	ClientList _clients;

	std::deque<Message> _messages;

	/// Messages stored for when new clients connect
	/// Environment message, set in the start
	std::vector<char> _env_msg;
	/// Project message, the latest one set
	std::vector<char> _proj_msg;
	/// Init message, the latest one set
	std::vector<char> _msg_init;
	/// Last update message? Might need a whole vector of these
	std::vector<char> _msg_update;

	/// Create MSGs
	std::vector< std::pair<uint32_t, Message> > _msg_creates;

	uint32_t _frame;
	uint32_t _update_frame;

	uint32_t _n_log_messages;
	std::vector<vl::LogMessage> _new_log_messages;

};	// class Server

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_SERVER_HPP
