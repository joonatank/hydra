/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file game_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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
// Necessary because we need to access some special functions
#include "gui/performance_overlay.hpp"

// Physics
#include "physics/physics_world.hpp"

/// File Loaders
#include "dotscene_loader.hpp"
#include "hsf_loader.hpp"
#include "collada/dae_importer.hpp"

#include "cad_importer.hpp"

// File writers
#include "collada/dae_exporter.hpp"

#include "animation/kinematic_world.hpp"

// Necessary for passing Settings to a callback signal
#include "settings.hpp"

#include "vrpn_analog_client.hpp"
// Necessary for creating and processing trackers
#include "tracker.hpp"
#include "tracker_serializer.hpp"
// Necessary for unloading a scene
#include "scene_node.hpp"

#include "hsf_writer.hpp"

#include "game_object.hpp"

vl::GameManager::GameManager(vl::Session *session, vl::Logger *logger, vl::ProgramOptions const &opt)
	: _session(session)
	, _python(0)
	, _resource_man(new vl::ResourceManager)
	, _event_man(new vl::EventManager)
	, _scene_manager(0)
	, _player(0)
	, _trackers( new vl::Clients( _event_man ) )
	, _logger(logger)
	, _auto_start(true)
	, _options(opt)
	, _fsm(new GameManagerFSM)
{
	if(!_session || !_logger)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	_kinematic_world.reset(new vl::KinematicWorld(this));

	_mesh_manager.reset(new MeshManager(new MasterMeshLoaderCallback(_resource_man)));
	_python = new vl::PythonContextImpl( this );

	_material_manager.reset(new MaterialManager(_session));

	// Not creating audio context because user needs to enable it separately.

	// Not Create the physics world because the user needs to enable it.
	
	// Create cad importer
	std::string cad_exe = opt.cad_importer_exe;
	if(!opt.cad_importer_enabled)
	{ cad_exe.clear(); }

	_cad_importer.reset(new CadImporter(cad_exe));

	_fsm->setGameManager(this);
	_fsm->start();
}

vl::GameManager::~GameManager(void )
{
	_trackers.reset();

	delete _scene_manager;
	delete _python;
	delete _event_man;
	delete _fsm;
}

void
vl::GameManager::step(void)
{
	_fire_step_start();

	/// Process triggers wether we are paused or not
	/// @todo should have at least two categories for events 
	/// those that are In Game events and those that are not
	/// (probably going to need more than two categories but for now).
	/// this allows pausing the updating of all bodies (In Game)
	/// but retains the ability move cameras using event system.
	/// We need at least two categories for this
	/// Simulation, Paused
	/// Usefull categories would be Menu and Debug
	/// Probably matching the Event categories to the Game States would
	/// also work just fine. And if they need to be expanded then we can
	/// allow users to add more game states which would auto create more
	/// event categories.
	/// This elaborate system also needs to allow masking events so
	/// they belong to more than just one category or an ALL category.

	// Process input devices
	getEventManager()->mainloop(getDeltaTime());

	// Process Tracking
	// If we have a tracker object update it, the update will handle all the
	// callbacks and appropriate updates (head matrix and scene nodes).
	// needs to be processed even if paused so we have perspective modifications
	for( size_t i = 0; i < _trackers->getNTrackers(); ++i )
	{
		_trackers->getTrackerPtr(i)->mainloop();
	}

	if(_eye_tracker)
	{ _eye_tracker->progress(); }

	if(isPlaying())
	{
		for(size_t i = 0; i < _analog_clients.size(); ++i )
		{ _analog_clients.at(i)->mainloop(); }
		
		vl::chrono c;
		_kinematic_world->step(getDeltaTime());
		_rendering_report[PT_KINEMATICS].push(c.elapsed());

		if( _physics_world )
		{
			c.reset();
			_physics_world->step(getDeltaTime());
			_rendering_report[PT_PHYSICS].push(c.elapsed());
		}

		c.reset();
		/// Check collisions and copy the SceneNode transformations
		_kinematic_world->finalise();
		_rendering_report[PT_COLLISIONS].push(c.elapsed());

		// Copy collision barrier transformations to visual objects
		_scene_manager->_step(getDeltaTime());
	}

	_cad_importer->mainloop();

	_fire_step_end();
}

vl::GameObjectRefPtr
vl::GameManager::createGameObject(std::string const &name)
{
	return _createGameObject(name, false);
}

vl::GameObjectRefPtr
vl::GameManager::createDynamicGameObject(std::string const &name)
{
	return _createGameObject(name, true);
}

void
vl::GameManager::removeGameObject(GameObjectRefPtr obj)
{
	std::clog << "vl::GameManager::removeGameObject" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

bool
vl::GameManager::hasGameObject(std::string const &name)
{
	return getGameObject(name) != GameObjectRefPtr();
}

vl::GameObjectRefPtr
vl::GameManager::getGameObject(std::string const &name)
{
	for(std::vector<GameObjectRefPtr>::iterator iter = _game_objects.begin();
		iter != _game_objects.end(); ++iter)
	{
		if((*iter)->getName() == name)
		{ return *iter; }
	}

	return GameObjectRefPtr();
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

bool
vl::GameManager::isQuited(void) const
{
	vl::state *s = _fsm->get_state_by_id(_fsm->current_state()[0]);
	
	if(dynamic_cast<GameManagerFSM_::Quited *>(s))
	{ return true; }

	return false;
}

bool
vl::GameManager::isPaused(void) const
{
	vl::state *s = _fsm->get_state_by_id(_fsm->current_state()[0]);
	
	if(dynamic_cast<GameManagerFSM_::Paused *>(s))
	{ return true; }

	return false;
}

bool
vl::GameManager::isPlaying(void) const
{
	vl::state *s = _fsm->get_state_by_id(_fsm->current_state()[0]);
	
	if(dynamic_cast<GameManagerFSM_::Playing *>(s))
	{ return true; }

	return false;
}

bool
vl::GameManager::isStopped(void) const
{
	vl::state *s = _fsm->get_state_by_id(_fsm->current_state()[0]);
	
	if(dynamic_cast<GameManagerFSM_::Stopped *>(s))
	{ return true; }

	return false;
}

std::string
vl::GameManager::getStateName(void) const
{
	vl::state *s = _fsm->get_state_by_id(_fsm->current_state()[0]);

	if(dynamic_cast<GameManagerFSM_::Stopped *>(s))
	{ return "Stopped"; }
	else if(dynamic_cast<GameManagerFSM_::Initing *>(s))
	{ return "Initing"; }
	else if(dynamic_cast<GameManagerFSM_::Loading *>(s))
	{ return "Loading"; }
	else if(dynamic_cast<GameManagerFSM_::Playing *>(s))
	{ return "Playing"; }
	else if(dynamic_cast<GameManagerFSM_::Paused *>(s))
	{ return "Paused"; }
	else if(dynamic_cast<GameManagerFSM_::Quited *>(s))
	{ return "Quited"; }
	else
	{ return "Unkown"; }
}

/// State Management
void
vl::GameManager::quit(void)
{ _fsm->process_event(vl::quit()); }

void
vl::GameManager::pause(void)
{ _fsm->process_event(vl::pause()); }

void
vl::GameManager::play(void)
{ _fsm->process_event(vl::play()); }

void
vl::GameManager::stop(void)
{ _fsm->process_event(vl::stop()); }

void
vl::GameManager::unload(void)
{
	std::clog << "vl::GameManager::unload" << std::endl;

	_old_project = _loaded_project.getFile();
	_old_global = _global_project.getFile();

	_fsm->process_event(vl::unload());
}

void
vl::GameManager::load(std::string const &project, std::string const &global)
{ _fsm->process_event(vl::load(project, global)); }

void
vl::GameManager::reload(void)
{
	// Unload needs to be called before calling this
	// load will only work when if state is Initing so the state
	// change handles rest of error checking for us.
	if(_old_project.empty() || _old_global.empty())
	{ return; }

	std::clog << "Reloading project : " << _old_project << " global : " << _old_global << std::endl;

	load(_old_project, _old_global);
}

void
vl::GameManager::restart(void)
{
	stop();

	/// @todo this only works for new HSF files
	try {
		loadScenes(_global_project, LOADER_FLAG_OVERWRITE);
		loadScenes(_loaded_project, LOADER_FLAG_OVERWRITE);
	}
	catch(...)
	{
		std::cerr << "Problem with reloading the project. Only HSF files are supported" << std::endl;
	}

	/// remove all created objects
	/// rerun all the python scripts
	_rerunPythonScripts();

	if(auto_start())
	{ play(); }
	else
	{ pause(); }
}

/// @todo this is going to be really complex when more states is added
/// so we should move the signals to their respective states
/// for now though they can stay here for testing if the architecture is even
/// feasable.
int
vl::GameManager::addStateChangedListener(vl::state const &state, StateChanged::slot_type const &slot)
{
	vl::state const *p_state = &state;
	if(dynamic_cast<GameManagerFSM_::Initing const *>(p_state))
	{
		_inited_signal.connect(slot);
	}
	else if(dynamic_cast<GameManagerFSM_::Loading const *>(p_state))
	{
		_loaded_signal.connect(slot);
	}
	else if(dynamic_cast<GameManagerFSM_::Playing const *>(p_state))
	{
		_played_signal.connect(slot);
	}
	else if(dynamic_cast<GameManagerFSM_::Paused const *>(p_state))
	{
		_paused_signal.connect(slot);
	}
	else if(dynamic_cast<GameManagerFSM_::Stopped const *>(p_state))
	{
		_stopped_signal.connect(slot);
	}
	else if(dynamic_cast<GameManagerFSM_::Quited const *>(p_state))
	{
		_quited_signal.connect(slot);
	}

	return 1;
}


void
vl::GameManager::_destroyDynamicObjects(void)
{
	std::clog << "vl::GameManager::removeDynamicObjects" << std::endl;

	// Destroy dynamic GameObjects
	GameObjectList obj_to_destroy;
	for(GameObjectList::iterator iter = _game_objects.begin();
		iter != _game_objects.end(); ++iter)
	{
		if((*iter)->isDynamic())
		{ obj_to_destroy.push_back(*iter); }
	}
	for(GameObjectList::iterator iter = obj_to_destroy.begin();
		iter != obj_to_destroy.end(); ++iter)
	{
		removeGameObject(*iter);
	}

	if(_physics_world)
	{
		_physics_world->destroyDynamicObjects();
	}
	if(_kinematic_world)
	{
		_kinematic_world->destroyDynamicObjects();
	}
	if(_scene_manager)
	{
		_scene_manager->destroyDynamicObjects();
	}

	/// Destroy dynamic events i.e. all events except those we have created
	_event_man->removeAll();
	_createQuitEvent();

	// Reset player
	_player->setCamera(_scene_manager->getEditorCamera());
}

void
vl::GameManager::_rerunPythonScripts(void)
{
	std::clog << "vl::GameManager::rerunPythonScripts" << std::endl;

	_python->reset();
	
	// run the python context
	if(!_global_project.empty())
	{ _addPythonScripts(_global_project); }
	if(!_loaded_project.empty())
	{ _addPythonScripts(_loaded_project); }

	/// Run the python scripts
	getPython()->autoRunScripts();

	std::clog << "vl::GameManager::rerunPythonScripts : DONE" << std::endl;
}

vl::Settings
vl::GameManager::getSettings(void) const
{
	vl::Settings set;
	set.setProjectSettings(_loaded_project);
	set.addAuxilarySettings(_global_project);

	return set;
}

void
vl::GameManager::loadScenes(vl::ProjSettings const &proj, LOADER_FLAGS flags)
{
	for( size_t i = 0; i < proj.getCase().getNscenes(); ++i )
	{
		loadScene(proj.getCase().getScene(i), flags);
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
vl::GameManager::loadScene(vl::SceneInfo const &scene_info, LOADER_FLAGS flags)
{
	std::cout << vl::TRACE << "Loading scene file = " << scene_info.getName() << std::endl;

	if(!scene_info.getUse())
	{
		std::cout << "Trying to load scene : " << scene_info.getName() 
			<< " that is not in use." << std::endl;
		return;
	}

	vl::chrono t;
	fs::path file(scene_info.getFile());
	try
	{
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
			std::clog << "Loading Ogre scene file \"" << scene_info.getFile() << "\"" << std::endl;

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
		else if(file.extension() == ".hsf")
		{
			std::clog << "Loading Hydra scene file : "
				<< file << std::endl;

			vl::TextResource resource;
			getResourceManager()->loadResource(scene_info.getFile(), resource);

			// Enable physics engine, might be needed might not be needed
			// shoudln't matter other than longer loading times and memory consumption
			// if no bodies are created the physics step should be almost a NOP.
			enablePhysics(true);

			// Force the use of new mesh manager
			vl::HSFLoader loader;
			// TODO pass attach node based on the scene
			// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
			// @todo add physics
			loader.parseScene(resource, this, flags);
		}
		else
		{
			// @todo should throw or do some default magic
			std::cout << "ERROR : Unknown scene file extension." << std::endl;
		}
	}
	catch(rapidxml::parse_error const &e)
	{
		std::clog << "XML parsing error : " << e.what() << std::endl
			<< " in file " << file.string() << std::endl
			<< e.where<char>() << std::endl;
	}

	std::cout << "Scene " << scene_info.getName() << " loaded. Loading took " << t.elapsed() << "." << std::endl;
}

void
vl::GameManager::loadScene(std::string const &file_name, LOADER_FLAGS flags)
{
	vl::SceneInfo scene_info;
	scene_info.setUse(true);
	scene_info.setFile(file_name);
	scene_info.setName(file_name);

	loadScene(scene_info, flags);
}

void
vl::GameManager::saveScene(std::string const &file_name)
{
	fs::path file(file_name);
	if(file.extension() == ".dae")
	{
		std::cout << vl::TRACE << "Writing Collada file" << std::endl;
		vl::dae::ExporterSettings settings;
		vl::dae::Managers man;
		man.material = _material_manager;
		man.scene = _scene_manager;
		man.mesh = _mesh_manager;
		vl::dae::Exporter writer(settings, man);
		writer.write(file);
	}
	else if(file.extension() == ".hsf")
	{
		std::clog << "Writing HSF file" << std::endl;

		vl::HSFWriter writer(this);
		writer.write(file, true);
	}
	else
	{
		std::cout << vl::CRITICAL << "Only Collada supports writing scene file." << std::endl;
	}
}


/// Event Management

void
vl::GameManager::_do_init(init const &evt)
{
	std::clog << "vl::GameManager::_do_init" << std::endl;

	// Setup resources
	_setupResources(*evt.environment);

	// Create the distributed objects
	_createSceneManager();
	_createPlayer();

	assert(_session);
	assert(!_gui);

	_gui.reset(new vl::gui::GUI(_session));

	// Window creation
	gui::WindowRefPtr win = _gui->createWindow(OBJ_GUI_CONSOLE);
	win->setVisible(false);
	
	win = _gui->createWindow(OBJ_GUI_PERFORMANCE_OVERLAY);
	gui::PerformanceOverlayRefPtr ov = boost::dynamic_pointer_cast<gui::PerformanceOverlay>(win);
	if(!ov)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }
	ov->setRenderingReport(&_rendering_report);
	ov->setVisible(_options.debug.overlay);
	ov->setShowAdvanced(_options.debug.overlay_advanced);

	_createQuitEvent();

	// Create the eye tracker
	// For now auto start it
	assert(_player && _scene_manager);
	_eye_tracker.reset(new EyeTracker(_player, _scene_manager));

	assert(evt.environment);
	_loadEnvironment(*evt.environment);

	_inited_signal();
}

void
vl::GameManager::_do_load(vl::load const &evt)
{
	std::clog << "vl::GameManager::_do_load" << std::endl;

	/// We do not allow loading when we already have project or global
	/// This should of course never happen because the FSM doesn't
	/// allow it.
	if(!_global_project.empty() || !_loaded_project.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	std::clog << "Loading Global " << evt.global << std::endl;
	_loadGlobal(evt.global);

	// load the project
	if(!evt.project.empty())
	{
		vl::ProjSettingsSerializer ser(ProjSettingsRefPtr(&_loaded_project, vl::null_deleter()));
		if(!ser.readFile(evt.project))
		{ BOOST_THROW_EXCEPTION(vl::exception()); }

		std::clog << "Loading project " << _loaded_project.getName() << std::endl;

		_addResources(_loaded_project);

		loadScenes(_loaded_project);
	}

	// run the python context
	if(!_global_project.empty())
	{ _addPythonScripts(_global_project); }
	if(!_loaded_project.empty())
	{ _addPythonScripts(_loaded_project); }

	/// Run the python scripts
	getPython()->autoRunScripts();

	_loaded_signal();
}

void
vl::GameManager::_do_play(vl::play const &evt)
{
	std::clog << "vl::GameManager::_do_play" << std::endl;
	_game_timer.resume();
	_played_signal();
}

void
vl::GameManager::_do_pause(vl::pause const &evt)
{
	std::clog << "vl::GameManager::_do_pause" << std::endl;
	_game_timer.stop();
	_paused_signal();
}

void
vl::GameManager::_do_stop(vl::stop const &evt)
{
	std::clog << "vl::GameManager::_do_stop" << std::endl;

	_game_timer.reset();
	_game_timer.stop();
	_stopped_signal();
}

void
vl::GameManager::_do_unload(vl::unload const &evt)
{
	std::clog << "vl::GameManager::_do_unload" << std::endl;

	_removeAll();

	// No signal because we don't have unload state
	_inited_signal();
}

void
vl::GameManager::_do_quit(vl::quit const &evt)
{
	std::clog << "vl::GameManager::_do_quit" << std::endl;
	_quited_signal();
}

/// ------------------------------ Private -----------------------------------
vl::GameObjectRefPtr
vl::GameManager::_createGameObject(std::string const &name, bool dynamic)
{
	GameObjectRefPtr obj = getGameObject(name);
	if(!obj)
	{
		obj.reset(new GameObject(name, this, dynamic));
		_game_objects.push_back(obj);
	}
	return obj;
}

void
vl::GameManager::_loadEnvironment(vl::config::EnvSettings const &env)
{
	std::clog << "vl::GameManager::_loadEnvironment" << std::endl;
	
	// Create Tracker needs the SceneNodes for mapping
	chrono t;
	_createTrackers(env);
	std::cout << "Creating trackers took : " <<  t.elapsed() << std::endl;

	assert(_player);
	_player->setIPD(env.getIPD());
}

void
vl::GameManager::_loadGlobal(std::string const &file_name)
{
	std::clog << "vl::GameManager::_loadGlobal" << std::endl;

	// reset the python context
	_python->reset();

	// reset the global
	vl::ProjSettings global;
	vl::ProjSettingsSerializer ser(ProjSettingsRefPtr(&global, vl::null_deleter()));
	// We must have a global file because we need resources for the GUI.
	if(!ser.readFile(file_name))
	{
		BOOST_THROW_EXCEPTION(vl::missing_file() << vl::desc("Global project file"));
	}

	std::clog << "Loading Global : " << global.getName() << std::endl;

	/// Should never already have a global config here
	if(!_global_project.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	_global_project = global;

	_addResources(_global_project);

	loadScenes(_global_project);
}

void
vl::GameManager::_removeAll(void)
{
	std::clog << "vl::GameManager::removeAll" << std::endl;
	assert(_scene_manager);
	assert(_python);

	// Reset python to pointers so that ref counted objects are destroyed
	std::clog << "vl::GameManager::removeAll : reset python" << std::endl;
	_python->reset();
	// Destroy event handlers
	// @todo this is problematic because it removes both Quit and Console events
	// also, which are rather necessary for us.
	std::clog << "vl::GameManager::removeAll : destroy events" << std::endl;
	_event_man->removeAll();
	_createQuitEvent();

	// Game objects need to be first because they can own bodies and nodes
	// ref counted so we let the destroyer handle these
	std::clog << "vl::GameManager::removeAll : destroy game objects" << std::endl;
	_game_objects.clear();

	std::clog << "vl::GameManager::removeAll : destroy kinematic bodies" << std::endl;
	if(_kinematic_world)
	{ _kinematic_world->removeAll(); }

	std::clog << "vl::GameManager::removeAll : destroy dynamic bodies" << std::endl;
	if(_physics_world)
	{ _physics_world->removeAll(); }

	// Handle all the rest Scene resources that are not handled by game objects
	// mostly for backwards compatibility with old system
	std::clog << "vl::GameManager::removeAll : destroy scene" << std::endl;
	_scene_manager->destroyScene();

	// @todo remove _analog_clients
	// created from python, so we should remove them

	// @todo remove _trackers
	// not a good idea, created from env so it's not affected by project loading

	// @todo clean projects
	_loaded_project = vl::ProjSettings();
	_global_project = vl::ProjSettings();

	// Reset stats
	_rendering_report = vl::ProfilerReport();
	_init_report = vl::Report<vl::time>();

	// Reset player
	CameraPtr cam = _scene_manager->getCamera("editor/perspective");
	assert(cam);
	_player->setCamera(cam);

	// remove loaded meshes
	// We need remove meshes because the mesh loader will throw if they are still loaded
	// python, scene manager and physics manager needs to be before this
	// @todo This should use cleanup_unused but it's broken atm.
	_mesh_manager->removeAll();

	// @todo Remove all loaded materials
	// Not yet necessary as the material system is not complete

	// Optional remove all resource paths

	// @todo Reset timers
	// not sure if it's necessary here, we could just reset them when loading

	// @todo add checks that confirm the kinematic, dynamic bodies and scene nodes
	// have been destroyed and the animation graph is empty.
	assert(_kinematic_world->getBodies().empty());
	assert(_kinematic_world->getConstraints().empty());
	if(_physics_world)
	{
		assert(_physics_world->getTubes().empty());
		assert(_physics_world->getBodies().empty());
		assert(_physics_world->getConstraints().empty());
	}
}

void
vl::GameManager::_createTrackers(vl::config::EnvSettings const &env)
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

void
vl::GameManager::_setupResources(vl::config::EnvSettings const &env)
{
	std::clog << "Adding project directories to resources." << std::endl;

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
vl::GameManager::_addResources(vl::ProjSettings const &proj)
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
vl::GameManager::_removeResources(vl::ProjSettings const &proj)
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
vl::GameManager::_addPythonScripts(vl::ProjSettings const &proj)
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


void
vl::CadImporterCallback::_load(std::string const &filename, std::string const &file_path)
{
	std::clog << "CadImporterCallback::_load" << std::endl;
	// @todo we need to replace the extension
	fs::path file(file_path);
	if(!fs::exists(file))
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Imported file does not exists.") << vl::file_name(file_path)); }

	if(file.extension() != ".dae")
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Imported file is not Collada.") << vl::file_name(file_path)); }

	_game->loadScene(file_path);
}
