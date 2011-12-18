/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file game_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Interface
#include "game_manager.hpp"

// Managers that we own
#include "python/python_context_impl.hpp"
#include "event_manager.hpp"
#include "resource_manager.hpp"
#include "mesh_manager.hpp"
#include "material_manager.hpp"
#include "scene_manager.hpp"

// Player
#include "player.hpp"

#include "gui/gui.hpp"
#include "gui/gui_window.hpp"

// Physics
#include "physics/physics_world.hpp"

/// File Loaders
#include "dotscene_loader.hpp"
#include "collada/dae_importer.hpp"

// File writers
#include "collada/dae_exporter.hpp"

#include "animation/kinematic_world.hpp"

#include "recording.hpp"

#include "settings.hpp"

#include "vrpn_analog_client.hpp"

#include "tracker.hpp"
#include "tracker_serializer.hpp"


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
	, _kinematic_world(new vl::KinematicWorld)
{
	if(!_session || !_logger)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	_mesh_manager.reset(new MeshManager(new MasterMeshLoaderCallback(_resource_man)));
	_python = new vl::PythonContextImpl( this );

	_material_manager.reset(new MaterialManager(_session));

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
			_physics_world = physics::World::create();
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
vl::GameManager::setupResources(vl::config::EnvSettings const &env)
{
	std::cout << vl::TRACE << "Adding project directories to resources." << std::endl;

	// Add environment directory, used for tracking configurations
	if(!env.getEnvironementDir().empty())
	{
		std::cout << vl::TRACE << "Adding ${environment}/tracking to the resources paths." << std::endl;
		fs::path tracking_path( fs::path(env.getEnvironementDir()) / "tracking" );
		if( fs::is_directory(tracking_path) )
		{ getResourceManager()->addResourcePath( tracking_path.string() ); }
	}
}

void
vl::GameManager::addResources(vl::ProjSettings const &proj)
{
	if(proj.empty())
	{
		std::clog << "Trying to add Empty project to Resources." << std::endl;
		return;
	}

	std::clog << "Project file = " << proj.getFile() << std::endl;

	fs::path proj_file(proj.getFile());

	// check that the paths are valid
	fs::path proj_dir = proj_file.parent_path();
	if(fs::is_directory(proj_dir))
	{
		getResourceManager()->addResourcePath(proj_dir.string());
	}
	else
	{ 
		BOOST_THROW_EXCEPTION(vl::missing_dir() 
			<< vl::file_name(proj_dir.string()) 
			<< vl::desc("Resource dir is not a directory."));
	}
}

void
vl::GameManager::removeResources(vl::ProjSettings const &proj)
{
	if(proj.empty())
	{
		std::clog << "Trying to remove Empty project from Resources." << std::endl;
		return;
	}

	fs::path proj_file(proj.getFile());

	// check that the paths are valid
	fs::path proj_dir = proj_file.parent_path().string();
	if(!fs::is_directory(proj_dir))
	{
		getResourceManager()->removeResourcePath(proj_dir.string());
	}
}

void
vl::GameManager::load(vl::config::EnvSettings const &env)
{
	// Create Tracker needs the SceneNodes for mapping
	chrono t;
	createTrackers(env);
	std::cout << "Creating trackers took : " <<  t.elapsed() << std::endl;

	assert(_player);
	_player->setIPD(env.getIPD());
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
vl::GameManager::loadProject(std::string const &file_name)
{
	vl::ProjSettings project;
	// load the project
	vl::ProjSettingsSerializer ser(ProjSettingsRefPtr(&project, vl::null_deleter()));
	bool retval = ser.readFile(file_name);
	assert(retval);

	std::clog << "Loading project " << project.getName() << std::endl;

	std::vector<vl::ProjSettings *> _run_scripts;

	// remove the already loaded project because we only support one for now
	if(!_loaded_project.empty())
	{
		// @todo this probably should work on file paths rather than names
		if(project.getName() == _loaded_project.getName())
		{
			std::cout << "Trying to load already loaded project " << project.getName() << std::endl;
			return; 
		}

		// @todo remove resource paths
		removeResources(_loaded_project);

		// @todo remove scene nodes
		
		// reset python
		_python->reset();

		// rerun the python context
		_run_scripts.push_back(&_global_project);	
	}
	// for loading a new project we don't need to do any reseting

	_run_scripts.push_back(&project);

	_loaded_project = project;

	addResources(_loaded_project);

	loadScenes(_loaded_project);

	for(size_t i = 0; i < _run_scripts.size(); ++i)
	{
		runPythonScripts(*_run_scripts.at(i));
	}
	

	vl::Settings set;
	set.setProjectSettings(_loaded_project);
	set.addAuxilarySettings(_global_project);

	// Send new settings
	_project_changed_signal(set);
}

void
vl::GameManager::loadGlobal(std::string const &file_name)
{
	// @todo this should check that the global is not already loaded

	// reset the python context
	_python->reset();

	// reset the global
	vl::ProjSettings global;
	vl::ProjSettingsSerializer ser(ProjSettingsRefPtr(&global, vl::null_deleter()));
	bool retval = ser.readFile(file_name);
	assert(retval);

	std::clog << "Loading Global : " << global.getName() << std::endl;

	if(global.getName() != _global_project.getName())
	{
		removeResources(_global_project);

		_global_project = global;

		addResources(_global_project);

		loadScenes(_global_project);

		// rerun the python context
		runPythonScripts(_global_project);
		runPythonScripts(_loaded_project);

		vl::Settings set;
		set.setProjectSettings(_loaded_project);
		set.addAuxilarySettings(_global_project);

		// Send new settings
		_project_changed_signal(set);
	}
}

void
vl::GameManager::removeProject(std::string const &name)
{
	// remove project
	if(_loaded_project.getName() == name)
	{
		removeResources(_loaded_project);

		// reset the python context
		_python->reset();

		// @todo should clean up
		// needs to remove scenes also

		// rerun the python context
		runPythonScripts(_global_project);

		vl::Settings set;
		set.addAuxilarySettings(_global_project);
	
		// Send new settings
		_project_changed_signal(set);
	}
}

void
vl::GameManager::loadScenes(vl::ProjSettings const &proj)
{
	for( size_t i = 0; i < proj.getCase().getNscenes(); ++i )
	{
		loadScene(proj.getCase().getScene(i));
	}
}

void
vl::GameManager::runPythonScripts(vl::ProjSettings const &proj)
{
	for( size_t i = 0; i < proj.getCase().getNscripts(); ++i )
	{
		// Load the python scripts
		ProjSettings::Script const &script = proj.getCase().getScript(i);
		
		// scripts that are in use are set to be auto ran others are just stored.
		vl::TextResource script_resource;
		getResourceManager()->loadResource(script.getFile(), script_resource);
		getPython()->addScript(script.getFile(), script_resource, script.getUse());
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

	vl::chrono t;
	fs::path file(scene_info.getFile());
	if(file.extension() == ".dae")
	{
		std::clog << "Loading Collada file." << std::endl;

		/// @todo this needs to find the absolute path using ResourceManager
		/// OpenCollada needs file name not the complete file
		/// per it's design to handle large files.
		std::string path;
		if(getResourceManager()->findResource(file.string(), path))
		{
			vl::dae::ImporterSettings settings;
			// Settings for testing Anark core importing, cleanup
			// can't be enabled as long as we are testing cube importing
			// we could use runtime for changing settings but there is no such functionality yet.
			//settings.handle_duplicates = vl::dae::ImporterSettings::HDN_USE_ORIGINAL;
			//settings.remove_duplicate_materials = true;
			vl::dae::Managers man;
			man.material = _material_manager;
			man.scene = _scene_manager;
			man.mesh = _mesh_manager;
			vl::dae::Importer loader(settings, man);
			loader.read(path);
		}
		else
		{
			std::cout << "Couldn't find DAE resource : " << file << std::endl;
		}
	}
	else if(file.extension() == ".scene")
	{
		std::clog << "Loading scene file." << std::endl;

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
	}
	else
	{
		// @todo should throw or do some default magic
		std::clog << "ERROR : Unknown scene file extension." << std::endl;
	}

	std::cout << "Scene " << scene_info.getName() << " loaded. Loading took " << t.elapsed() << "." << std::endl;
}

void
vl::GameManager::loadScene(std::string const &file_name)
{
	vl::SceneInfo scene_info;
	scene_info.setUse(true);
	scene_info.setFile(file_name);
	scene_info.setName(file_name);

	loadScene(scene_info);
}

void
vl::GameManager::saveScene(std::string const &file_name)
{
	fs::path file(file_name);
	if(file.extension() == ".dae")
	{
		std::cout << vl::TRACE << "Writing dae file" << std::endl;
		vl::dae::ExporterSettings settings;
		vl::dae::Managers man;
		man.material = _material_manager;
		man.scene = _scene_manager;
		man.mesh = _mesh_manager;
		vl::dae::Exporter writer(settings, man);
		writer.write(file);
	}
	else
	{
		std::cout << vl::CRITICAL << "Only Collada supports writing scene file." << std::endl;
	}
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
	vl::KeyTrigger *trig = getEventManager()->createKeyTrigger( OIS::KC_ESCAPE, KEY_MOD_META );
	trig->addKeyDownListener( boost::bind(&GameManager::quit, this) );
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
