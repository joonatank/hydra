/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file cluster/states.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */


#ifndef HYDRA_CLUSTER_STATES_HPP
#define HYDRA_CLUSTER_STATES_HPP

#include <stdint.h>

#include "base/state_machines.hpp"

namespace vl
{

namespace cluster
{

/// The rendering state that the client is in rendering loop
/// Flags
enum CLIENT_STATE
{
	CS_CLEAR		= 0,
	CS_START		= 1<<0,
	CS_UPDATE_READY = 1<<1,	// Ready to receive an update
	CS_UPDATE		= 1<<2,	// NOT IN USE, Update has been sent
	CS_UPDATE_DONE	= 1<<3,	// NOT IN USE, An update has been completed
	CS_DRAW_READY	= 1<<4,
	CS_DRAW			= 1<<5,	// Rendering loop : Draw has been sent
	CS_DRAW_DONE	= 1<<6,
};

/// Holds a compound state and allows complex queries
/// Possible for non exclusive states
/// TODO change booleans to FLAGS
struct ClientState
{
	ClientState(void)
		: environment(false), project(false)
		, wants_render(false), wants_output(false)
		, has_init(false), frame(-1), create_frame(-1)
		, rendering_state(CS_CLEAR), shutdown(false)
	{}

	void clear_rendering_state(void)
	{ rendering_state = (uint32_t)CS_CLEAR; }

	bool is_rendering(void) const
	{
		return rendering_state != (uint32_t)CS_CLEAR;
	}

	/// @brief check if client is in a particular state
	bool has_rendering_state(CLIENT_STATE cs) const
	{ return (rendering_state & (uint32_t)cs) == (uint32_t)cs; }

	void set_rendering_state(CLIENT_STATE cs)
	{ rendering_state = rendering_state | (uint32_t)cs; }

	// Has environment config
	bool environment;
	// Has project config
	bool project;
	// Is currently rendering
//	bool rendering;
	// Wants rendering commands
	bool wants_render;
	// Wants output
	bool wants_output;
	// true if init message has been sent
	bool has_init;
	// Last renderer frame
	// needs to be int so that we can have it negative
	int64_t frame;
	// Last received create message
	int64_t create_frame;
	// Rendering state, only valid if rendering is true
	uint32_t rendering_state;
	bool shutdown;
};

inline
char const *getStateAsString(CLIENT_STATE cs)
{
	switch(cs)
	{
	case CS_CLEAR :
		return "CS_CLEAR";
	case CS_UPDATE_READY :
		return "CS_UPDATE_READY";
	case CS_UPDATE :
		return "CS_UPDATE";
	case CS_UPDATE_DONE :
		return "CS_UPDATE_DONE";
	case CS_DRAW_READY :
		return "CS_DRAW_READY";
	case CS_DRAW_DONE :
		return "CS_DRAW_DONE";
	default :
		return "";
	}
}


// FSM Events
// New clients are not concerned in the FSM
// they will be added a bit later probably by the use of submachines
// where a message handler is the large machine and it drives the
// submachines: rendering, new client, resouce requests, misc
//
// all message types are easily convertable to events
// so an implementation where all messages are events wouldn't be unthinkable
struct timer_expired {};
struct init {};
struct render 
{
	render(uint32_t frame_, vl::time const &timestamp_, vl::time const &limit = vl::time(10, 0))
		: timelimit(limit), frame(frame_), timestamp(timestamp_)
	{}

	vl::time timelimit;
	uint32_t frame;
	vl::time timestamp;
};
struct swap
{
	swap(uint32_t frame_, vl::time const &timestamp_, vl::time const &limit = vl::time(10, 0))
		: timelimit(limit), frame(frame_), timestamp(timestamp_)
	{}

	vl::time timelimit;
	uint32_t frame;
	vl::time timestamp;
};
// Update should handle both sending of create message and sending of update message
struct update
{
	update(uint32_t frame_, vl::time const &timestamp_, vl::time const &limit = vl::time(10, 0))
		: timelimit(limit), frame(frame_), timestamp(timestamp_)
	{}

	vl::time timelimit;
	uint32_t frame;
	vl::time timestamp;
};
struct clear_error {};
struct swap_done
{
	swap_done(uint32_t frame_, vl::time const &timestamp_)
		: frame(frame_), timestamp(timestamp_)
	{}

	uint32_t frame;
	vl::time timestamp;
};

struct quit {};
struct load {};
struct load_done {};
struct start {};
struct init_requested {};
struct init_received {};
struct load_requested {};
struct load_received {};


class ClientFSMImpl
{
	// internal FSM functions
public :
	virtual void _do_init(init const &evt) = 0;

	virtual void _do_render(render const &evt) = 0;

	virtual void _do_swap(swap const &evt) = 0;

	virtual void _do_update(vl::cluster::update const &evt) = 0;

	virtual void _do_swap_done(swap_done const &evt) = 0;

	virtual void _do_rest(vl::none const &evt) = 0;

	virtual void _do_quit(quit const &evt) = 0;
	virtual void _do_load(load const &evt) = 0;

	virtual void _report_error(timer_expired const &evt) = 0;

	virtual bool is_rendering(void) const = 0;

};

/// New FSM Client
class ClientFSM_ :  public msm::front::state_machine_def<ClientFSM_, vl::state> 
{
public :

	ClientFSMImpl *_impl;

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


	void setServer(ClientFSMImpl *server)
	{ _impl = server; }

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

	template<typename T>
	void _do_load(T const &evt)
	{
		assert(_impl);
		//_impl->_do_load(evt);
	}
	
	template<typename T>
	void _do_start(T const &evt)
	{
	}

	void _do_quit(quit const &evt)
	{
		assert(_impl);
		_impl->_do_quit(evt);
	}
	
	template<typename T>
	void _do_init(T const &evt)
	{
		assert(_impl);
		//_impl->_do_init(evt);
	}

	template<typename T>
	void _do_init_requested(T const &evt)
	{
		assert(_impl);
	}

	
	template<typename T>
	void _do_load_requested(T const &evt)
	{
		assert(_impl);
	}

	void _report_error(timer_expired const &evt)
	{
		_impl->_report_error(evt);
	}
	

	template<typename T>
	bool is_rendering(T const &evt)
	{
		assert(_impl);
		return _impl->is_rendering();
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
	/// Doing nothing at the moment
	struct Resting : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {} // {std::clog << "Server FSM : entering: Resting" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {} // {std::clog << "Server FSM : leaving: Resting" << std::endl;}
	};
	// State while the client has not yet received configuration
	// project or environment which was requested
	// this state does not allow rendering to take place
	// but allows both errors and quit messages
	struct Waiting : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: WaitingInit" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: WaitingInit" << std::endl;}
	};
	// State when we are loading environment
	struct Initing : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Initing" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Initing" << std::endl;}
	};
	/// State when we are loading project
	struct Loading : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Initing" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Initing" << std::endl;}
	};

	/*
	struct StartDraw : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: StartDraw" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: StartDraw" << std::endl;}
	};
	*/

	/// Application states
	// The application has not yet been started, initial state
	struct NotRunning : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Terminated" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Terminated" << std::endl;}
	};
	// The application is running fine
	struct Running : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Terminated" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Terminated" << std::endl;}
	};
	/// Exit state
	struct Terminated : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Terminated" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Terminated" << std::endl;}
	};

	/// Rendering states
	struct NotRendering : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Error" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Error" << std::endl;}
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


	/// Error states
	struct AllOk : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Error" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Error" << std::endl;}
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
	// defining a tupple where we have the running and exit states in other region
	// Data passing in one
	// the Rendering logic in one
	// and Lastly the Application state in one
	typedef mpl::vector<Unknown, NotRendering, AllOk, NotRunning> initial_state;

	typedef ClientFSM_ s;
struct transition_table : vl::mpl::vector<
//    Start     Event        Target      Action                      Guard 
//   +---------+------------+-----------+---------------------------+----------------------------+ 
// @todo rendering and data passing are not mutually exclusive really
// so should we combine them?
// or should we just handle the rendering loop as usual but not really render?
// or use guards for that?
// Rendering
  row< NotRendering , update		,	Updating    , &s::_do_update , &s::is_rendering >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Updating, render				,	Rendering   , &s::_do_render , &s::is_rendering >,
a_row< Updating, timer_expired		,	Error		, &s::_report_error >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Rendering , swap				,	Swapping    , &s::_do_swap   , &s::is_rendering >,
a_row< Rendering , timer_expired	,	Error		, &s::_report_error >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Swapping , swap_done			,	NotRendering, &s::_do_swap_done , &s::is_rendering >,

// Data passing
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Initing , load				,	Loading		, &s::_do_load  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Loading , load_done			,	Resting		, &s::_do_rest  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Resting, init_requested		,	Waiting		, &s::_do_init_requested >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Waiting, init_received		,	Resting		, &s::_do_init >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Resting, load_requested		,	Waiting		, &s::_do_load_requested >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Waiting, load_received		,	Loading		, &s::_do_load >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
// Application logic
// @todo exit state should be available always, I think there is a short hand for this
a_row< NotRunning,	quit	,	Terminated		, &s::_do_quit >,
a_row< NotRunning,	start	,	Running			, &s::_do_start >,
a_row< Running,		quit	,	Terminated		, &s::_do_quit >,

//   +---------+------------+-----------+---------------------------+----------------------------+ 

// Error handling
a_row< AllOk , timer_expired		,	Error		, &s::_report_error >
// @todo missing error handled
//a_row< Error	, clear_error		,	Resting		, &s::_do_rest >
> {};

    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::clog << "no transition from state " << state
            << " on event " << typeid(e).name() << std::endl;
    }

};	// class ClientFSM_

}	// namespace cluster

}	// namespace vl

#endif // HYDRA_CLUSTER_STATES_HPP
