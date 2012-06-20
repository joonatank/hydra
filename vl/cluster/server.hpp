/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluser/server.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
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

// @todo Synchronisation
// Should we send zero data messages?
// The client can not handle not receiving updates but only receiving a draw message
// at the moment, so should there always be an update message and
// then the draw message or is the update message optional?
// if the update message is optional
// there needs to be a flag in MSG_START_FRAME which messages
// are going to be sent in that frame
// MSG_CREATE and/or MSG_UPDATE
// or more accurately the ID/frameID of the current messages in the server
// so client can choose to either proceed without receiving those
// or block till they are received.
// synchronising the DRAW is not necessary other than for the
// MSG_SG_CREATE, MSG_SG_UPDATE and MSG_SWAP

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

	class ClientFSM : public ClientFSM_
	{
	public :
		ClientFSM(void)
			: address()
			, _server(0)
			, environment_sent_time(vl::time(10, 0))
			, last_alive()
			, create_frame(-1)
			, ignore_updates(false)
		{}
		
		boost::udp::endpoint address;
		// used for sending messages from actions
		Server *_server;

		/// Used for not trying to send environment less than a couple of seconds a part
		// @todo this should be in the State not here
		// @todo why is this chrono and not time?
		vl::chrono environment_sent_time;
		
		/// @brief When the client was last seen alive, in servers internal clock time
		/// There is a timeout when the Client is consired dead if not seen.
		/// This timeout and the action taken depend on the server.
		vl::time last_alive;

		/// What messages have been sent to the client without receiving an ACK
		std::map<vl::time, MessagePart> _sent_msgs;

		void enable_rendering(bool enable)
		{ _rendering_enabled = enable; }

		bool is_rendering_enabled(void) const
		{ return _rendering_enabled; }

		/// @fixme
		void enable_output(bool enable)
		{ }

		/// @fixme
		bool is_output_enabled(void) const
		{ return false; }

		// Last received create message
		int64_t create_frame;

		bool ignore_updates;

		/// Data
	private :
		bool _rendering_enabled;

		/// Public Pure virtuals from ClientFSM
	public :

		/// Private Pure Virtuals from ClientFSM
	private :
		virtual void _do_rest(void);

		virtual void _do_quit(quit const &evt);

		virtual void _do_init(init_done const &evt);

		virtual void _do_load(load_done const &evt);

		virtual void _do_init_requested(init_requested const &evt);
		virtual void _do_load_requested(load_requested const &evt);

		virtual void _do_graph_requested(graph_requested const &evt);
		virtual void _do_graph_load(graph_loaded const &evt);

		virtual void _do_start_frame(start_frame const &evt);
		virtual void _do_frame_done(frame_done const &evt);

		virtual void _do_handle_error(timer_expired const &evt);

	private :
		ClientFSM(ClientFSM const &);
		ClientFSM &operator=(ClientFSM const &);


	};	// ClientFSM

	typedef vl::msm::back::state_machine<Server::ClientFSM> Client;

	Server(uint16_t const port);

	~Server();

	void poll(void);

	/// @brief Start drawing on all the slaves. Non blocking
	/// @return true if at least one slave is rendering, false otherwise
	/// Blocks till updates have been sent to all slaves and 
	/// slaves have started rendering.
	/// @todo Replace with callbacks to the caller so we can implement
	/// blocking in the Application side.
	/// This avoids the problem of having to change server when testing
	/// different synchronisation schemes.
	void start_draw(uint32_t frame, vl::time const &timestamp);

	/// @brief Finish drawing on every slave
	/// Blocks till done
	/// @todo Replace with callbacks to the caller so we can implement
	/// blocking in the Application side.
	/// This avoids the problem of having to change server when testing
	/// different synchronisation schemes.
	void finish_draw(uint32_t frame, vl::time const &timestamp);

	/// @brief shutdown all clients
	/// non-blocking
	/// @todo there should be shutdown signal that is sent when all clients
	/// have been gracefully shutdown and disconnected
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
	{ _request_message_signal.connect(slot); return 1; }

	int addMessageListener(MessageReceived::slot_type const &slot)
	{ _message_received_signal.connect(slot); return 1; }

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

	typedef std::vector<Client *> ClientList;

	/// LogReceiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

	vl::Report<vl::time> const &getReport(void) const
	{ return _server_report; }


	/// Status queries
	bool has_clients(void) const;

	bool has_rendering_clients(void) const;

	/// @brief introduce an artificial lag to all clients
	/// This method should never be used for anything else than testing.
	void inject_lag(vl::time const &t);

	/// @internal called from ClientFSM
	void _request_message(Server::ClientFSM *client, MSG_TYPES type);

	/// @internal called from ClientFSM
	void _send_message(Server::ClientFSM *client, Message const &msg);

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
	void _handle_message(Message &msg, Server::Client &client);

	Server::Client *_add_client( boost::udp::endpoint const &endpoint );

	bool _has_client(boost::udp::endpoint const &address) const;

	Server::Client &_find_client(boost::udp::endpoint const &address);

	Server::Client *_find_client_ptr(boost::udp::endpoint const &address);

	Server::Client const *_find_client_ptr(boost::udp::endpoint const &address) const;

	// Updates the clients update frame after sending
	// Which will ensure that no matter where and how many times
	// this is called the same messaage will never be sent more than once
	void _sendCreate(Server::Client &client);

	void _sendOuput(Server::Client &client);

	void _sendMessage(Server::Client &client, vl::cluster::Message const &msg);

	void _handle_ack(Server::Client &client, MSG_TYPES ack_to, vl::cluster::Message const &msg);

	/// @brief Blocks till all the client state machines have a given flag
	/// these flags for now are used to distinguis the different phases
	/// of the rendering loop and nothing else.
	/// @param Flag is given in as a template parameter.
	/// @todo add timers in to the FSM so that this function can be failed
	/// if a given time limit exceeds.
	template <typename T> void _block_till_state_has_flag(void);

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

	// signals
	MessageReceived _message_received_signal;
	RequestMessage _request_message_signal;
	std::vector<std::pair<Server::Client *, MSG_TYPES> > _requested_msgs;

	vl::Report<vl::time> _server_report;
	vl::chrono _report_timer;
	// The running clock of this server used to manage clients (timeouts etc.)
	vl::chrono _internal_clock;

	/// Maximum time till we consider a slave to be dead
	vl::time _maximum_time_to_timeout;

	/// Rendering loop state variables
	/// only valid while the rendering is in progress
	ClientList _renderers;
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

	// the initial state of the FSM. Must be defined
	// @todo replace with a region where another one is for errors
	// needs to be able to reset the other regions state though.
	typedef mpl::vector<Unknown, AllOk> initial_state;

	typedef ServerFSM_ s;
struct transition_table : vl::mpl::vector<
//    Start     Event        Target      Action                      Guard 
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Unknown , init				,	Initing    , &s::_do_init  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Initing , vl::none			,	NotRendering	, &s::_do_rest  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< NotRendering , update		,	Updating    , &s::_do_update	,  &s::has_rendering_clients >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Updating, render				,	Rendering   , &s::_do_render	, &s::has_rendering_clients >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Rendering , swap				,	Swapping    , &s::_do_swap		, &s::has_rendering_clients >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Swapping , swap_done			,	NotRendering	, &s::_do_swap_done , &s::has_rendering_clients >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
// Error handling
a_row< AllOk	, timer_expired		,	Error		, &s::_report_error >,
a_row< Error	, clear_error		,	AllOk		, &s::_do_rest >
> {};

protected:
    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
		// @fixme just a Hack so this will not print endlessly when there are no clients
		if(state != 2 && state != 6)
		{
			std::clog << "no transition from state " << state
				<< " on event " << typeid(e).name() << std::endl;
		}
    }

	/// We need to override this to get valid exception handling
	/// default is asserting a failure which makes the exceptions message carbage.
	/// Also we need exceptions in Release code which makes the default
	/// implemantion very dangorous.
	template <class Fsm,class Event>
	void exception_caught(Event const& ,Fsm&, std::exception &e)
	{
		boost::exception_ptr ex = boost::current_exception();
		boost::rethrow_exception(ex);
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

template<typename T>
void
vl::cluster::Server::_block_till_state_has_flag(void)
{
	vl::chrono t;
	bool ready = false;
	while( !ready )
	{
		// @todo
		// We should here mark the client as lost for this round of rendering
		// This allows us to render and accept input on the master when the
		// slaves are still initialising.

		poll();

		ready = true;
		ClientList::const_iterator iter;
		for( iter = _renderers.begin(); iter != _renderers.end(); ++iter )
		{
			if( !(*iter)->is_flag_active<T>() && !(*iter)->ignore_updates )
			{
				ready = false;
				break;
			}
		}

		// TODO should wait only for a while and then resent the last message

		// Needs to sleep in Linux at least. Busy wait will cause a huge lag.
		vl::msleep(uint32_t(0));
	}
}

#endif // HYDRA_CLUSTER_SERVER_HPP
