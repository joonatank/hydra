/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 */

#include "game_manager.hpp"

// Python manager
#include "python.hpp"

// Event Manager
#include "event_manager.hpp"

// Resource Manager concrete implementation
#include "distrib_resource_manager.hpp"

// SceneManager
#include "scene_manager.hpp"

// Player
#include "player.hpp"

// Physics
#include "physics/physics_world.hpp"

vl::GameManager::GameManager( vl::Logger *logger )
	: _python(0)
	, _resource_man( new vl::DistribResourceManager )
	, _event_man( new vl::EventManager )
	, _scene_manager(0)
	, _player(0)
	, _trackers( new vl::Clients( _event_man ) )
	, _audio_manager(0)
	, _background_sound(0)
	, _logger(logger)
	, _quit( false )
	, _physics_world(0)
{
	_python = new vl::PythonContext( this );

	std::cout << vl::TRACE << "Init audio." << std::endl;

	cAudio::ILogger *audio_logger = cAudio::getLogger();
	if( audio_logger->isLogReceiverRegistered("File") )
	{
		std::cout << vl::TRACE << "Removing cAudio File logger" << std::endl;
		audio_logger->unRegisterLogReceiver("File");
	}

	//Create an Audio Manager
	_audio_manager = cAudio::createAudioManager(true);

	// Not Create the physics world because the user needs to enable it.
}

vl::GameManager::~GameManager(void )
{
	std::cout << "Exit audio." << std::endl;

	//Shutdown cAudio
	if( _audio_manager )
	{
		_audio_manager->shutDown();
		cAudio::destroyAudioManager(_audio_manager);
	}
	delete _physics_world;
}

void
vl::GameManager::createSceneManager( vl::Session *session )
{
	assert( !_scene_manager );
	_scene_manager = new vl::SceneManager( session );
}


vl::PythonContextPtr
vl::GameManager::getPython(void )
{
	return _python;
}

vl::ResourceManagerPtr
vl::GameManager::getReourceManager(void )
{
	return _resource_man;
}

vl::PlayerPtr
vl::GameManager::getPlayer(void )
{
	return _player;
}

vl::EventManagerPtr
vl::GameManager::getEventManager(void )
{
	return _event_man;
}

vl::SceneManagerPtr
vl::GameManager::getSceneManager( void )
{
	return _scene_manager;
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

void
vl::GameManager::quit(void )
{ _quit = true; }

bool
vl::GameManager::step(void )
{
	// Process Tracking
	// If we have a tracker object update it, the update will handle all the
	// callbacks and appropriate updates (head matrix and scene nodes).
	for( size_t i = 0; i < _trackers->getNTrackers(); ++i )
	{
		_trackers->getTracker(i)->mainloop();
	}

	if( _physics_world )
	{
		_physics_world->step();
	}

	return !_quit;
}


void
vl::GameManager::createBackgroundSound( std::string const &song_name )
{
	assert( getReourceManager() );

	//Create an audio source and load a sound from a file
	std::string file_path;

	if( getReourceManager()->findResource( song_name, file_path ) )
	{
		vl::Resource resource;
		getReourceManager()->loadOggResource( song_name, resource );
		_background_sound = _audio_manager
			->createFromMemory("The_Dummy_Song", resource.get(), resource.size(), "ogg" );
	}
	else
	{
		std::cerr << "Couldn't find " << song_name << " from resources." << std::endl;
	}
}

vl::physics::World *
vl::GameManager::getPhysicsWorld( void )
{
	return _physics_world;
}

void
vl::GameManager::enablePhysics( bool enable )
{
	if( enable )
	{
		// Create the physics if they don't exist
		if( _physics_world == 0 )
		{
			_physics_world = new physics::World();
		}
	}
}
