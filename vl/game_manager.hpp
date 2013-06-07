/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file game_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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

#include "program_options.hpp"

#include "profiler_report.hpp"
// Necessary for LOADER_FLAGS
#include "flags.hpp"
// Necessary for EyeTracker
#include "eye_tracker.hpp"
// Necessary for creating a proper implementation of callback functor
#include "cad_importer.hpp"

namespace vl
{

// SM events
struct init
{
	init(vl::config::EnvSettingsRefPtr env)
		: environment(env)
	{}

	vl::config::EnvSettingsRefPtr environment;
};

struct play {};
struct quit {};
struct stop {};
struct pause {};
struct unload {};
struct load
{
	load(std::string const &project_file, std::string const &global_file, LOADER_FLAGS f = LOADER_FLAG_NONE)
		: project(project_file)
		, global(global_file)
		, flags(f)
	{}

	std::string project;
	std::string global;
	LOADER_FLAGS flags;
};

// Forward declaration
struct GameManagerFSM_;
// Pick a back-end
typedef vl::msm::back::state_machine<GameManagerFSM_> GameManagerFSM;

/** @class GameManager
 *	@brief Main manager that handles resource loading a simulation
 *	states
 *
 *	Following up coming changes will break compatibility
 *	@todo break into Program manager and Simulation (game) manager
 *	Program manager handles the running of the program, i.e. exiting 
 *	starting/stopping simulations
 *	Simulation manager handles the running of a individual simulation
 *	Only one simulation can be ran at a time.
 *	Object creation belongs to Simulation manager
 *	Project loading replaced by loading a simulation
 *	Global resources are loaded by Program manager and can not be reset
 *	Project resources are loaded when a simulation is created
 *
 *	@todo support for multiple projects removed
 *	One global configuration for setting up shared resources
 *	One project configuration that defines the simulation
 *	Sharing resources need to be either done using global resources
 *	python modules etc. or if a more complex system is necessary we
 *	can implement specific systems to allow resource sharing for some
 *	resources.
 *
 */
class HYDRA_API GameManager 
{
	typedef boost::signal<void (void)> StateChanged;
public :
	/// @brief constructor
	/// @param session the distributed session where to register objects to
	/// @param logger an instance of Logging class where all logging is redirected
	/// this one should usually be the only one
	/// @todo add passing of the exe directory (for python modules)
	GameManager(vl::Session *session, vl::Logger *logger, vl::ProgramOptions const &opt);

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

	vl::ProfilerReport &getRenderingReport(void)
	{ return _rendering_report; }

	vl::Report<vl::time> &getInitReport(void)
	{ return _init_report; }

	/// @brief Step the simulation forward
	void step(void);

	vl::ClientsRefPtr getTrackerClients( void )
	{ return _trackers; }

	vl::Logger *getLogger(void)
	{ return _logger; }

	EyeTrackerRefPtr getEyeTracker(void)
	{ return _eye_tracker; }

	/// @brief creates a new GameObject
	/// @return new GameObject if none with that name exist, otherwise already existing object
	/// @param name the name of the GameObject to create
	GameObjectRefPtr createGameObject(std::string const &name);

	GameObjectRefPtr createDynamicGameObject(std::string const &name);

	void removeGameObject(GameObjectRefPtr obj);

	bool hasGameObject(std::string const &name);

	GameObjectRefPtr getGameObject(std::string const &name);

	GameObjectList const &getGameObjectList(void) const
	{ return _game_objects; }

	/// ------------------------------ Kinematics ----------------------------
	KinematicWorldRefPtr getKinematicWorld(void)
	{ return _kinematic_world; }
	
	/// ------------------------------ Physics -------------------------------
	/// Get the physics World
	/// IF physics has not been enabled returns zero
	physics::WorldRefPtr getPhysicsWorld(void)
	{ return _physics_world; }

	/// Enable physics to be used in this game
	/// First call to this function will create the physics world with true
	/// After that this function is a No OP.
	/// Calling this with false is a No OP.
	/// Parameters : enable, true to enable physics
	void enablePhysics( bool enable );

	bool isPhysicsEnabled(void)
	{ return(_physics_world != 0); }


	/// Timers
	vl::time getProgramTime(void) const
	{ return _program_timer.elapsed(); }

	vl::time getGameTime(void) const
	{ return _game_timer.elapsed(); }

	/// State management
	
	/// @brief quit the game/simulation
	/// Short hand for state change request
	/// End state is QUIT
	void quit(void);

	/// @brief pause the game/simulation
	/// Short hand for state change request
	/// End state is PAUSE
	void pause(void);

	/// @brief start or resume the game/simulation
	/// Short hand for state change request
	/// End state is PLAY
	void play(void);

	/// @brief Stop the simulation, ready for restart or unloading
	/// @todo should clear all changes made during simulation
	/// End state is STOP
	void stop(void);

	/// @brief Unloads the current project and global
	/// Always unloads both project and global so both need to be reloaded
	/// End state is INITING
	/// @todo after calling unload calling load immediately will cause
	/// undefined behaviour (possible crashes). Please let the mainloop
	/// roll at least once before calling load after this.
	void unload(void);

	/// @brief Loads new project and global
	/// At the moment both need to be loaded, you can't just use new project
	/// @param project config file name
	/// @param global config file name
	/// Valid states for calling this INITING
	/// End state is STOP unless autoplay is true then PLAY
	void load(std::string const &project, std::string const &global);

	/// @brief unload, load rolled into one
	/// Mostly for testing that unload and load works.
	/// Valid states for calling this INITING
	/// End state is STOP
	/// @todo this does not unload
	/// Usage at the moment is unload followed by reload
	/// This is because of technical limitations
	void reload(void);

	/// @brief stops the current simulation and returns it to the initial state
	/// Clears all changes done by the simulation and returns to initial state
	/// which is set when the project is loaded.
	/// Reloads the project files (loading them from the disc) and overwrites all
	/// changes made by the simulation then
	/// resets the python context and reruns all python scripts.
	/// End state is PLAY or PAUSE depending on auto_run
	///
	/// @note Does not reload project files, this feature may be added later.
	void restart(void);

	bool isQuited(void) const;
	bool isPaused(void) const;
	bool isPlaying(void) const;
	bool isStopped(void) const;

	std::string getStateName(void) const;

	/// @brief get the current project and global settings
	/// Mostly usefull for project change callbacks
	vl::Settings getSettings(void) const;

	/// Scene handling

	// @todo loadScene should be removed and use the load state instead
	/// @brief Load all scenes for the project configuration
	void loadScenes(vl::ProjSettings const &proj, LOADER_FLAGS flags = LOADER_FLAG_NONE);

	/// @brief Load a scene from the scene configuration
	void loadScene(vl::SceneInfo const &scene_info, LOADER_FLAGS flags = LOADER_FLAG_NONE);

	/// @brief filename based scene loading
	/// @param file_name
	/// @param flags for controlling loading process, only available for HSF format
	/// Two separate versions instead of automatic overloading for easier python integration
	/// @todo can we use a full system path here or not?
	void loadScene(std::string const &file_name, LOADER_FLAGS flags);
	void loadScene(std::string const &file_name)
	{ loadScene(file_name, LOADER_FLAG_NONE); }

	/// @brief saves the scene file
	/// This does not respect any of the Scene configuration and will always
	/// save all the objects to one scene file.
	/// Works only for GameObjects and files read from HSF 
	/// old .scene format is not supported.
	/// @todo this should be replaced with save project
	void saveScene(std::string const &file_name);

	vl::time const &getDeltaTime(void) const
	{ return _delta_time; }

	vrpn_analog_client_ref_ptr createAnalogClient(std::string const &name);

	/// @todo this is bad
	/// We can not allow reseting the options like this.
	/// We need to either use a init function that takes these options as a parameter
	/// or we need to pass them to the Constructor.
	/// After they have been set in init they can then be edited using getOptions.
	/// setOptions()

	CadImporterRefPtr getCadImporter(void)
	{ return _cad_importer; }

	vl::ProgramOptions const &getOptions(void) const
	{ return _options; }
	vl::ProgramOptions &getOptions(void)
	{ return _options; }

	vl::ProjSettings const &getProjectSettings(void) const
	{ return _loaded_project; }

	vl::ProjSettings const &getGlobalSettings(void) const
	{ return _global_project; }

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

	/// @internal
	/// @brief Test function
	/// This function destroys all dynamic objects (created from python)
	/// If python context is not also cleared this will most likely
	/// cause crashing because of dangling pointers.
	/// DO NOT USE
	void _destroyDynamicObjects(void);

	/// @internal
	/// @brief Reset python context and ran all python functions again
	/// Used for testing before properly implementing GameManager::reset
	/// DO NOT USE
	void _rerunPythonScripts(void);

private :
	/// Non copyable
	GameManager( GameManager const &);
	GameManager & operator=( GameManager const &);

	vl::GameObjectRefPtr _createGameObject(std::string const &name, bool dynamic);

	/// Main loading functions use configurations files
	void _loadEnvironment(vl::config::EnvSettings const &env);

	/// @brief loads a new global configuration
	/// This will remove the old global and reset the python context
	void _loadGlobal(std::string const &file_name);

	/// @todo this needs to be moved to State Machine which handles loading
	/// and this method needs to be private
	/// @brief remove all loaded resources and destroy objects
	/// Resets python context, destroys all objects (bodies, gameobjects, scenenodes),
	/// removes all loaded resources, removes all resources paths, 
	/// removes all event triggers.
	/// Usefull when reloading all projects
	///
	/// Does not remove any managers only simulation objects
	/// that are created from either project configs or python.
	/// Does also not remove any objects created from environment config,
	/// changing environment config is not supported, the program needs to be
	/// restarted.
	/// This might be changed in the future but more likely change is to allow
	/// changing individual parameters and saving them to env config.
	/// 
	/// This is a hack to get project reloading working.
	/// Will be replaced by more sophisticated functionality when new
	/// simulation manager is implemented.
	void _removeAll(void);

	/// @todo this takes over 1 second to complete which is almost a second too much
	void _createTrackers(vl::config::EnvSettings const &env);

	void _setupResources(vl::config::EnvSettings const &env);

	void _addResources(vl::ProjSettings const &proj);

	void _removeResources(vl::ProjSettings const &proj);
	
	void _addPythonScripts(vl::ProjSettings const &proj);

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

	CadImporterRefPtr _cad_importer;

	/// Tracking
	vl::ClientsRefPtr _trackers;

	vl::gui::GUIRefPtr _gui;

	vl::ProfilerReport _rendering_report;
	vl::Report<vl::time> _init_report;

	vl::Logger *_logger;

	vl::MeshManagerRefPtr _mesh_manager;
	vl::MaterialManagerRefPtr _material_manager;

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

	EyeTrackerRefPtr _eye_tracker;

	vl::ProjSettings _loaded_project;
	vl::ProjSettings _global_project;

	bool _auto_start;

	vl::ProgramOptions _options;

	GameObjectList _game_objects;

	std::string _old_project;
	std::string _old_global;

	// signals
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

	void _do_unload(vl::unload const &evt);

};	// class GameManager


class CadImporterCallback : public ImporterCallback
{
public :

	// @todo add create method that creates ref counted version of this
	// also remove public constructor after adding the method
	static CadImporterCallback *create(GameManager *man)
	{
		return new CadImporterCallback(man);
	}

private :
	CadImporterCallback(GameManager *man)
		: _game(man)
	{}

	// Overload the private load function
	void _load(std::string const &filename, std::string const &file_path);

	GameManager *_game;
};



/// @class GameManagerSM
/// the internal state machine for GameManager
struct GameManagerFSM_ : public msm::front::state_machine_def<GameManagerFSM_, vl::state>
{
	/// Experimental solution for exceptions to interrupt the state machine
	typedef int no_exception_thrown; 

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

	void _do_unload(unload const &evt)
	{
		assert(_impl);
		_impl->_do_unload(evt);
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
/// Load environment config and create necessary objects
/// Init should never be called more than once per program run
a_row< Unknown , init		 ,	Initing    , &g::_do_init  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
/// Load global and project
a_row< Initing , load		 ,	Loading    , &g::_do_load  >,
a_row< Initing , stop		 ,	Stopped    , &g::_do_stop  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
  row< Loading , vl::none	 ,  Stopped    , &g::_do_stop , &g::no_auto_start >,
  row< Loading , vl::none	 ,  Playing	   , &g::_do_play , &g::auto_start >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Stopped , unload      ,  Initing    , &g::_do_unload  >,
a_row< Stopped , play        ,  Playing    , &g::_do_play  >,
a_row< Stopped , pause		 ,  Paused     , &g::_do_pause >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Playing , stop        ,  Stopped    , &g::_do_stop  >,
a_row< Playing , pause       ,  Paused     , &g::_do_pause >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
a_row< Paused  , play		 ,  Playing    , &g::_do_play  >,
a_row< Paused  , stop        ,  Stopped    , &g::_do_stop  >,
//   +---------+------------+-----------+---------------------------+----------------------------+ 
// @todo should be replaced with a second FSM so we can always exit
a_row< Initing , quit        ,  Quited    , &g::_do_quit  >,
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
	/*
	template <class Fsm,class Event>
	void exception_caught(Event const& ,Fsm&, std::exception &e)
	{
		std::clog << "exception : " << e.what() << std::endl;
		boost::exception_ptr ex = boost::current_exception();
		boost::rethrow_exception(ex);
	}
	*/

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
