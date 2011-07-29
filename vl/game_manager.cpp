/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file game_manager.cpp
 */

#include "game_manager.hpp"

// Python manager
#include "python.hpp"
// Event Manager
#include "event_manager.hpp"
// Resource Manager
#include "resource_manager.hpp"
// Mesh manager
#include "mesh_manager.hpp"
// Scene Manager
#include "scene_manager.hpp"

// Player
#include "player.hpp"

#include "gui/gui.hpp"

// Physics
#include "physics/physics_world.hpp"

#include "dotscene_loader.hpp"

#include "constraint_solver.hpp"

// Input devices
#include "input/input.hpp"

#include "recording.hpp"

vl::GameManager::GameManager(vl::Logger *logger)
	: _python(0)
	, _resource_man(new vl::ResourceManager)
	, _event_man(new vl::EventManager)
	, _scene_manager(0)
	, _player(0)
	, _trackers( new vl::Clients( _event_man ) )
	, _audio_manager(0)
	, _background_sound(0)
	, _logger(logger)
	, _env_effects_enabled(true)
	, _state(GS_UNKNOWN)
	, _constraint_solver(new vl::ConstraintSolver)
{
	_mesh_manager.reset(new MeshManager(new MasterMeshLoaderCallback(_resource_man)));
	_python = new vl::PythonContext( this );

	// Not creating audio context because user needs to enable it separately.

	// Not Create the physics world because the user needs to enable it.
}

vl::GameManager::~GameManager(void )
{
	//Shutdown cAudio
	if( _audio_manager )
	{
		std::cout << "Exit audio." << std::endl;
		_audio_manager->shutDown();
		cAudio::destroyAudioManager(_audio_manager);
	}

	_trackers.reset();

	delete _scene_manager;
	delete _python;
	delete _event_man;
}

void
vl::GameManager::createSceneManager( vl::Session *session )
{
	assert( !_scene_manager );
	_scene_manager = new vl::SceneManager(session, _mesh_manager);
}

vl::PlayerPtr
vl::GameManager::createPlayer( void )
{
	assert( !_player );
	_player = new Player();
	return _player;
}

void
vl::GameManager::toggleBackgroundSound( void )
{
	if(isAudioEnabled())
	{ return; }

	if( !_background_sound )
	{
		std::cerr << "NO background sound to toggle." << std::endl;
		return;
	}

	if( _background_sound->isPlaying() )
	{ _background_sound->pause(); }
	else
	{ _background_sound->play2d(false); }
}

bool
vl::GameManager::step(void)
{
	if(isPlayed())
	{
		// process input devices
		for(size_t i = 0; i < _input_devices.size(); ++i)
		{
			_input_devices.at(i)->mainloop();
		}

		// Process Tracking
		// If we have a tracker object update it, the update will handle all the
		// callbacks and appropriate updates (head matrix and scene nodes).
		for( size_t i = 0; i < _trackers->getNTrackers(); ++i )
		{
			_trackers->getTrackerPtr(i)->mainloop();
		}

		if( _physics_world )
		{
			_physics_world->step();
		}

		vl::time elapsed = _step_timer.elapsed();

		_process_constraints(elapsed);

		_scene_manager->_step(elapsed);
	}

	_step_timer.reset();

	return !isQuited();
}

void
vl::GameManager::enableAudio(bool enable)
{
	if( enable )
	{
		if(_audio_manager)
		{ return; }

		std::cout << vl::TRACE << "Init audio." << std::endl;

		cAudio::ILogger *audio_logger = cAudio::getLogger();
		if( audio_logger->isLogReceiverRegistered("File") )
		{
			std::cout << vl::TRACE << "Removing cAudio File logger" << std::endl;
			audio_logger->unRegisterLogReceiver("File");
		}

		//Create an Audio Manager
		_audio_manager = cAudio::createAudioManager(true);
	}
}

void
vl::GameManager::createBackgroundSound( std::string const &song_name )
{
	if( !isAudioEnabled() )
	{ return; }

	assert(getResourceManager());

	//Create an audio source and load a sound from a file
	std::string file_path;

	if(getResourceManager()->findResource(song_name, file_path))
	{
		vl::Resource resource;
		getResourceManager()->loadOggResource( song_name, resource );
		_background_sound = _audio_manager
			->createFromMemory(song_name.c_str(), resource.get(), resource.size(), "ogg");
	}
	else
	{
		std::cerr << "Couldn't find " << song_name << " from resources." << std::endl;
	}
}

void
vl::GameManager::addInputDevice(vl::InputDeviceRefPtr dev)
{
	_input_devices.push_back(dev);
}

void 
vl::GameManager::addConstraint(vl::ConstraintRefPtr constraint)
{
	_constraint_solver->addConstraint(constraint);
}

void
vl::GameManager::removeConstraint(vl::ConstraintRefPtr constraint)
{
	_constraint_solver->removeConstraint(constraint);
}

bool
vl::GameManager::hasConstraint(vl::ConstraintRefPtr constraint) const
{
	return _constraint_solver->hasConstraint(constraint);
}

vl::physics::WorldRefPtr
vl::GameManager::getPhysicsWorld( void )
{
	return _physics_world;
}

void
vl::GameManager::enablePhysics( bool enable )
{
	if(enable)
	{
		// Create the physics if they don't exist
		if(!_physics_world)
		{
			_physics_world.reset(new physics::World());
		}
	}
}

void 
vl::GameManager::enableEnvironmentalEffects(bool enable)
{
	_env_effects_enabled = enable;
	// @todo reset the scene
}

void 
vl::GameManager::setWeather(vl::Weather const &weather)
{
	_weather = weather;
	/// @todo should do changes in the scenes
}

void 
vl::GameManager::setTimeOfDay(vl::Date const &date)
{
	_date = date;
	/// @todo should do changes in the scenes
}

bool 
vl::GameManager::requestStateChange(vl::GAME_STATE state)
{
	if(state == _state)
	{ return true; }

	switch(state)
	{
	case GS_INIT:
		if(_state != GS_UNKNOWN)
		{ return false; }
		_state = GS_INIT;
		break;

	case GS_PLAY:
		if( _state == GS_INIT )
		{
			_game_timer.reset();
		}
		else if( _state == GS_PAUSE )
		{
			_game_timer.resume();
		}
		else
		{ return false; }
		_state = GS_PLAY;
		_step_timer.reset();
		break;

	case GS_PAUSE:
		if( _state == GS_PLAY )
		{
			_game_timer.stop();
			_state = GS_PAUSE;
		}
		// Allow moving straight to paused state
		else if( _state == GS_INIT )
		{
			_state = GS_PAUSE;
		}
		break;

	case GS_QUIT:
		_state = GS_QUIT;
		break;

	default:
		return false;
	}

	return true;
}

vl::RecordingRefPtr
vl::GameManager::loadRecording(std::string const &path)
{
	std::cout << vl::TRACE << "vl::GameManager::loadRecording" << std::endl;
	vl::Resource resource;
	getResourceManager()->loadRecording(path, resource);

	RecordingRefPtr rec(new Recording(path));
	rec->read(resource);

	return rec;
}

void
vl::GameManager::loadScene(vl::SceneInfo const &scene_info)
{
	std::cout << vl::TRACE << "Loading scene file = " << scene_info.getName() << std::endl;

	vl::TextResource resource;
	getResourceManager()->loadResource(scene_info.getFile(), resource);

	// Default to false for now
	bool use_mesh = false;
	if(scene_info.getUseNewMeshManager() == CFG_ON)
	{ use_mesh = true; }

	if(scene_info.getUsePhysics())
	{ enablePhysics(true); }

	vl::DotSceneLoader loader(use_mesh);
	// TODO pass attach node based on the scene
	// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
	// @todo add physics
	loader.parseDotScene(resource, getSceneManager(), getPhysicsWorld());

	std::cout << "Scene " << scene_info.getName() << " loaded." << std::endl;
}

void
vl::GameManager::_process_constraints(vl::time const &t)
{
	_constraint_solver->step(t);
}
