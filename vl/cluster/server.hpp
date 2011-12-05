/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluser/server.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_CLUSTER_SERVER_HPP
#define HYDRA_CLUSTER_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "message.hpp"
#include "states.hpp"

#include "logger.hpp"

#include "base/timer.hpp"

#include "typedefs.hpp"

/// for profiling
#include "base/report.hpp"

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

struct ServerDataCallback : public vl::Callback
{
	/// Create the SceneGraph init message
	virtual Message createInitMessage(void) = 0;

	virtual Message createEnvironmentMessage(void) = 0;

	virtual Message createProjectMessage(void) = 0;

	/// @todo add CREATE_MSG and UPDATE_MSG also

	/// Create a resource message containing certain type of resource with the name
	virtual Message createResourceMessage(RESOURCE_TYPE type, std::string const &name) = 0;
};

class Server : public LogReceiver
{
public:

	struct ClientInfo
	{
		ClientInfo(boost::udp::endpoint const &p)
			: address(p)
			, environment_sent_time(vl::time(10, 0))
		{}

		boost::udp::endpoint address;
		ClientState state;

		/// Used for not trying to send environment less than a couple of seconds a part
		vl::timer environment_sent_time;

		/// What messages have been sent to the client without receiving an ACK
		std::map<vl::time, MessagePart> _sent_msgs;
	};

	Server(uint16_t const port, ServerDataCallback *cb);

	~Server();

	void poll(void);

	/// Update the SceneGraph on slaves
	/// Blocks till they are updated
	void update(void);

	/// @brief Start drawing on all the slaves. Non blocking
	/// @return true if at least one slave is rendering, false otherwise
	bool start_draw(void);

	/// Finish drawing on every slave
	/// Blocks till done
	void finish_draw(vl::time const &limit = vl::time());

	// TODO this should block till all the clients have shutdown
	void shutdown( void );

	/// New message interface
	/// @todo should all be moved to callbacks
	/// first it will simplify the interface (no duplicate methods)
	/// second it will always provide the latest version when it's needed
	/// no more copying temporaries and sending them when requested.
	void sendMessage(Message const &msg);

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

	void block_till_initialised(vl::time const &limit = vl::time());

	vl::time getSimulationTime(void) const;

	typedef std::vector<ClientInfo> ClientList;
	typedef std::vector<ClientInfo *> ClientRefList;

	/// LogReceiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

	// Return non-const reference for ease (to integrate into python)
	vl::Report<vl::time> &getReport(void)
	{ return _server_report; }

private :
	void _handle_message(Message &msg, ClientInfo &client);

	ClientInfo &_add_client( boost::udp::endpoint const &endpoint );

	bool _has_client(boost::udp::endpoint const &address) const;

	ClientInfo &_find_client(boost::udp::endpoint const &address);

	ClientInfo *_find_client_ptr(boost::udp::endpoint const &address);

	ClientInfo const *_find_client_ptr(boost::udp::endpoint const &address) const;

	// Updates the clients update frame after sending
	// Which will ensure that no matter where and how many times
	// this is called the same messaage will never be sent more than once
	void _sendCreate(ClientInfo &client);

	void _sendUpdate(ClientInfo const &client);

	void _sendOuput(ClientInfo &client);

	void _sendMessage(ClientInfo const &client, vl::cluster::Message const &msg);

	void _handle_ack(ClientInfo &client, MSG_TYPES ack_to);

	/// @brief blocks till all the clients have are in state
	/// @param cs state which the clients should be in
	/// @param timelimit maximum time in ms to wait before returning
	/// 0 means no timelimit
	/// @return true if state was changed, false if timelimit expired
	bool _block_till_state(CLIENT_STATE cs, ClientRefList clients, vl::time const &limit = vl::time());

	bool _rendering( void );

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;

	ClientList _clients;

	std::deque<Message> _messages;

	/// @todo this should be removed, use a callback to create it when needed
	/// Last update message? Might need a whole vector of these
	Message _msg_update;

	/// Create MSGs
	std::vector< std::pair<uint32_t, Message> > _msg_creates;

	// @todo these should be in GameManager/Config
	uint32_t _frame;
	uint32_t _update_frame;

	// @todo this should be moved to GameManager
	vl::timer _sim_timer;

	uint32_t _n_log_messages;
	std::vector<vl::LogMessage> _new_log_messages;

	ServerDataCallback *_data_cb;

	vl::Report<vl::time> _server_report;
	vl::timer _report_timer;

};	// class Server

}	// namespace cluster

}	// namespace vl

#endif // HYDRA_CLUSTER_SERVER_HPP
