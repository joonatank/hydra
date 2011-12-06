/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file game_manager.cpp
 *	
 *	This file is part of Hydra VR game engine.
 *
 */

#include "game_manager.hpp"

// Python manager
#include "python/python.hpp"
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
#include "gui/gui_window.hpp"

// Physics
#include "physics/physics_world.hpp"

#include "dotscene_loader.hpp"

#include "animation/kinematic_world.hpp"

#include "recording.hpp"

#include "actions_misc.hpp"

#include "settings.hpp"

#include "vrpn_analog_client.hpp"

vl::GameManager::GameManager(vl::Session *session, vl::Logger *logger)
	: _session(session)
	, _python(0)
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
{
	if(!_session || !_logger)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	_kinematic_world.reset(new vl::KinematicWorld(this));

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
	_fire_step_start();

	if(isPlayed())
	{
		getEventManager()->getFrameTrigger()->update(getDeltaTime());

		// Process input devices
		getEventManager()->mainloop();

		// Process Tracking
		// If we have a tracker object update it, the update will handle all the
		// callbacks and appropriate updates (head matrix and scene nodes).
		for( size_t i = 0; i < _trackers->getNTrackers(); ++i )
		{
			_trackers->getTrackerPtr(i)->mainloop();
		}

		for(size_t i = 0; i < _analog_clients.size(); ++i )
		{ _analog_clients.at(i)->mainloop(); }

		_kinematic_world->step(getDeltaTime());

		if( _physics_world )
		{
			_physics_world->step(getDeltaTime());
		}

		/// Check collisions and copy the SceneNode transformations
		_kinematic_world->finalise();

		// Copy collision barrier transformations to visual objects
		_scene_manager->_step(getDeltaTime());
	}

	_fire_step_end();

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
			_physics_world = physics::World::create(this);
			assert(_physics_world);
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
		_init();
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

void
vl::GameManager::setupResources(vl::Settings const &settings, vl::config::EnvSettings const &env)
{
	std::cout << vl::TRACE << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	std::vector<std::string> paths = settings.getAuxDirectories();
	paths.push_back(settings.getProjectDir());
	for( size_t i = 0; i < paths.size(); ++i )
	{ getResourceManager()->addResourcePath( paths.at(i) ); }

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	std::cout << vl::TRACE << "Adding ${environment}/tracking to the resources paths." << std::endl;
	fs::path tracking_path( fs::path(env.getEnvironementDir()) / "tracking" );
	if( fs::is_directory(tracking_path) )
	{ getResourceManager()->addResourcePath( tracking_path.string() ); }
}

void
vl::GameManager::load(vl::config::EnvSettings const &env)
{
	// Create Tracker needs the SceneNodes for mapping
	timer t;
	createTrackers(env);
	std::cout << "Creating trackers took : " <<  t.elapsed() << std::endl;

	assert(_player);
	_player->setIPD(env.getIPD());
}

void
vl::GameManager::load(vl::Settings const &proj)
{
	timer t;
	loadScenes(proj);
	std::cout << "Loading scenes took : " <<  t.elapsed() << std::endl;

	t.reset();
	runPythonScripts(proj);
	std::cout << "Executing scripts took : " <<  t.elapsed() << std::endl;
}

vl::RecordingRefPtr
vl::GameManager::loadRecording(std::string const &path)
{
	if(!isInited())
	{ BOOST_THROW_EXCEPTION(vl::invalid_game_state()); }

	std::cout << vl::TRACE << "vl::GameManager::loadRecording" << std::endl;
	vl::Resource resource;
	getResourceManager()->loadRecording(path, resource);

	RecordingRefPtr rec(new Recording(path));
	rec->read(resource);

	return rec;
}

void
vl::GameManager::loadScenes(vl::Settings const &proj)
{
	std::cout << vl::TRACE << "Loading Scenes for Project : " << proj.getProjectName()
		<< std::endl;

	// Get scenes
	std::vector<vl::ProjSettings::Scene> scenes = proj.getScenes();

	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		vl::SceneInfo const &scene = scenes.at(i);
		loadScene(scene);
	}
}

void
vl::GameManager::runPythonScripts(vl::Settings const &proj)
{
	/// @todo this should also load scripts that are not used but are
	/// available, these should of course not be run
	/// use the new auto_run interface in PythonContext
	std::vector<std::string> scripts = proj.getScripts();

	for( size_t i = 0; i < scripts.size(); ++i )
	{
		// Load the python scripts
		vl::TextResource script_resource;
		getResourceManager()->loadResource( scripts.at(i), script_resource );
		getPython()->addScript(scripts.at(i), script_resource, true);
	}

	/// Run the python scripts
	getPython()->autoRunScripts();
}

void
vl::GameManager::createTrackers(vl::config::EnvSettings const &env)
{
	assert(_trackers);

	std::vector<std::string> tracking_files = env.getTrackingFiles();

	std::cout << vl::TRACE << "Processing " << tracking_files.size()
		<< " tracking files." << std::endl;

	/// @todo This part is the great time consumer, need to pin point the time hog
	for( std::vector<std::string>::const_iterator iter = tracking_files.begin();
		 iter != tracking_files.end(); ++iter )
	{
		// Read a file
		vl::TextResource resource;
		getResourceManager()->loadResource(*iter, resource);

		vl::TrackerSerializer ser(_trackers);
		ser.parseTrackers(resource);
	}
}

vl::vrpn_analog_client_ref_ptr
vl::GameManager::createAnalogClient(std::string const &name)
{
	vrpn_analog_client_ref_ptr ptr(new vrpn_analog_client);
	ptr->_create(name);
	_analog_clients.push_back(ptr);
	return ptr;
}

void
vl::GameManager::loadScene(vl::SceneInfo const &scene_info)
{
	if(!isInited())
	{ BOOST_THROW_EXCEPTION(vl::invalid_game_state()); }

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

/// ------------------------------ Private -----------------------------------
void
vl::GameManager::_init(void)
{
	// Create the distributed objects
	_createSceneManager();
	_createPlayer();

	assert(_session);
	assert(!_gui);

	_gui.reset(new vl::gui::GUI(_session));

	// Window creation
	gui::WindowRefPtr win = _gui->createWindow("console");
	win->setVisible(false);

	_createQuitEvent();
}

void
vl::GameManager::createEditor(void)
{
	std::clog << "vl::GameManager::createEditor" << std::endl;
	assert(_gui);

	gui::WindowRefPtr win = _gui->createWindow("editor");
	win->setVisible(false);
	win = _gui->createWindow("scene_graph_editor");
	win->setVisible(false);
	win = _gui->createWindow("material_editor");
	win->setVisible(false);
	win = _gui->createWindow("script_editor");
	win->setVisible(false);
}

vl::SceneManagerPtr
vl::GameManager::_createSceneManager(void)
{
	assert(!_scene_manager);
	assert(_mesh_manager && _session);
	_scene_manager = new vl::SceneManager(_session, _mesh_manager);
	return _scene_manager;
}

vl::PlayerPtr
vl::GameManager::_createPlayer(void)
{
	assert(!_player);
	assert(_scene_manager);
	_player = new Player(_scene_manager);
	// Registering Player in init
	// TODO move this to do an automatic registration similar to SceneManager
	_session->registerObject(_player, OBJ_PLAYER);

	return _player;
}

void
vl::GameManager::_createQuitEvent(void)
{
	// Add a trigger event to Quit the Application
	QuitAction *quit = QuitAction::create();
	quit->data = this;
	// Add trigger
	vl::KeyTrigger *trig = getEventManager()->createKeyTrigger( OIS::KC_ESCAPE, KEY_MOD_META );
	trig->setKeyDownAction(quit);
}

void
vl::GameManager::_fire_step_start(void)
{
	_delta_time = _step_timer.elapsed();
}

void
vl::GameManager::_fire_step_end(void)
{
	_step_timer.reset();
}
