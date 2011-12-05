/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-10
 *	@file config.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

/// Interface
#include "config.hpp"

#include "base/exceptions.hpp"

// Necessary for retrieving other managers
#include "game_manager.hpp"
// Necessary for executing python commands from slaves
#include "python/python.hpp"
// Necessary for retrieving registered triggers, KeyTrigger and FrameTrigger and sending events.
#include "event_manager.hpp"
// Necessary for the Event handling
#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>

#include "input/joystick_event.hpp"

// Necessary for sending Environment and Project configs
#include "distrib_settings.hpp"
// Necessary for loading meshes to Server
#include "mesh_manager.hpp"

// Necessary for local renderer
#include "renderer_interface.hpp"

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
					vl::config::EnvSettingsRefPtr env,
					vl::Logger &logger,
					vl::RendererUniquePtr rend)
	: _game_manager(0)
	, _proj(settings)
	, _env(env)
	, _server()
	, _running(true)
	, _renderer(rend)
{
	std::cout << vl::TRACE << "vl::Config::Config" << std::endl;
	assert( _env );
	assert( _env->isMaster() );
	// @todo should check that the environment settings are valid

	_game_manager = new vl::GameManager(this, &logger);

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

	_game_manager->setupResources(settings, *env);

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
vl::Config::init(bool enable_editor)
{
	std::cout << vl::TRACE << "vl::Config::init" << std::endl;
	vl::timer init_timer;

	vl::Report<vl::time> &report = _game_manager->getInitReport();
	vl::timer t;
	if(!_game_manager->requestStateChange(GS_INIT))
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Couldn't change state to INIT")); }
	report["Starting GameManager"].push(t.elapsed());

	if(enable_editor)
	{ _game_manager->createEditor(); }

	// Local renderer needs to be inited rather than send a message
	if( _renderer.get() )
	{
		t.reset();
		_renderer->init(_env);
		report["Initing Renderer"].push(t.elapsed());
	}

	// Send the project to the local Renderer
	if(_renderer.get())
	{
		t.reset();
		_renderer->setProject(_proj);
		report["Setting project to Renderer"].push(t.elapsed());
	}

	/// @todo change to use GS_LOAD which is inititiated automatically after GS_INIT
	_game_manager->load(*_env);
	_game_manager->load(_proj);

	/// Updating the Renderers
	t.reset();
	_server->poll();
	_updateFrameMsgs();
	_updateServer();
	report["Updating server"].push(t.elapsed());

	if(_renderer.get())
	{
		t.reset();
		vl::cluster::Message msg(_msg_create);
		_renderer->createSceneObjects(msg);
		msg = createMsgInit();
		_renderer->initScene(msg);
		report["Creating objects in Renderer"].push(t.elapsed());
	}

	report["Total init"].push(init_timer.elapsed());
	report.finish();
	
	_stats_timer.reset();

	_game_manager->getPython()->addVariable("server_report", _server->getReport());
	
	// TODO this should block till both slaves and local renderer are ready
	// Problematic as this does not take into account clients that are started
	// later than this function is ran.
	// Another problem is that if we block here the starting is going to take
	// longer no matter what
	// For example problematic configurations and so on.
	// We should rather block the simulation till all slaves are up,
	// which should be configurable from ini file, but allow the use of
	// global actions like EXIT.
//	std::clog << "blocking till slave clients are ready." << std::endl;
//	_server->block_till_initialised();
	_server->poll();

	// Start
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
	assert(_server);

	vl::timer loop_timer;
	vl::Report<vl::time> &report = _game_manager->getRenderingReport();

	vl::timer timer;

	// Get new event messages that are processed in GameManager::step
	_server->poll();
	_handleMessages();

	// Process a time step in the game
	timer.reset();
	if( !_game_manager->step() )
	{ stopRunning(); }
	report["step time"].push(timer.elapsed());

	/// Provide the updates to slaves
	timer.reset();
	_updateFrameMsgs();
	_updateServer();
	_updateRenderer();

	// TODO separate stats for rendering slaves and local
	_server->update();
	report["scene graph update time"].push(timer.elapsed());

	/// Render the scene
	timer.reset();
	bool rendering = _server->start_draw();
	// Rendering after the server has sent the command to slaves
	if( _renderer.get() )
	{
		vl::timer l;
		_renderer->draw();
		report["local rendering"].push(l.elapsed());
	}
	if(rendering)
	{
		vl::time limit(1, 0);
		_server->finish_draw(limit);
	}

	// Finish local renderer
	if( _renderer.get() )
	{
		vl::timer l;
		_renderer->swap();
		report["local swap"].push(l.elapsed());

		l.reset();
		_renderer->capture();
		report.get_number("local capture").push(l.elapsed());
	}

	report["Rendering loop"].push(loop_timer.elapsed());

	// Update statistics every 10 seconds
	// @todo time limit should be configurable
	if( _stats_timer.elapsed() > vl::time(10) )
	{
		report.finish();
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
	std::cout << vl::TRACE << "vl::Config::createMsgEnvironemnt" << std::endl;

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
	std::cout << vl::TRACE << "vl::Config::createMsgProject" << std::endl;

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
	{
		vl::cluster::Message msg(_msg_create);
		_renderer->createSceneObjects(msg);
	}

	vl::cluster::Message msg(_msg_update);
	_renderer->updateScene(msg);

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
				// @todo should pass the whole event structure
				_game_manager->getEventManager()->keyPressed(evt.key);
			}
			break;

			case vl::cluster::EVT_KEY_RELEASED :
			{
				OIS::KeyEvent evt( 0, OIS::KC_UNASSIGNED, 0 );
				stream >> evt;
				// @todo should pass the whole event structure
				_game_manager->getEventManager()->keyReleased(evt.key);
			}
			break;

			case vl::cluster::EVT_MOUSE_PRESSED :
			{
				OIS::MouseButtonID b_id;
				OIS::MouseEvent evt( 0, OIS::MouseState() );
				stream >> b_id >> evt;

				// @todo should pass the mouse state to event manager
			}
			break;

			case vl::cluster::EVT_MOUSE_RELEASED :
			{
				OIS::MouseButtonID b_id;
				OIS::MouseEvent evt( 0, OIS::MouseState() );
				stream >> b_id >> evt;

				// @todo should pass the mouse state to event manager
			}
			break;

			case vl::cluster::EVT_MOUSE_MOVED :
			{
				OIS::MouseEvent evt( 0, OIS::MouseState() );
				stream >> evt;

				// @todo should pass the mouse state to event manager
			}
			break;

			case vl::cluster::EVT_JOYSTICK_PRESSED :
			{
				vl::JoystickEvent evt;
				int button;
				stream >> button >> evt;

				_game_manager->getEventManager()->update_joystick(evt);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_RELEASED :
			{
				vl::JoystickEvent evt;
				int button;
				stream >> button >> evt;

				_game_manager->getEventManager()->update_joystick(evt);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_AXIS :
			{
				vl::JoystickEvent evt;
				int axis;
				stream >> axis >> evt;

				_game_manager->getEventManager()->update_joystick(evt);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_POV :
			{
				vl::JoystickEvent evt;
				int pov;
				stream >> pov >> evt;

				_game_manager->getEventManager()->update_joystick(evt);
			}
			break;

			case vl::cluster::EVT_JOYSTICK_VECTOR3 :
			{
				vl::JoystickEvent evt;
				int index;
				stream >> index >> evt;

				_game_manager->getEventManager()->update_joystick(evt);
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
