/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-10
 *	@file config.cpp
 */

#include "config.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include "actions_misc.hpp"

#include "dotscene_loader.hpp"

#include "tracker_serializer.hpp"
#include "base/filesystem.hpp"
#include "resource_manager.hpp"
#include "resource.hpp"

#include "scene_manager.hpp"
// Necessary for hiding collision meshes
#include "scene_node.hpp"

// Necessary for retrieving other managers
#include "game_manager.hpp"
// Necessary for executing python scripts
#include "python.hpp"
// Necessary for retrieving registered triggers, KeyTrigger and FrameTrigger.
#include "event_manager.hpp"
// Necessary for registering Player
#include "player.hpp"
// Necessary for sending Environment and Project configs
#include "distrib_settings.hpp"

// Necessary for loading meshes to Server
#include "mesh_manager.hpp"

#include "base/string_utils.hpp"
#include "base/sleep.hpp"

#include "renderer_interface.hpp"

#include "gui/gui.hpp"

/// ---------------------------------- Callbacks -----------------------------
vl::ConfigMsgCallback::ConfigMsgCallback(vl::Config *own)
	: owner(own)
{
	assert(owner);
}

void
vl::ConfigMsgCallback::operator()(vl::cluster::Message const &msg)
{
	assert(owner);
	owner->pushMessage(msg);
}

vl::ConfigServerDataCallback::ConfigServerDataCallback(vl::Config *own)
	: owner(own)
{
	assert(owner);
}

vl::cluster::Message
vl::ConfigServerDataCallback::createInitMessage(void)
{
	assert(owner);
	return owner->createMsgInit();
}


vl::cluster::Message 
vl::ConfigServerDataCallback::createEnvironmentMessage(void)
{
	assert(owner);
	return owner->createMsgEnvironment();
}

vl::cluster::Message 
vl::ConfigServerDataCallback::createProjectMessage(void)
{
	assert(owner);
	return owner->createMsgProject();
}

vl::cluster::Message
vl::ConfigServerDataCallback::createResourceMessage(vl::cluster::RESOURCE_TYPE type, std::string const &name)
{
	std::clog << "vl::ConfigServerDataCallback::createResourceMessage" << std::endl;

	vl::cluster::Message msg(vl::cluster::MSG_RESOURCE, 0, vl::time());
	if( type == vl::cluster::RES_MESH )
	{
		assert(owner->getGameManager()->getMeshManager()->hasMesh(name));
		if(!owner->getGameManager()->getMeshManager()->hasMesh(name))
		{
			BOOST_THROW_EXCEPTION(vl::null_pointer());
		}

		vl::MeshRefPtr mesh = owner->getGameManager()->getMeshManager()->getMesh(name);
		
		vl::cluster::MessageDataStream stream = msg.getStream();
		stream << vl::cluster::RES_MESH << name << *mesh;
	}
	else
	{
		BOOST_THROW_EXCEPTION(vl::not_implemented());
	}

	return msg;
}

/// ---------------------------------- Config --------------------------------
vl::Config::Config( vl::Settings const & settings,
					vl::EnvSettingsRefPtr env,
					vl::Logger &logger,
					vl::RendererInterfacePtr rend)
	: _game_manager( new vl::GameManager(&logger) )
	, _proj(settings)
	, _env(env)
	, _server()
	, _running(true)
	, _renderer(rend)
{
	std::cout << vl::TRACE << "vl::Config::Config" << std::endl;
	assert( _env );
	// TODO assert that the settings are valid
	assert( _env->isMaster() );

	/// Server can be created here because the callback is only called
	/// when the Server has environment and project
	vl::ConfigServerDataCallback *cb = new vl::ConfigServerDataCallback(this);
	_server.reset(new vl::cluster::Server(_env->getServer().port, cb));
	_callbacks.push_back(cb);

	// if we have a renderer we have to set callbacks
	if(_renderer.get())
	{
		vl::MsgCallback *callback = new vl::ConfigMsgCallback(this);
		_renderer->setSendMessageCB(callback);
		_callbacks.push_back(callback);
	}

	_createResourceManager( settings, env );

	_game_manager->createSceneManager(this);

	_renderer->setMeshManager(_game_manager->getMeshManager());
}

vl::Config::~Config( void )
{
	std::cout << vl::TRACE << "vl::Config::~Config" << std::endl;

	delete _game_manager;

	for( std::vector<vl::Callback *>::iterator iter = _callbacks.begin();
		iter != _callbacks.end(); ++iter )
	{ delete *iter; }
}

/// @todo this is taking way too long
/// @todo server should be running and responding to messages at the same time
/// Problem calling Server::poll infrequently through this function will
/// cause the clients to freeze.
/// Anyhow it would be much more useful to call Server::poll every 1ms,
/// for example with interrupts.
void
vl::Config::init( void )
{
	std::cout << vl::TRACE << "vl::Config::init" << std::endl;
	vl::timer init_timer;

	_game_manager->requestStateChange(GS_INIT);

	vl::timer t;
	/// @todo most of this should be moved to the constructor, like object
	/// creation
	/// sending of initial messages to rendering threads should still be here

	_setEnvironment(_env);
	std::cout << "Sending Environment took : " <<  t.elapsed() << std::endl;
	t.reset();

	_setProject(_proj);
	std::cout << "Project took : " <<  t.elapsed() << std::endl;
	t.reset();

	// Create the player necessary for Trackers
	vl::PlayerPtr player = _game_manager->createPlayer();

	assert( player );
	player->setIPD(_env->getIPD());

	// Registering Player in init
	// TODO move this to do an automatic registration similar to SceneManager
	registerObject( player, OBJ_PLAYER );

	vl::gui::GUIRefPtr gui(new vl::gui::GUI(this));
	_game_manager->setGUI(gui);

	vl::SceneManager *sm = _game_manager->getSceneManager();
	assert( sm );

	std::cout << "Registering gui, scene manager and player took : "
		<<  t.elapsed() << std::endl;
	t.reset();

	_loadScenes();

	std::cout << "Loading scenes took : " <<  t.elapsed() << std::endl;
	t.reset();

	// Create Tracker needs the SceneNodes for mapping
	_createTrackers(_env);
	std::cout << "Creating trackers took : " <<  t.elapsed() << std::endl;
	t.reset();

	/// @todo python scripts should be moved to GameManager with project
	/// loading
	/// @todo this should also load scripts that are not used but are
	/// available, these should of course not be run
	/// use the new auto_run interface in PythonContext
	std::vector<std::string> scripts = _proj.getScripts();

	/// @todo replace with the new interface
	for( size_t i = 0; i < scripts.size(); ++i )
	{
		// Load the python scripts
		vl::TextResource script_resource;
		_game_manager->getReourceManager()->loadResource( scripts.at(i), script_resource );
		_game_manager->getPython()->addScript(scripts.at(i), script_resource, true);
	}

	/// Run the python scripts
	t.reset();
	_game_manager->getPython()->autoRunScripts();
	std::cout << "Executing scripts took : " <<  t.elapsed() << std::endl;
	t.reset();

	_createQuitEvent();

	std::cout << vl::TRACE << "vl::Config:: updating server" << std::endl;
	_server->poll();
	_updateFrameMsgs();
	_updateServer();
	std::cout << "Updating server took : " <<  t.elapsed() << std::endl;
	t.reset();

	if(_renderer.get())
	{
		_renderer->sendMessage(_msg_create);
		_renderer->sendMessage(createMsgInit());
	}

	std::cout << "Loading Renderer with new created objects took : "
		<<  t.elapsed() << std::endl;
	t.reset();

	_game_manager->getStats().logInitTime( ((double)init_timer.elapsed())*1e3 );

	_stats_timer.reset();

	// TODO this should block till both slaves and local renderer are ready
	// Problematic as this does not take into account clients that are started
	// later than this function is ran.
//	std::clog << "blocking till slave clients are ready." << std::endl;
//	_server->block_till_initialised();
	_server->poll();

	_game_manager->play();
}

void
vl::Config::exit(void)
{
	std::cout << vl::TRACE << "vl::Config::exit" << std::endl;

	_renderer.reset();

	_server->shutdown();
}

void
vl::Config::render( void )
{
	vl::timer timer;

	assert(_server);

	// Process a time step in the game
	// New event interface
	_game_manager->getEventManager()->getFrameTrigger()->update();
	_game_manager->getStats().logFrameProcessingTime( (double(timer.elapsed()))*1e3 );

	timer.reset();
	if( !_game_manager->step() )
	{ stopRunning(); }
	_game_manager->getStats().logStepTime( ((double)timer.elapsed())*1e3 );

	/// Provide the updates to slaves
	_updateFrameMsgs();
	_updateServer();
	_updateRenderer();

	// TODO separate stats for rendering slaves and local
	timer.reset();
	/// Render the scene
	_server->update(_game_manager->getStats());
	bool rendering = _server->start_draw(_game_manager->getStats());
	// Rendering after the server has sent the command to slaves
	if( _renderer.get() )
	{
		_renderer->draw();
	}
	if(rendering)
	{
		vl::time limit(1, 0);
		_server->finish_draw(_game_manager->getStats(), limit);
	}

	// Finish local renderer
	if( _renderer.get() )
	{
		_renderer->swap();
		_renderer->capture();
	}
	_game_manager->getStats().logRenderingTime( ((double)timer.elapsed())*1e3 );
	timer.reset();

	// Poll after updating the drawables
	_server->poll();

	// TODO where the receive input messages should be?
	_handleMessages();

	_game_manager->getStats().logEventProcessingTime(((double)timer.elapsed())*1e3);

	// Update statistics every 10 seconds
	// @todo time limit should be configurable
	if( _stats_timer.elapsed() > vl::time(10) )
	{
		_game_manager->getStats().update();
//		std::clog << _game_manager->getStats() << std::endl;
		_stats_timer.reset();
	}
}

vl::cluster::Message
vl::Config::popMessage(void)
{
	if( !messages() )
	{ return vl::cluster::Message(); }

	vl::cluster::Message msg = _messages.front();
	_messages.pop_front();
	return msg;
}

void
vl::Config::pushMessage(vl::cluster::Message const &msg)
{
	_messages.push_back(msg);
}

vl::cluster::Message
vl::Config::createMsgInit(void)
{
	vl::cluster::Message msg( vl::cluster::MSG_SG_UPDATE, 0, vl::time() );

	std::vector<vl::Distributed *>::iterator iter;
	for( iter = _registered_objects.begin(); iter != _registered_objects.end();
		++iter )
	{
		assert( (*iter)->getID() != vl::ID_UNDEFINED );
		vl::cluster::ObjectData data( (*iter)->getID() );
		vl::cluster::ByteDataStream stream = data.getStream();
		(*iter)->pack( stream, vl::Distributed::DIRTY_ALL );
		data.copyToMessage(&msg);
		/// Don't clear dirty because this is a special case
	}

	return msg;
}

vl::cluster::Message 
vl::Config::createMsgEnvironment(void) const
{
	std::clog << "vl::Config::createMsgEnvironemnt" << std::endl;

	vl::SettingsByteData data;
	vl::cluster::ByteDataStream stream( &data );
	stream << _env;

	vl::cluster::Message msg( vl::cluster::MSG_ENVIRONMENT, 0, vl::time() );
	data.copyToMessage( &msg );

	return msg;
}

/// @todo this should be replace by sending of a vector of paths
/// that's the only thing needed in the Renderer
vl::cluster::Message 
vl::Config::createMsgProject(void) const
{
	std::clog << "vl::Config::createMsgProject" << std::endl;

	vl::SettingsByteData data;
	vl::cluster::ByteDataStream stream( &data );
	stream << _proj;

	vl::cluster::Message msg( vl::cluster::MSG_PROJECT, 0, vl::time() );
	data.copyToMessage( &msg );

	return msg;
}

/// ------------ Private -------------
void
vl::Config::_updateServer( void )
{
	assert( _server );

	if( !_msg_create.empty() )
	{
		_server->sendCreate(_msg_create);
	}

	_server->sendUpdate(_msg_update);

	// Send logs
	if( _server->logEnabled() )
	{
		vl::Logger *log = _game_manager->getLogger();
		while( _server->nLoggedMessages() <  log->nMessages() )
		{
			_server->logMessage( log->getMessage(_server->nLoggedMessages()) );
		}
	}

	_server->poll();
}

void
vl::Config::_updateRenderer(void)
{
	if( !_renderer.get() )
	{ return; }

	if( !_msg_create.empty() )
	{ _renderer->sendMessage(_msg_create); }

	_renderer->sendMessage(_msg_update);

	// Send logs
	if( _renderer->logEnabled() )
	{
		vl::Logger *log = _game_manager->getLogger();
		while( _renderer->nLoggedMessages() <  log->nMessages() )
		{
			_renderer->logMessage( log->getMessage(_renderer->nLoggedMessages()) );
		}
	}
}

void
vl::Config::_updateFrameMsgs(void)
{
	_createMsgCreate();
	_createMsgUpdate();
}

void
vl::Config::_createMsgCreate(void)
{
	// New objects created need to send SG_CREATE message
	if( !getNewObjects().empty() )
	{
		_msg_create = vl::cluster::Message( vl::cluster::MSG_SG_CREATE, 0, vl::time() );
		_msg_create.write( getNewObjects().size() );
		for( size_t i = 0; i < getNewObjects().size(); ++i )
		{
			OBJ_TYPE type = getNewObjects().at(i).first;
			uint64_t id = getNewObjects().at(i).second->getID();
			_msg_create.write( type );
			_msg_create.write( id );
		}

		clearNewObjects();
	}
	else
	{
		_msg_create.clear();
	}
}

void
vl::Config::_createMsgUpdate(void)
{
	// Create SceneGraph updates
	_msg_update = vl::cluster::Message( vl::cluster::MSG_SG_UPDATE, 0, vl::time() );

	std::vector<vl::Distributed *>::iterator iter;
	for( iter = _registered_objects.begin(); iter != _registered_objects.end();
		++iter )
	{
		if( (*iter)->isDirty() )
		{
			assert( (*iter)->getID() != vl::ID_UNDEFINED );
			vl::cluster::ObjectData data( (*iter)->getID() );
			vl::cluster::ByteDataStream stream = data.getStream();
			(*iter)->pack(stream);
			data.copyToMessage(&_msg_update);
			/// Clear dirty because this update has been applied
			(*iter)->clearDirty();
		}
	}
}

void
vl::Config::_setEnvironment(vl::EnvSettingsRefPtr env)
{
	vl::timer t;

	// Local renderer needs to be inited rather than send a message
	if( _renderer.get() )
	{
		_renderer->init(env);
	}
	std::cout << "Initing Renderer took : " << t.elapsed() << std::endl;
}

void
vl::Config::_setProject(vl::Settings const &proj)
{
	// Send the project to the local Renderer
	if(_renderer.get())
	{
		vl::timer t;
		vl::cluster::Message msg = createMsgProject();
		_renderer->sendMessage(msg);
		std::cout << "Sending message of size " << sizeof(msg) << "bytes "
			<< "to Renderer took " << t.elapsed() << std::endl;
	}
}

/// @todo with project message this takes 450ms to complete
/// because Renderer is damned slow parsing the message.
void
vl::Config::_sendMessage(vl::cluster::Message const &msg)
{
	if(_renderer.get())
	{ _renderer->sendMessage(msg); }

	_server->sendMessage(msg);
}

/// @todo this takes over 1 second to complete which is almost a second too much
void
vl::Config::_createTrackers(vl::EnvSettingsRefPtr settings)
{
	vl::ClientsRefPtr clients = _game_manager->getTrackerClients();
	assert( clients );

	std::vector<std::string> tracking_files = settings->getTrackingFiles();

	std::cout << vl::TRACE << "Processing " << tracking_files.size() << " tracking files."
		<< std::endl;

	/// @todo This part is the create time consumer
	/// Need to use a report to pin point the hog
	vl::timer t;
	for( std::vector<std::string>::const_iterator iter = tracking_files.begin();
		 iter != tracking_files.end(); ++iter )
	{
		// Read a file
		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( *iter, resource );

		vl::TrackerSerializer ser( clients );
		ser.parseTrackers(resource);
	}
	std::cout << "Parsing tracking files took : " << t.elapsed() << std::endl;
	t.reset();
	std::cout << "Starting trackers took : " << t.elapsed() << std::endl;
	t.reset();
	std::cout << "Creating head trigger took : " << t.elapsed() << std::endl;
}

void
vl::Config::_loadScenes( void )
{
	std::cout << vl::TRACE << "Loading Scenes for Project : " << _proj.getProjectName()
		<< std::endl;

	// Get scenes
	std::vector<vl::ProjSettings::Scene> scenes = _proj.getScenes();

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		vl::SceneInfo const &scene = scenes.at(i);
		_game_manager->loadScene(scene);
	}
}

void
vl::Config::_createQuitEvent(void )
{
	// Add a trigger event to Quit the Application
	assert( _game_manager );
	vl::QuitAction *quit = vl::QuitAction::create();
	quit->data = _game_manager;
	// Add trigger
	vl::KeyTrigger *trig = _game_manager->getEventManager()->createKeyTrigger( OIS::KC_ESCAPE, KEY_MOD_META );
	trig->setKeyDownAction(quit);
}

void
vl::Config::_createResourceManager( vl::Settings const &settings, vl::EnvSettingsRefPtr env )
{
	std::cout << vl::TRACE << "Initialising Resource Manager" << std::endl;

	std::cout << vl::TRACE << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	std::vector<std::string> paths = settings.getAuxDirectories();
	paths.push_back(settings.getProjectDir());
	for( size_t i = 0; i < paths.size(); ++i )
	{ _game_manager->getReourceManager()->addResourcePath( paths.at(i) ); }

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	std::cout << vl::TRACE << "Adding ${environment}/tracking to the resources paths." << std::endl;
	fs::path tracking_path( fs::path(env->getEnvironementDir()) / "tracking" );
	if( fs::is_directory(tracking_path) )
	{ _game_manager->getReourceManager()->addResourcePath( tracking_path.file_string() ); }
}

/// Event Handling
void
vl::Config::_handleMessages( void )
{
	// Check local messages, pushed there by callbacks
	while( messages() )
	{
		vl::cluster::Message msg = popMessage();
		_handleMessage(msg);
	}

	// TODO this should use the same callback system as Renderer
	while( _server->messages() )
	{
		vl::cluster::Message msg = _server->popMessage();
		_handleMessage(msg);
	}
}

void
vl::Config::_handleMessage(vl::cluster::Message &msg)
{
	switch( msg.getType() )
	{
		case vl::cluster::MSG_INPUT:
			_handleEventMessage(msg);
			break;

		case vl::cluster::MSG_COMMAND:
			_handleCommandMessage(msg);
			break;

		case vl::cluster::MSG_REG_OUTPUT:
			// Should not be added as we are using the LogReceiver interface
			// Problematic as the Remote nodes need this message, but the
			// local node does not.
			break;

		default :
			std::cout << vl::CRITICAL << "Unhandled Message in Config : type = "
				<< vl::cluster::getTypeAsString(msg.getType()) << std::endl;
	}
}

void
vl::Config::_handleEventMessage(vl::cluster::Message &msg)
{
	while( msg.size() )
	{
		vl::cluster::EventData data;
		data.copyFromMessage(&msg);
		vl::cluster::ByteDataStream stream = data.getStream();
		switch( data.getType() )
		{
			case vl::cluster::EVT_KEY_PRESSED :
			{
				OIS::KeyEvent evt( 0, OIS::KC_UNASSIGNED, 0 );
				stream >> evt;
				_handleKeyPressEvent(evt);
			}
			break;

			case vl::cluster::EVT_KEY_RELEASED :
			{
				OIS::KeyEvent evt( 0, OIS::KC_UNASSIGNED, 0 );
				stream >> evt;
				_handleKeyReleaseEvent(evt);
			}
			break;

			case vl::cluster::EVT_MOUSE_PRESSED :
			{
				OIS::MouseButtonID b_id;
				OIS::MouseEvent evt( 0, OIS::MouseState() );
				stream >> b_id >> evt;
				_handleMousePressEvent(evt, b_id);
			}
			break;

			case vl::cluster::EVT_MOUSE_RELEASED :
			{
				OIS::MouseButtonID b_id;
				OIS::MouseEvent evt( 0, OIS::MouseState() );
				stream >> b_id >> evt;
				_handleMouseReleaseEvent(evt, b_id);
			}
			break;

			case vl::cluster::EVT_MOUSE_MOVED :
			{
				OIS::MouseEvent evt( 0, OIS::MouseState() );
				stream >> evt;
				_handleMouseMotionEvent(evt);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_PRESSED :
			{
				OIS::JoyStickEvent evt( 0, OIS::JoyStickState() );
				int button;
				stream >> button >> evt;
				_handleJoystickButtonPressedEvent(evt, button);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_RELEASED :
			{
				OIS::JoyStickEvent evt( 0, OIS::JoyStickState() );
				int button;
				stream >> button >> evt;
				_handleJoystickButtonReleasedEvent(evt, button);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_AXIS :
			{
				OIS::JoyStickEvent evt( 0, OIS::JoyStickState() );
				int axis;
				stream >> axis >> evt;
				_handleJoystickAxisMovedEvent(evt, axis);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_POV :
			{
				OIS::JoyStickEvent evt( 0, OIS::JoyStickState() );
				int pov;
				stream >> pov >> evt;
				_handleJoystickPovMovedEvent(evt, pov);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_VECTOR3 :
			{
				OIS::JoyStickEvent evt( 0, OIS::JoyStickState() );
				int index;
				stream >> index >> evt;
				_handleJoystickVector3MovedEvent(evt, index);
			}
			break;

			default :
				std::cout << vl::CRITICAL << "vl::Config::_receiveEventMessages : "
					<< "Unhandleded message type." << std::endl;
				break;
		}
	}
}

void
vl::Config::_handleCommandMessage(vl::cluster::Message &msg)
{
	// TODO there should be a maximum amount of messages stored
	// TODO works only on ASCII at the moment
	std::string cmd;
	msg.read(cmd);
	_game_manager->getPython()->executeCommand(cmd);
}

void
vl::Config::_handleKeyPressEvent( OIS::KeyEvent const &event )
{
	OIS::KeyCode kc = event.key;

	_game_manager->getEventManager()->keyPressed(kc);
}

void
vl::Config::_handleKeyReleaseEvent( OIS::KeyEvent const &event )
{
	OIS::KeyCode kc = event.key;

	_game_manager->getEventManager()->keyReleased(kc);
}

void
vl::Config::_handleMousePressEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id )
{
	std::cout << vl::TRACE << "vl::Config::_handleMousePressEvent" << std::endl;
}

void
vl::Config::_handleMouseReleaseEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id )
{
	std::cout << vl::TRACE << "vl::Config::_handleMouseReleaseEvent" << std::endl;
}

void
vl::Config::_handleMouseMotionEvent( OIS::MouseEvent const &event )
{
}

void
vl::Config::_handleJoystickButtonPressedEvent( OIS::JoyStickEvent const &event, int button )
{
	std::cout << vl::TRACE << "vl::Config::_handleJoystickButtonPressedEvent" << std::endl;
}

void
vl::Config::_handleJoystickButtonReleasedEvent( OIS::JoyStickEvent const &event, int button )
{
	std::cout << vl::TRACE << "vl::Config::_handleJoystickButtonReleasedEvent" << std::endl;
}

void
vl::Config::_handleJoystickAxisMovedEvent( OIS::JoyStickEvent const &event, int axis )
{
	std::cout << vl::TRACE << "vl::Config::_handleJoystickAxisMovedEvent" << std::endl;
}

void
vl::Config::_handleJoystickPovMovedEvent( OIS::JoyStickEvent const &event, int pov )
{
	std::cout << vl::TRACE << "vl::Config::_handleJoystickPovMovedEvent" << std::endl;
}

void
vl::Config::_handleJoystickVector3MovedEvent( OIS::JoyStickEvent const &event, int index )
{
	std::cout << vl::TRACE << "vl::Config::_handleJoystickVector3MovedEvent" << std::endl;
}
