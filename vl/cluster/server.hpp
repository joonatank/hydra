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

#include "base/chrono.hpp"

#include "typedefs.hpp"

/// for profiling
#include "base/report.hpp"

#include "base/state_machines.hpp"

#include <boost/signal.hpp>

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{


// Forward declaration
class ServerFSM_;
// Pick a back-end
typedef vl::msm::back::state_machine<ServerFSM_> ServerFSM;

struct RequestedMessage
{
	RequestedMessage(MSG_TYPES type_)
		: type(type_)
	{}

	RequestedMessage(MSG_TYPES type_, std::string const &name_, RESOURCE_TYPE res_type_)
		: type(type_), name(name_), res_type(res_type_)
	{}

	MSG_TYPES type;
	// extra data only useful for resource messages
	std::string name;
	RESOURCE_TYPE res_type;
};

class Server : public LogReceiver
{
	typedef boost::signal<void (RequestedMessage const &)> RequestMessage;
	typedef boost::signal<void (vl::cluster::Message const &)> MessageReceived;

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
		vl::chrono environment_sent_time;

		/// What messages have been sent to the client without receiving an ACK
		std::map<vl::time, MessagePart> _sent_msgs;
	};

	Server(uint16_t const port);

	~Server();

	void poll(void);

	/// Update the SceneGraph on slaves
	/// Blocks till they are updated
	void update(uint32_t frame, vl::time const &timestamp);

	/// @brief Start drawing on all the slaves. Non blocking
	/// @return true if at least one slave is rendering, false otherwise
	void start_draw(uint32_t frame, vl::time const &timestamp);

	/// Finish drawing on every slave
	/// Blocks till done
	void finish_draw(uint32_t frame, vl::time const &timestamp);

	// TODO this should block till all the clients have shutdown
	void shutdown( void );

	/// New message interface
	/// @todo should all be moved to callbacks
	/// first it will simplify the interface (no duplicate methods)
	/// second it will always provide the latest version when it's needed
	/// no more copying temporaries and sending them when requested.
	///
	/// mhhh should it?
	/// we can do easily boost::signal mapping if necessary
	/// but what is driving what?
	/// architecture wise Master should be deciding when new messages
	/// are available and Server (or similar instance) should store
	/// those messages and forward them when convienient
	///
	/// this would imply that we map a Master method to sendMessage
	/// and we can remove all callbacks and specific send methods
	/// when the order matters the Master should insure that
	/// sendMessage is called in correct order as 
	/// it should be FIFO
	/// Sender doesn't need to care about the Method he is calling
	/// or the order that different types of messages is sent
	/// Server handles that.
	/// Sender needs to care that same type type of Messages are sent
	/// in correct order.
	void sendMessage(Message const &msg);

	int addRequestMessageListener(RequestMessage::slot_type const &slot)
	{ _request_message.connect(slot); return 1; }

	int addMessageListener(MessageReceived::slot_type const &slot)
	{ _message_received.connect(slot); return 1; }

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

	typedef std::vector<ClientInfo> ClientList;
	typedef std::vector<ClientInfo *> ClientRefList;

	/// LogReceiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

	// Return non-const reference for ease (to integrate into python)
	vl::Report<vl::time> &getReport(void)
	{ return _server_report; }


	/// Status queries
	bool has_clients(void) const;

	bool has_rendering_clients(void) const;


	// FSM functions
	template<typename T>
	void process_event(T const &evt);

	// internal FSM functions
public :
	void _do_init(init const &evt);

	void _do_render(render const &evt);

	void _do_swap(swap const &evt);

	void _do_update(vl::cluster::update const &evt);

	void _do_swap_done(swap_done const &evt);

	void _do_rest(vl::none const &evt);

	void _report_error(timer_expired const &evt);

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

	void _sendInit(ClientInfo &client);

	void _sendUpdate(ClientInfo &client);

	void _sendOuput(ClientInfo &client);

	void _sendMessage(ClientInfo &client, vl::cluster::Message const &msg);

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

	uint32_t _n_log_messages;
	std::vector<vl::LogMessage> _new_log_messages;

	//ServerDataCallback *_data_cb;
	// signals
	MessageReceived _message_received;
	RequestMessage _request_message;
	std::vector<std::pair<ClientInfo &, MSG_TYPES> > _requested_msgs;

	vl::Report<vl::time> _server_report;
	vl::chrono _report_timer;


	/// Rendering loop state variables
	/// only valid while the rendering is in progress
	ClientRefList _renderers;
	uint32_t _frame;
	bool _draw_error;

	// FSM
	std::auto_ptr<ServerFSM> _fsm;

};	// class Server


class ServerFSM_ :  public msm::front::state_machine_def<ServerFSM_, vl::state> 
{
public :

	Server *_impl;

	template <class Event,class FSM>
	void on_entry(Event const& ,FSM&) 
	{
		std::cout << "entering: Server FSM" << std::endl;
	}

	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ) 
	{
		std::cout << "leaving: Server FSM" << std::endl;
	}


	void setServer(Server *server)
	{ _impl = server; }

	void _do_init(init const &evt)
	{
		assert(_impl);
		_impl->_do_init(evt);
	}

	template<typename T>
	void _do_rest(T const &evt)
	{
		assert(_impl);
		_impl->_do_rest(vl::none());
	}

	void _do_render(render const &evt)
	{
		assert(_impl);
		_impl->_do_render(evt);
	}

	void _do_swap(swap const &evt)
	{
		assert(_impl);
		_impl->_do_swap(evt);
	}

	void _do_update(update const &evt)
	{
		assert(_impl);
		_impl->_do_update(evt);
	}

	void _do_swap_done(swap_done const &evt)
	{
		assert(_impl);
		_impl->_do_swap_done(evt);
	}

	void _report_error(timer_expired const &evt)
	{
		_impl->_report_error(evt);
	}
	

	template<typename T>
	bool has_rendering_clients(T const &evt)
	{
		return _impl->has_rendering_clients();
	}
	
	// The list of FSM states
	// Initial state
	struct Unknown : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Unknown" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Unknown" << std::endl;}
	};
	// State that is before we have loaded anything
	struct Initing : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Initing" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Initing" << std::endl;}
	};
	/// Doing nothing at the moment
	struct Resting : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {} // {std::clog << "Server FSM : entering: Resting" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {} // {std::clog << "Server FSM : leaving: Resting" << std::endl;}
	};
	struct Updating : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {} // {std::clog << "Server FSM : entering: Update" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {} //{std::clog << "Server FSM : leaving: Update" << std::endl;}
	};
	/// Render
	struct Rendering : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {} // {std::clog << "Server FSM : entering: Rendering" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {} //{std::clog << "Server FSM : leaving: Rendering" << std::endl;}
	};
	struct Swapping : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {} //{std::clog << "Server FSM : entering: Swapping" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {} //{std::clog << "Server FSM : leaving: Swapping" << std::endl;}
	};
	struct Error : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Error" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Error" << std::endl;}
	};

	// the initial state of the FSM. Must be defined
	typedef Unknown initial_state;

	typedef ServerFSM_ s;
struct transition_table : vl::mpl::vector<
//    Start     Event        Target      Action                      Guard 
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Unknown , init				,	Initing    , &s::_do_init  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Initing , vl::none			,	Resting    , &s::_do_rest  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Resting , update				,	Updating    , &s::_do_update,  &s::has_rendering_clients >,
a_row< Resting , timer_expired		,	Error		, &s::_report_error >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Updating, render				,	Rendering   , &s::_do_render >,
a_row< Updating, timer_expired		,	Error		, &s::_report_error >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Rendering , swap				,	Swapping    , &s::_do_swap  >,
a_row< Rendering , timer_expired	,	Error		, &s::_report_error >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Swapping , swap_done			,	Resting		, &s::_do_swap_done >,
a_row< Swapping , timer_expired		,	Error		, &s::_report_error >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Error	, clear_error		,	Resting		, &s::_do_rest >
> {};

    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::clog << "no transition from state " << state
            << " on event " << typeid(e).name() << std::endl;
    }

};	// class ServerFSM_


}	// namespace cluster

}	// namespace vl

/// Templates
template<typename T>
void
vl::cluster::Server::process_event(T const &evt)
{
	_fsm->process_event(evt);
}

#endif // HYDRA_CLUSTER_SERVER_HPP
