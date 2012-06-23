/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file cluster/states.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */


#ifndef HYDRA_CLUSTER_STATES_HPP
#define HYDRA_CLUSTER_STATES_HPP

#include <stdint.h>

#include "base/state_machines.hpp"

// General callback functions
#include <boost/function.hpp>

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
		, has_init(false), frame(-1), update_frame(-1)
		, create_frame(-1), rendering_state(CS_CLEAR)
		, shutdown(false)
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
	int64_t update_frame;
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
namespace event
{
struct frame_event
{
	frame_event(uint32_t frame_, vl::time const &timestamp_)
		: frame(frame_), timestamp(timestamp_)
	{}

	uint32_t frame;
	vl::time timestamp;
	boost::function<void (void)> callback;
};

struct start_frame : public frame_event
{
	start_frame(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};
struct frame_done : public frame_event
{
	frame_done(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};

struct render : public frame_event
{
	render(uint32_t frame_, vl::time const &timestamp_, vl::time const &limit = vl::time(1, 0))
		: frame_event(frame_, timestamp_)
		, timelimit(limit)
	{}

	vl::time timelimit;
};
struct swap : public frame_event
{
	swap(uint32_t frame_, vl::time const &timestamp_, vl::time const &limit = vl::time(1, 0))
		: frame_event(frame_, timestamp_)
		, timelimit(limit)
	{}

	vl::time timelimit;
};
// Update should handle both sending of create message and sending of update message
struct update : public frame_event
{
	update(uint32_t frame_, vl::time const &timestamp_, vl::time const &limit = vl::time(1, 0))
		: frame_event(frame_, timestamp_)
		, timelimit(limit)
	{}

	vl::time timelimit;


};

struct swap_done : public frame_event
{
	swap_done(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};

struct quit {};
struct load {};
struct load_done {};
struct init_done {};
struct start {};
struct init_requested {};
struct load_requested {};
struct timer_expired {};
struct clear_error {};
struct init {};
struct graph_requested {};
struct graph_loaded {};


// Events
// @todo move to top side and use common naming scheme
struct update_requested : public frame_event
{
	update_requested(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};
struct update_done : public frame_event
{
	update_done(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};
struct draw_requested : public frame_event
{
	draw_requested(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};
struct draw : public frame_event
{
	draw(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};
struct draw_done : public frame_event
{
	draw_done(uint32_t frame_, vl::time const &timestamp_)
		: frame_event(frame_, timestamp_)
	{}
};

}	// namespace event
/// End events

/// Flags. Allow information about a property of the current state
/// Flags can not be used for directly at least for dynamic behaviour
/// or we need to divide the FSM into different parts based on the
/// dynamic behaviour. For example init part and running part
/// this is because a flag is a static data of a state so one state
/// will always have a certain flag enabled.
/// Might not be a bad idea though dividing state machines.
struct RenderingFlag {};
struct UpdateDoneFlag {};
struct DrawStartedFlag {};
struct NotRenderingFlag {};


/// States

/// Rendering states
// Doing nothing
struct NotRendering : public vl::msm::front::state<vl::state>
{
};

/// Updating the scene graph
struct Updating : public vl::msm::front::state<vl::state>
{
};

/// Rendering the scene
struct Rendering : public vl::msm::front::state<vl::state>
{
};

/// Swapping front and back buffers
struct Swapping : public vl::msm::front::state<vl::state>
{
};

/// States used in Client::RenderingFSM
struct FrameDone : public vl::msm::front::state<vl::state>
{
	typedef mpl::vector3<NotRenderingFlag, UpdateDoneFlag, DrawStartedFlag> flag_list;
};

struct FrameStarted : public vl::msm::front::state<vl::state> 
{
};

struct DrawReady : public vl::msm::front::state<vl::state>
{
	typedef mpl::vector1<UpdateDoneFlag> flag_list;
};

struct DrawSent : public vl::msm::front::state<vl::state>
{
	typedef mpl::vector2<UpdateDoneFlag, DrawStartedFlag> flag_list;
};

struct Drawing : public vl::msm::front::state<vl::state>
{
	typedef mpl::vector2<UpdateDoneFlag, DrawStartedFlag> flag_list;
};

/// Application states
// The application is running fine
// bot starting state and the normal operation state
struct Running : public vl::msm::front::state<vl::state>
{
	// every (optional) entry/exit methods get the event passed.
	template <class Event,class FSM>
	void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Running" << std::endl;}
	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Running" << std::endl;}
};
/// Exit state
// @todo this should be separated to two states Terminate Requested and Terminated
// Terminated is the state when the Application (or a remote Application) is really terminated
// Terminate Requested is the state where the Termination has been invoked and can not
// be reversed, but it has not yet been completed.
// The divide is mostly useful for Remote Applications where we need to wait till they
// terminate before terminating our selves.
struct Terminated : public msm::front::terminate_state<> 
{
	// every (optional) entry/exit methods get the event passed.
	template <class Event,class FSM>
	void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Terminated" << std::endl;}
};

/// Error states
struct AllOk : public vl::msm::front::state<vl::state>
{
	// every (optional) entry/exit methods get the event passed.
	template <class Event,class FSM>
	void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: AllOk" << std::endl;}
	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: AllOk" << std::endl;}
};
struct Error : public msm::front::interrupt_state<event::clear_error> 
{
	// every (optional) entry/exit methods get the event passed.
	template <class Event,class FSM>
	void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Error" << std::endl;}
	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Error" << std::endl;}
};

/// End States

// @todo remove, short hand so we don't need to fix all the events
using namespace vl::cluster::event;
/// New FSM Client
class ClientFSM_ :  public msm::front::state_machine_def<ClientFSM_, vl::state> 
{
public :
	ClientFSM_(void)
		: _frame(0)
		, _ready_for_rendering(false)
		, _ready_for_updates(false)
	{}

	template <class Event,class FSM>
	void on_entry(Event const& ,FSM&) 
	{
		std::cout << "entering: Client FSM" << std::endl;
	}

	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ) 
	{
		std::cout << "leaving: Client FSM" << std::endl;
	}

	/// State machine functions
	/// Template method patter, these call pure virtual functions
	/// @note If made pure virtual visual compiler crashes
	template<typename T>
	void _rest(T const &evt)
	{
		_do_rest();
	}

	void _quit(quit const &evt)
	{
		_do_quit(evt);
	}

	void _init(init_done const &evt) 
	{
		_do_init(evt);
	}
	void _load(load_done const &evt)
	{
		_ready_for_updates = true;
		_do_load(evt);
	}

	void _init_requested(init_requested const &evt)
	{
		_do_init_requested(evt);
	}
	void _load_requested(load_requested const &evt)
	{
		_do_load_requested(evt);
	}

	void _graph_requested(graph_requested const &evt)
	{
		_do_graph_requested(evt);
	}

	void _graph_load(graph_loaded const &evt)
	{
		_do_graph_load(evt);
	}

	void _start_frame(start_frame const &evt)
	{
		assert(_frame < evt.frame || _frame == 0);
		_frame = evt.frame;
		_do_start_frame(evt);
	}

	void _frame_done(frame_done const &evt)
	{
		_do_frame_done(evt);
	}

	void _handle_error(timer_expired const &evt)
	{
		_do_handle_error(evt);
	}

	void _graph_loaded(graph_loaded const &evt)
	{
		_ready_for_rendering = true;
	}

	template<typename T>
	bool _is_ready_for_rendering(T const &evt)
	{
		return is_ready_for_rendering();
	}

private :
	/// Pure Virtual
	virtual void _do_rest(void) = 0;
	virtual void _do_quit(quit const &evt) = 0;

	virtual void _do_init(init_done const &evt) = 0;
	virtual void _do_load(load_done const &evt) = 0;

	virtual void _do_init_requested(init_requested const &evt) = 0;
	virtual void _do_load_requested(load_requested const &evt) = 0;

	virtual void _do_graph_requested(graph_requested const &evt) = 0;
	virtual void _do_graph_load(graph_loaded const &evt) = 0;

	virtual void _do_start_frame(start_frame const &evt) = 0;
	virtual void _do_frame_done(frame_done const &evt) = 0;

	virtual void _do_handle_error(timer_expired const &evt) = 0;

public :
	/// Guards
	/// Return true if environment, project, zero frame create message and MSG_SG_INIT
	/// has been received.
	/// @todo this doesn't take into account wether the client wants to render or not
	/// (is rendering enabled in inherited class)
	bool is_ready_for_rendering(void) const
	{ return _ready_for_rendering; }

	bool is_ready_for_updates(void) const
	{ return _ready_for_updates; }

	// Data
protected :
	uint32_t _frame;
	bool _ready_for_rendering;
	bool _ready_for_updates;

	// States
public :
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
	struct Resting : public vl::msm::front::state<vl::state> {};

	// State while the client has not yet received configuration
	// project or environment which was requested
	// this state does not allow rendering to take place
	// but allows both errors and quit messages
	struct Waiting : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Waiting" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Waiting" << std::endl;}
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
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: Loading" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: Loading" << std::endl;}
	};

	/// State when we are loading SceneGraph
	struct GraphLoading : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::clog << "Server FSM : entering: GraphLoading " << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::clog << "Server FSM : leaving: GraphLoading " << std::endl;}
	};

	/// Rendering FSM
	/// @todo move to top level so we can do separate instances for
	/// both Server and Client rendering loops
	struct Rendering_ : public msm::front::state_machine_def<Rendering_, vl::state>
	{
		// Actions
		void _frame_start(start_frame const &evt) {}

		void _update_requested(update_requested const &evt)
		{
		}

		void _update(update const &evt)
		{
			// Dirty way to handle the event
			evt.callback();
		}

		void _update_done(update_done const &evt)
		{
			// Dirty way to handle the event
			evt.callback();
		}
		
		void _draw_requested(draw_requested const &evt) {}

		void _draw(draw const &evt) {}
		
		void _draw_done(draw_done const &evt) {}

		// Guards
		
		typedef FrameStarted initial_state;

		typedef Rendering_ r;

		/// @todo add Guards to protect from incorrect frame numbers
		/// These would invoke an error report to user but otherwise just fail
		/// without changing the state (similar to the one in Server atm)
		struct transition_table : vl::mpl::vector<
		//      Start     Event         Next      Action               Guard
		//   +---------+------------+-----------+---------------------------+
		// MSG_FRAME_START,	Sent from master to start a frame loop
		// This is not necessary as the Main state machine moves to this machine
		// from MSG_FRAME_START
		//a_row< NotRendering ,	start_frame		,	FrameStarted	, &r::_frame_start >,
		//   +---------+------------+-----------+---------------------------+
		// MSG_REQ_SG_UPDATE, sent from slave to get MSG_SG_UPDATE
		a_row< FrameStarted	,	update_requested,	Updating	, &r::_update_requested >,
		//   +---------+------------+-----------+---------------------------+
		// MSG_SG_UPDATE, sent from master
		//a_row< UpdateRequested	,	update		,	Updating		, &r::_update >,
		//   +---------+------------+-----------+---------------------------+
		// MSG_DRAW_READY, sent from slave that it's ready for MSG_DRAW
		a_row< Updating	,	update_done		,	DrawReady		, &r::_update_done >,
		//   +---------+------------+-----------+---------------------------+
		// MSG_DRAW, sent from master
		a_row< DrawReady	,	draw_requested	,	DrawSent		, &r::_draw_requested >,
		//   +---------+------------+-----------+---------------------------+
		// MSG_DRAWING, sent from slave that it started rendering
		a_row< DrawSent		,	draw			,	Drawing			, &r::_draw >,
		//   +---------+------------+-----------+---------------------------+
		// MSG_DRAW_DONE, sent from slave that it finished rendering
		a_row< Drawing		,	draw_done		,	FrameDone    , &r::_draw_done  >
		//   +---------+------------+-----------+---------------------------+
		> {};
	};
	

	// back-end for submachine
	typedef msm::back::state_machine<Rendering_> Rendering;

	// the initial state of the FSM. Must be defined
	// defining a tupple where we have the running and exit states in other region
	// Data passing in one
	// the Rendering logic in one
	// and Lastly the Application state in one
	typedef mpl::vector<Resting, AllOk, Running> initial_state;

	typedef ClientFSM_ s;
struct transition_table : vl::mpl::vector<
//    Start     Event        Target      Action                      Guard 
//   +---------+------------+-----------+---------------------------+----------------------------+ 
// Data passing
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Resting,		init_requested	,	Initing			, &s::_init_requested >,
a_row< Resting,		load_requested	,	Loading			, &s::_load_requested >,
a_row< Resting,		graph_requested ,	GraphLoading	, &s::_graph_requested >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Loading		,	load_done		,	Resting			, &s::_rest  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Initing		,	init_done		,	Resting			, &s::_rest >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< GraphLoading	,	graph_loaded	,	Resting			, &s::_graph_loaded >,

// @todo add MSG_SG_CREATE
// @todo add MSG_RESOURCE
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Resting	,	start_frame		,	Rendering	, &s::_start_frame,  &s::_is_ready_for_rendering >,
a_row< Rendering,	frame_done		,	Resting		, &s::_frame_done >,

//   +---------+------------+-----------+---------------------------+----------------------------+ 
// Application logic
a_row< Running	,	quit			,	Terminated		, &s::_quit >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 

// Error handling
a_row< AllOk	,	timer_expired	,	Error		, &s::_handle_error >,
// @todo missing error handled
a_row< Error	,	clear_error		,	AllOk		, &s::_rest >
> {};

	// Replaces the default no-transition response.
	template <class FSM,class Event>
	void no_transition(Event const& e, FSM&, int state)
	{
		std::clog << "ClientFSM : no transition from state " << state
			<< " on event " << typeid(e).name() << std::endl;
	}

};	// class ClientFSM_

}	// namespace cluster

}	// namespace vl

#endif // HYDRA_CLUSTER_STATES_HPP
