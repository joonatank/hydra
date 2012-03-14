/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file game_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
 *	Game Manager
 *	Manages all the game manager and owns them
 *	Has methods to retrieve them
 *	Has methods to reset the game
 *	Manages the SoundManager
 *	Has methods to set background sound
 *	Has methods to toggle background sound on/off
 *
 *	Later
 *	Manages the active SceneManager
 *	Manages the physics world
 *
 *	Non copyable
 */

#ifndef HYDRA_GAME_MANAGER_HPP
#define HYDRA_GAME_MANAGER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include "typedefs.hpp"

#include "cluster/session.hpp"

#include "base/report.hpp"
#include "base/chrono.hpp"
#include "logger.hpp"

// Necessary for SceneInfo
#include "base/projsettings.hpp"

#include <boost/signal.hpp>

#include "base/state_machines.hpp"

namespace vl
{

/// @struct Weather
/// @brief definition for the weather
/// @todo Not implemented yet
struct HYDRA_API Weather
{
	Weather(void)
		: clouds(0), lighting(0), rain(0)
	{}

	uint16_t clouds;
	uint16_t lighting;
	uint16_t rain;
};

/// @struct Date
/// @brief Holds the current time and day
/// For now only time of day is implemented
struct HYDRA_API Date
{
	/// @brief Construct a time of day from string "hours:minutes"
	/// hours and minutes should both be integers 
	/// they should be withing limits, hours [0, 23] minutes [0, 59]
	/// The implementation can choose to handle incorrect format as it likes.
	Date(std::string const &time)
	{
		setTime(time);
	}

	Date(uint16_t h = 0, uint16_t m = 0)
		: hours(0), min(0)
	{
		setTime(h, m);
	}

	std::string getTime(void) const
	{
		std::stringstream ss;
		ss << hours << ":" << min;
		return ss.str();
	}

	/// @brief Set the time of day from string "hours:minutes"
	void setTime(std::string const &time)
	{
		std::stringstream ss(time);
		char tmp;
		uint16_t h, m;
		ss >> h >> tmp >> m;
		setTime(h, m);
	}

	/// @brief set time using hours and minutes
	/// @param h hours, is clamped to interval [0, 23]
	/// @param m minutes, is clamped to interval [0, 59]
	void setTime(uint16_t h, uint16_t m)
	{
		hours = h % 23;
		min = m % 59;
	}

	void addTime(uint16_t h, uint16_t m)
	{
		setTime(hours+h, min+m);
	}

	uint16_t hours;
	uint16_t min;
};


// SM events
struct init
{
	init(vl::config::EnvSettingsRefPtr env, std::string const global_file)
		: environment(env), global(global_file)
	{}

	vl::config::EnvSettingsRefPtr environment;
	std::string global;
};

struct play {};
struct quit {};
struct stop {};
struct pause {};
struct load
{
	load(std::string const project_file)
		: project(project_file)
	{}

	std::string project;
};

// Forward declaration
struct GameManagerFSM_;
// Pick a back-end
typedef vl::msm::back::state_machine<GameManagerFSM_> GameManagerFSM;

/** @class GameManager
 */
class HYDRA_API GameManager 
{
	typedef boost::signal<void (vl::Settings const &)> ProjectChanged;
	typedef boost::signal<void (void)> StateChanged;
public :
	/// @brief constructor
	/// @param session the distributed session where to register objects to
	/// @param logger an instance of Logging class where all logging is redirected
	/// this one should usually be the only one
	/// @todo add passing of the exe directory (for python modules)
	GameManager(vl::Session *session, vl::Logger *logger);

	virtual ~GameManager( void );

	vl::PythonContextPtr getPython(void)
	{ return _python; }

	PlayerPtr getPlayer(void)
	{ return _player; }

	ResourceManagerRefPtr getResourceManager(void)
	{ return _resource_man; }

	EventManagerPtr getEventManager(void)
	{ return _event_man; }

	vl::SceneManagerPtr getSceneManager(void)
	{ return _scene_manager; }

	MeshManagerRefPtr getMeshManager(void)
	{ return _mesh_manager; }
	
	MaterialManagerRefPtr getMaterialManager(void)
	{ return _material_manager; }

	vl::gui::GUIRefPtr getGUI(void)
	{ return _gui; }

	vl::Report<vl::time> &getRenderingReport(void)
	{ return _rendering_report; }

	vl::Report<vl::time> &getInitReport(void)
	{ return _init_report; }

	/// @brief Step the simulation forward
	void step(void);

	vl::ClientsRefPtr getTrackerClients( void )
	{ return _trackers; }

	vl::Logger *getLogger(void)
	{ return _logger; }

	/// ------------------------------ Kinematics ----------------------------
	KinematicWorldRefPtr getKinematicWorld(void)
	{ return _kinematic_world; }
	
	/// ------------------------------ Physics -------------------------------
	/// Get the physics World
	/// IF physics has not been enabled returns zero
	physics::WorldRefPtr getPhysicsWorld( void );

	/// Enable physics to be used in this game
	/// First call to this function will create the physics world with true
	/// After that this function is a No OP.
	/// Calling this with false is a No OP.
	/// Parameters : enable, true to enable physics
	void enablePhysics( bool enable );

	bool isPhysicsEnabled(void)
	{ return(_physics_world != 0); }

	/// Parameters that control all the scenes, 
	/// what they do is dependent on the implementation of the scene and they
	/// might be totally ignored.

	/// @brief enable/disable automatic environment modification of the scene
	/// Enabled by default
	/// If disabled setting weather, sky or time of day will not affect any
	/// of the scenes in the Game.
	/// Mind you these can also be disabled in the SceneManager and usually
	/// are better suited there, but this is provided so that the user can
	/// have a fine grain control over the game environment.
	void enableEnvironmentalEffects(bool enable);

	/// @brief Set the current weather
	/// These usually modify the sky, global lighting and possibly add effects
	void setWeather(Weather const &weather);

	/// @brief Get the current weather
	Weather const &getWeather(void) const
	{ return _weather; }

	/// @brief Set the current time of the day
	/// This usually modifies the sky, lighting (moon, stars, sun)
	void setTimeOfDay(Date const &date);

	Date const &getTimeOfDay(void) const
	{ return _date; }


	/// Timers
	vl::time getProgramTime(void) const
	{ return _program_timer.elapsed(); }

	vl::time getGameTime(void) const
	{ return _game_timer.elapsed(); }

	/// State management
	
	/// @brief quit the game/simulation
	/// Short hand for state change request
	void quit(void);

	/// @brief pause the game/simulation
	/// Short hand for state change request
	void pause(void);

	void play(void);

	void stop(void);

	void restart(void);

	bool isQuited(void) const;
	bool isPaused(void) const;
	bool isPlaying(void) const;

	void setupResources(vl::config::EnvSettings const &env);

	void addResources(vl::ProjSettings const &proj);

	void removeResources(vl::ProjSettings const &proj);

	/// Resource loading

	RecordingRefPtr loadRecording(std::string const &path);

	/// Project handling
	void loadProject(std::string const &file_name);

	/// @brief removes the current project and resets python context
	void removeProject(std::string const &name);

	void loadScenes(vl::ProjSettings const &proj);

	void loadScene(vl::SceneInfo const &scene_info);

	/// Test functions for Collada importer/exporter
	void loadScene(std::string const &file_name);
	void saveScene(std::string const &file_name);

	/// @todo not really working
	void unloadScene(std::string const &name);

	/// @todo not really working
	void unloadScenes(vl::ProjSettings const &proj);

	vl::time const &getDeltaTime(void) const
	{ return _delta_time; }

	void runPythonScripts(vl::ProjSettings const &proj);

	/// @todo this takes over 1 second to complete which is almost a second too much
	void createTrackers(vl::config::EnvSettings const &env);

	vrpn_analog_client_ref_ptr createAnalogClient(std::string const &name);

	int addProjectChangedListener(ProjectChanged::slot_type const &slot)
	{ _project_changed_signal.connect(slot); return 1; }

	template<typename T>
	void process_event(T const &evt);

	bool auto_start(void) const
	{ return _auto_start; }

	void set_auto_start(bool start)
	{ _auto_start = start; }

	// void addListener
	// @todo this should probably return the connection
	// @todo add a list of possible states for which we have listeners
	int addStateChangedListener(vl::state const &state, StateChanged::slot_type const &slot);

private :
	/// Non copyable
	GameManager( GameManager const &);
	GameManager & operator=( GameManager const &);

	/// Main loading functions use configurations files
	void _loadEnvironment(vl::config::EnvSettings const &env);

	/// @brief loads a new global configuration
	/// This will remove the old global and reset the python context
	void _loadGlobal(std::string const &file_name);

	/// Distributed object creation
	SceneManagerPtr _createSceneManager(void);
	PlayerPtr _createPlayer(void);

	/// Events
	void _createQuitEvent( void );

	void _fire_step_start(void);

	void _fire_step_end(void);

	// Where objects are registered
	vl::Session *_session;

	vl::PythonContextPtr _python;
	vl::ResourceManagerRefPtr _resource_man;
	vl::EventManagerPtr _event_man;
	vl::SceneManagerPtr _scene_manager;

	PlayerPtr _player;

	/// Tracking
	vl::ClientsRefPtr _trackers;

	vl::gui::GUIRefPtr _gui;

	vl::Report<vl::time> _rendering_report;
	vl::Report<vl::time> _init_report;

	vl::Logger *_logger;

	vl::MeshManagerRefPtr _mesh_manager;
	vl::MaterialManagerRefPtr _material_manager;

	bool _env_effects_enabled;
	Weather _weather;
	Date _date;

	/// Timers
	vl::chrono _program_timer;
	vl::stop_chrono _game_timer;
	vl::chrono _step_timer;
	vl::time _delta_time;

	/// Physics
	physics::WorldRefPtr _physics_world;

	/// Non physics constraints
	KinematicWorldRefPtr _kinematic_world;

	std::vector<vrpn_analog_client_ref_ptr> _analog_clients;

	vl::ProjSettings _loaded_project;
	vl::ProjSettings _global_project;

	bool _auto_start;

	// signals
	ProjectChanged _project_changed_signal;

	StateChanged _inited_signal;
	StateChanged _quited_signal;
	StateChanged _loaded_signal;
	StateChanged _played_signal;
	StateChanged _paused_signal;
	StateChanged _stopped_signal;

	/// Finite State Machine definition
	GameManagerFSM *_fsm;

/// Internal state machine
public :
	/// Session is not valid in the constructor because 
	/// GameManager is created from sessions constructor
	/// so the init function is provided for creating distributed objects.
	/// Called when user initiates INIT state change.
	void _do_init(vl::init const &evt);

	void _do_load(vl::load const &evt);

	void _do_play(vl::play const &evt);

	void _do_pause(vl::pause const &evt);

	void _do_stop(vl::stop const &evt);

	void _do_quit(vl::quit const &evt);

};	// class GameManager

/// @class GameManagerSM
/// the internal state machine for GameManager
struct GameManagerFSM_ : public msm::front::state_machine_def<GameManagerFSM_, vl::state>
{
	GameManager *_impl;

	GameManagerFSM_(void)
		: _impl(0)
	{}

	template <class Event,class FSM>
	void on_entry(Event const& ,FSM&) 
	{
		std::cout << "entering: GameManager FSM" << std::endl;
	}

	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ) 
	{
		std::cout << "leaving: GameManager FSM" << std::endl;
	}

	// The list of FSM states
	// Initial state
	struct Unknown : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Unknown" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Unknown" << std::endl;}
	};
	// State that is before we have loaded anything
	struct Initing : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Initing" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Initing" << std::endl;}
	};
	// State that is when we are still loading a project
	struct Loading : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Loading" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Loading" << std::endl;}
	};
	// State that is when we are in play
	struct Playing : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Playing" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Playing" << std::endl;}
	};
	struct Stopped : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Stopped" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Stopped" << std::endl;}
	};
	struct Paused : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Paused" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Paused" << std::endl;}
	};
	struct Quited : public vl::msm::front::state<vl::state>
	{
		// every (optional) entry/exit methods get the event passed.
		template <class Event,class FSM>
		void on_entry(Event const&,FSM& ) {std::cout << "entering: Quited" << std::endl;}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& ) {std::cout << "leaving: Quited" << std::endl;}
	};

	void setGameManager(GameManager *game)
	{ _impl = game; }

	void _do_init(init const &evt)
	{
		assert(_impl);
		_impl->_do_init(evt);
	}

	void _do_load(load const &evt)
	{
		assert(_impl);
		_impl->_do_load(evt);
	}

	void _do_play(play const &evt)
	{
		assert(_impl);
		_impl->_do_play(evt);
	}

	void _do_play(vl::none const &evt)
	{
		assert(_impl);
		_impl->_do_play(vl::play());
	}

	void _do_pause(pause const &evt)
	{
		assert(_impl);
		_impl->_do_pause(evt);
	}

	void _do_stop(stop const &evt)
	{
		assert(_impl);
		_impl->_do_stop(evt);
	}

	void _do_stop(vl::none const &evt)
	{
		assert(_impl);
		_impl->_do_stop(vl::stop());
	}

	void _do_quit(quit const &evt)
	{
		assert(_impl);
		_impl->_do_quit(evt);
	}

	bool auto_start(vl::none const &)
	{
		assert(_impl);
		return _impl->auto_start();
	}

	bool no_auto_start(vl::none const &)
	{
		assert(_impl);
		return !_impl->auto_start();
	}

	// the initial state of the FSM. Must be defined
	typedef Unknown initial_state;

	typedef GameManagerFSM_ g;
struct transition_table : vl::mpl::vector<
//    Start     Event        Target      Action                      Guard 
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Unknown , init		 ,	Initing    , &g::_do_init  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Initing , load		 ,	Loading    , &g::_do_load  >,
a_row< Initing , stop		 ,	Stopped    , &g::_do_stop  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Loading , vl::none	 ,  Stopped    , &g::_do_stop , &g::no_auto_start >,
  row< Loading , vl::none	 ,  Playing	   , &g::_do_play , &g::auto_start >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Stopped , play        ,  Playing    , &g::_do_play  >,
a_row< Stopped , pause		 ,  Paused     , &g::_do_pause >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Playing , stop        ,  Stopped    , &g::_do_stop  >,
a_row< Playing , pause       ,  Paused     , &g::_do_pause >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Paused  , play		 ,  Playing    , &g::_do_play  >,
a_row< Paused  , stop        ,  Stopped    , &g::_do_stop  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Paused  , quit        ,  Quited    , &g::_do_quit  >,
a_row< Stopped , quit        ,  Quited    , &g::_do_quit  >,
a_row< Playing , quit        ,  Quited    , &g::_do_quit  >
//   +---------+------------+-----------+---------------------------+----------------------------+ 
> {};

protected:
    /// Replaces the default no-transition response.
	/// Default is asserting false which is not exactly what we want
	/// better to report the error for user, might even use
	/// clog instead of cout so that they are debug messages.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cout << "no transition from state " << state
            << " on event " << typeid(e).name() << std::endl;
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

};	// struct GameManagerFSM_

}	// namespace vl

/// Templates
template<typename T>
void
vl::GameManager::process_event(T const &evt)
{
	std::clog << "vl::GameManager::process_event" << std::endl;
	_fsm->process_event(evt);
}

#endif // HYDRA_GAME_MANAGER_HPP
