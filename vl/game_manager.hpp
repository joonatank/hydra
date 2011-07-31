/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file game_manager.cpp
 *
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

#ifndef VL_GAME_MANAGER_HPP
#define VL_GAME_MANAGER_HPP

#include "typedefs.hpp"

#include "tracker.hpp"

// TODO this should not need to be included here
#include "tracker_serializer.hpp"

#include "session.hpp"

#include "stats.hpp"
#include "base/timer.hpp"
#include "logger.hpp"

// Necessary for SceneInfo
#include "base/projsettings.hpp"

// Audio
#include <cAudio/cAudio.h>

namespace vl
{

/// @struct Weather
/// @brief definition for the weather
/// @todo Not implemented yet
struct Weather
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
struct Date
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

// Only one state can be active at a time
// Do not change the order Init needs to be lower than other states
enum GAME_STATE
{
	GS_UNKNOWN = 0,	// Starting state
	GS_INIT = 1,	// Game initialised
	GS_PLAY = 2,	// Game played
	GS_PAUSE = 3,	// Game paused
	GS_QUIT = 4,	// Game quited
};

/** @class GameManager
 */
class GameManager
{
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

	vl::gui::GUIRefPtr getGUI(void)
	{ return _gui; }

	vl::Stats &getStats(void)
	{ return _stats; }

	void toggleBackgroundSound( void );

	/// @brief Step the simulation forward
	/// @return true if the simulation is still running, false if it's not
	bool step( void );

	vl::ClientsRefPtr getTrackerClients( void )
	{ return _trackers; }

	/// @brief Enable disable audio rendering, for now disable does not work
	/// @param enable if true creates the audio context, false is ignored
	void enableAudio(bool enable);

	bool isAudioEnabled(void)
	{ return(_audio_manager != 0); }

	void createBackgroundSound( std::string const &name );

	vl::Logger *getLogger(void)
	{ return _logger; }

	void addInputDevice(InputDeviceRefPtr dev);

	/// ------------------------------ Constraints ---------------------------
	void addConstraint(vl::ConstraintRefPtr constraint);

	void removeConstraint(vl::ConstraintRefPtr constraint);

	bool hasConstraint(vl::ConstraintRefPtr constraint) const;

	ConstraintSolverRefPtr getConstraintSolver(void)
	{ return _constraint_solver; }

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

	/// @return true if the change is allowed
	bool requestStateChange(GAME_STATE state);
	
	/// @brief quit the game/simulation
	/// Short hand for state change request
	void quit(void)
	{ requestStateChange(GS_QUIT); }

	bool isQuited(void) const
	{ return _state == GS_QUIT; }

	/// @brief pause the game/simulation
	/// Short hand for state change request
	void pause(void)
	{ requestStateChange(GS_PAUSE); }

	bool isPaused(void) const
	{ return _state == GS_PAUSE; }

	void play(void)
	{ requestStateChange(GS_PLAY); }

	bool isPlayed(void) const
	{ return _state == GS_PLAY; }

	GAME_STATE getState(void) const
	{ return _state; }

	bool isInited(void) const
	{ return _state > GS_INIT; }

	RecordingRefPtr loadRecording(std::string const &path);

	/// Project handling
	/// @todo should the file already be loaded or not?
	///void loadProject(std::string const &file_name);

	/// Scene handling
	void loadScene(vl::SceneInfo const &scene_info);

private :
	/// Non copyable
	GameManager( GameManager const &);
	GameManager & operator=( GameManager const &);

	/// Session is not valid in the constructor because 
	/// GameManager is created from sessions constructor
	/// so the init function is provided for creating distributed objects.
	/// Called when user initiates INIT state change.
	void _init(void);

	SceneManagerPtr _createSceneManager(void);

	PlayerPtr _createPlayer(void);

	void _process_constraints(vl::time const &t);

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

	vl::Stats _stats;

	/// Audio objects
	cAudio::IAudioManager *_audio_manager;
	cAudio::IAudioSource *_background_sound;

	vl::Logger *_logger;

	vl::MeshManagerRefPtr _mesh_manager;
	bool _env_effects_enabled;
	Weather _weather;
	Date _date;

	/// Timers
	vl::timer _program_timer;
	vl::stop_timer _game_timer;
	vl::timer _step_timer;

	/// State
	GAME_STATE _state;

	/// Physics
	physics::WorldRefPtr _physics_world;

	/// Non physics constraints
	ConstraintSolverRefPtr _constraint_solver;

	std::vector<InputDeviceRefPtr> _input_devices;

};	// class GameManager

}	// namespace vl

#endif // VL_GAME_MANAGER_HPP
