/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-10
 *	@file config.cpp
 */

#include "config.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "actions_misc.hpp"

#include "dotscene_loader.hpp"

#include "tracker_serializer.hpp"
#include "base/filesystem.hpp"
#include "distrib_resource_manager.hpp"

#include "resource.hpp"

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

#include "base/string_utils.hpp"
#include "base/sleep.hpp"

namespace
{

vl::KEY_MOD getModifier( OIS::KeyCode kc )
{
	// Shift
	if( kc == OIS::KC_LSHIFT || kc == OIS::KC_RSHIFT )
	{
		return vl::MOD_SHIFT;
	}
	// Alt
	else if( kc == OIS::KC_LMENU || kc == OIS::KC_RMENU )
	{
		return vl::MOD_META;
	}
	// Control
	else if( kc == OIS::KC_RCONTROL || kc == OIS::KC_LCONTROL )
	{
		return vl::MOD_CTRL;
	}
	// Windows
	else if( kc == OIS::KC_LWIN || kc == OIS::KC_RWIN )
	{
		return vl::MOD_SUPER;
	}

	return vl::MOD_NONE;
}

}

vl::Config::Config( vl::Settings const & settings, vl::EnvSettingsRefPtr env )
	: _game_manager( new vl::GameManager )
	, _settings(settings)
	, _env(env)
	, _server(new vl::cluster::Server( _env->getServer().port ))
	, _gui(0)
	, _running(true)
	, _key_modifiers(MOD_NONE)
{
	std::cout << "vl::Config::Config" << std::endl;
	assert( _env );
	// TODO assert that the settings are valid
	assert( _env->isMaster() );

	_createResourceManager( settings, env );

	// TODO this should be done in python
	std::string song_name("The_Dummy_Song.ogg");
	_game_manager->createBackgroundSound(song_name);

	_game_manager->createSceneManager( this );
}

vl::Config::~Config( void )
{
	std::cout << "vl::Config::~Config" << std::endl;

	delete _game_manager;

	// Destroy server
	_server.reset();

	std::cout << "vl::Config::~Config : DONE" << std::endl;
}

void
vl::Config::init( void )
{
	std::cout << "vl::Config::init" << std::endl;
	Ogre::Timer timer;

	/// @todo most of this should be moved to the constructor, like object
	/// creation
	/// sending of initial messages to rendering threads should still be here

	// Send the Environment
	_sendEnvironment();

	// Send the project
	_sendProject();

	// Create the player necessary for Trackers
	vl::PlayerPtr player = _game_manager->createPlayer();

	assert( player );
	// Registering Player in init
	// TODO move this to do an automatic registration similar to SceneManager
	registerObject( player, OBJ_PLAYER );

	_gui = new vl::gui::GUI( this, vl::ID_UNDEFINED );
	_game_manager->setGUI(_gui);

	vl::SceneManager *sm = _game_manager->getSceneManager();
	assert( sm );

	_loadScenes();

	// Create Tracker needs the SceneNodes for mapping
	_createTracker( _env );

	std::vector<std::string> scripts = _settings.getScripts();

	std::cout << "Running " << scripts.size() << " python scripts." << std::endl;

	for( size_t i = 0; i < scripts.size(); ++i )
	{
		// Run init python scripts
		vl::TextResource script_resource;
		_game_manager->getReourceManager()->loadResource( scripts.at(i), script_resource );
		_game_manager->getPython()->executePythonScript( script_resource );
	}

	_createQuitEvent();

	std::cout << "vl::Config:: updating server" << std::endl;
	_updateServer();

	_stats.logInitTime( (double(timer.getMicroseconds()))/1e3 );
	_stats_timer.reset();
}

void
vl::Config::exit( void )
{
	std::cout << "vl::Config::exit" << std::endl;

	_server->shutdown();
	// TODO this should wait till all the clients have shutdown
	vl::msleep(1);
	_server->receiveMessages();

	std::cout << "Config exited." << std::endl;
}

void
vl::Config::render( void )
{
	Ogre::Timer timer;

	// Process a time step in the game
	// New event interface
	_game_manager->getEventManager()->getFrameTrigger()->update();
	_stats.logFrameProcessingTime( (double(timer.getMicroseconds()))/1e3 );

	timer.reset();
	if( !_game_manager->step() )
	{ stopRunning(); }
	_stats.logStepTime( (double(timer.getMicroseconds()))/1e3 );

	timer.reset();
	/// Provide the updates to slaves
	_updateServer();
	_stats.logStepTime( (double(timer.getMicroseconds()))/1e3 );

	timer.reset();
	/// Render the scene
	_server->render();
	_stats.logRenderingTime( (double(timer.getMicroseconds()))/1e3 );

	timer.reset();
	// TODO where the receive input messages should be?
	_receiveEventMessages();
	_stats.logEventProcessingTime( (double(timer.getMicroseconds()))/1e3 );

	// Print info every 10 seconds
	// @todo time limit and wether or not to print anything should be configurable
	if( _stats_timer.getMilliseconds() > 10e3 )
	{
		_stats.print();
		_stats.clear();
		_stats_timer.reset();
	}
}

/// ------------ Private -------------
void
vl::Config::_updateServer( void )
{
	assert( _server );

	// Handle received messages
	_server->receiveMessages();

	// New objects created need to send SG_CREATE message
	if( !getNewObjects().empty() )
	{
		vl::cluster::Message msg( vl::cluster::MSG_SG_CREATE );
		msg.write( getNewObjects().size() );
		for( size_t i = 0; i < getNewObjects().size(); ++i )
		{
			OBJ_TYPE type = getNewObjects().at(i).first;
			uint64_t id = getNewObjects().at(i).second->getID();
			msg.write( type );
			msg.write( id );
		}
		_server->sendCreate( msg );
		clearNewObjects();
	}

	{
		// Create SceneGraph updates
		vl::cluster::Message msg( vl::cluster::MSG_SG_UPDATE );
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _registered_objects.begin(); iter != _registered_objects.end();
			++iter )
		{
			if( (*iter)->isDirty() )
			{
				assert( (*iter)->getID() != vl::ID_UNDEFINED );
				vl::cluster::ObjectData data( (*iter)->getID() );
				vl::cluster::ByteStream stream = data.getStream();
				(*iter)->pack(stream);
				data.copyToMessage(&msg);
			}
		}

		_server->sendUpdate( msg );
	}

	// New clients need the whole SceneGraph
	// TODO this is not good here
	// Provide a functor that can create the initial message
	if( _server->needsInit() )
	{
		vl::cluster::Message msg( vl::cluster::MSG_SG_UPDATE );
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _registered_objects.begin(); iter != _registered_objects.end();
			++iter )
		{
			assert( (*iter)->getID() != vl::ID_UNDEFINED );
			vl::cluster::ObjectData data( (*iter)->getID() );
			vl::cluster::ByteStream stream = data.getStream();
			(*iter)->pack( stream, vl::Distributed::DIRTY_ALL );
			data.copyToMessage(&msg);
		}

		_server->sendInit( msg );
	}
}

void
vl::Config::_sendEnvironment ( void )
{
	std::cout << "vl::Config::_sendEnvironment" << std::endl;
	assert( _server );

	vl::SettingsByteData data;
	vl::cluster::ByteStream stream( &data );
	stream << _env;

	vl::cluster::Message msg( vl::cluster::MSG_ENVIRONMENT );
	data.copyToMessage( &msg );
	_server->sendEnvironment(msg);
}

void
vl::Config::_sendProject ( void )
{
	std::cout << "vl::Config::_sendProject" << std::endl;
	assert( _server );

	vl::SettingsByteData data;
	vl::cluster::ByteStream stream( &data );
	stream << _settings;

	vl::cluster::Message msg( vl::cluster::MSG_PROJECT );
	data.copyToMessage( &msg );
	_server->sendProject(msg);
}

void
vl::Config::_createTracker( vl::EnvSettingsRefPtr settings )
{
	std::cout << "Creating Trackers." << std::endl;

	vl::ClientsRefPtr clients = _game_manager->getTrackerClients();
	assert( clients );

	std::vector<std::string> tracking_files = settings->getTrackingFiles();

	std::cout << "Processing " << tracking_files.size() << " tracking files."
		<< std::endl;

	for( std::vector<std::string>::const_iterator iter = tracking_files.begin();
		 iter != tracking_files.end(); ++iter )
	{
		// Read a file
		std::cout << "Copy tracking resource : " << *iter << std::endl;

		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( *iter, resource );

		vl::TrackerSerializer ser( clients );
		ser.parseTrackers(resource);
	}

	// Start the trackers
	std::cout << "Starting " << clients->getNTrackers() << " trackers." << std::endl;
	for( size_t i = 0; i < clients->getNTrackers(); ++i )
	{
		clients->getTracker(i)->init();
	}

	// Create Action
	vl::HeadTrackerAction *action = vl::HeadTrackerAction::create();
	assert( _game_manager->getPlayer() );
	action->setPlayer( _game_manager->getPlayer() );

	// This will get the head sensor if there is one
	// If not it will create a FakeTracker instead
	std::string const head_trig_name("glassesTrigger");
	vl::EventManager *event_man = _game_manager->getEventManager();

	if( event_man->hasTrackerTrigger(head_trig_name) )
	{
		vl::TrackerTrigger *head_trigger = event_man->getTrackerTrigger(head_trig_name);
		head_trigger->setAction( action );
	}
	else
	{
		std::cout << "Creating a fake head tracker" << std::endl;
		vl::TrackerRefPtr tracker( new vl::FakeTracker );
		vl::SensorRefPtr sensor( new vl::Sensor );
		sensor->setDefaultPosition( Ogre::Vector3(0, 1.5, 0) );

		// Create the trigger
		std::cout << "Creating a fake head tracker trigger" << std::endl;
		vl::TrackerTrigger *head_trigger
			= _game_manager->getEventManager()->createTrackerTrigger(head_trig_name);
		head_trigger->setAction( action );
		sensor->setTrigger( head_trigger );

		std::cout << "Adding a fake head tracker" << std::endl;
		// Add the tracker
		tracker->setSensor( 0, sensor );
		clients->addTracker(tracker);
	}
	std::cout << "Trackers created." << std::endl;
}

void
vl::Config::_loadScenes( void )
{
	std::cout << "Loading Scenes for Project : " << _settings.getProjectName()
		<< std::endl;

	// Get scenes
	std::vector<vl::ProjSettings::Scene> scenes = _settings.getScenes();

	// If we don't have Scenes there is no point loading them
	if( !scenes.size() )
	{
		std::cout << "Project does not have any scene files." << std::endl;
		return;
	}
	else
	{
		std::cout << "Project has " << scenes.size() << " scene files."
			<< std::endl;
	}

	// Clean up old scenes
	// TODO this should be implemented

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string scene_file_name = scenes.at(i).getName();

		std::cout << "Loading scene file = " << scene_file_name << std::endl;

		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( scenes.at(i).getFile(), resource );

		vl::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( resource, _game_manager->getSceneManager() );

		std::cout << "Scene " << scene_file_name << " loaded." << std::endl;
	}

	_hideCollisionBarries();
}

void
vl::Config::_hideCollisionBarries( void )
{
	vl::SceneManager *sm = _game_manager->getSceneManager();

	for( size_t i = 0; i < sm->getNSceneNodes(); ++i )
	{
		vl::SceneNode *node = sm->getSceneNode(i);
		std::string str( node->getName().substr(0, 3) );
		vl::to_lower(str);
		if(  str == "cb_" )
		{ node->setVisibility(false); }
	}
}

void
vl::Config::_createQuitEvent(void )
{
	std::cout << "Creating QuitEvent" << std::endl;

	// Add a trigger event to Quit the Application
	assert( _game_manager );
	vl::QuitAction *quit = vl::QuitAction::create();
	quit->data = _game_manager;
	// Add trigger
	vl::KeyTrigger *trig = _game_manager->getEventManager()->createKeyPressedTrigger( OIS::KC_ESCAPE, MOD_META );
	trig->addAction(quit);
}

void
vl::Config::_createResourceManager( vl::Settings const &settings, vl::EnvSettingsRefPtr env )
{
	std::cout << "Initialising Resource Manager" << std::endl;

	std::cout << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	std::vector<std::string> paths = settings.getAuxDirectories();
	paths.push_back(settings.getProjectDir());
	for( size_t i = 0; i < paths.size(); ++i )
	{ _game_manager->getReourceManager()->addResourcePath( paths.at(i) ); }

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	std::cout << "Adding ${environment}/tracking to the resources paths." << std::endl;
	fs::path tracking_path( fs::path(env->getEnvironementDir()) / "tracking" );
	if( fs::is_directory(tracking_path) )
	{ _game_manager->getReourceManager()->addResourcePath( tracking_path.file_string() ); }
}

/// Event Handling
void
vl::Config::_receiveEventMessages( void )
{
	_server->receiveMessages();

	while( _server->inputMessages() )
	{
		vl::cluster::Message *msg = _server->popInputMessage();
		while( msg->size() )
		{
			vl::cluster::EventData data;
			data.copyFromMessage(msg);
			vl::cluster::ByteStream stream = data.getStream();
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

				default :
					std::cout << "vl::Config::_receiveEventMessages : "
						<< "Unhandleded message type." << std::endl;
					break;
			}
		}
		delete msg;
	}

	while( _server->commands() )
	{
		std::string cmd = _server->popCommand();
		_game_manager->getPython()->executePythonCommand(cmd);
	}
}

bool
vl::Config::_handleKeyPressEvent( OIS::KeyEvent const &event )
{
	OIS::KeyCode kc = event.key;

	if( getModifier(kc) != MOD_NONE )
	{
		// TODO this needs to release any event without the added modifier
		// is this even possible?

		// Add modifiers
		_key_modifiers = (KEY_MOD)(_key_modifiers | getModifier(kc));
	}

	// Modifiers can also be used as event triggers.

	// Check if the there is a trigger for this event
	if( _game_manager->getEventManager()->hasKeyPressedTrigger( kc, _key_modifiers ) )
	{
		_game_manager->getEventManager()->getKeyPressedTrigger( kc, _key_modifiers )->update();
	}
	// Try without modifiers if no event existed
	else if( _game_manager->getEventManager()->hasKeyPressedTrigger(kc) )
	{
		_game_manager->getEventManager()->getKeyPressedTrigger(kc)->update();
	}

	return true;
}

bool
vl::Config::_handleKeyReleaseEvent( OIS::KeyEvent const &event )
{
	OIS::KeyCode kc = event.key;

	// TODO if modifier is removed we should release the event with modifiers
	if( getModifier(kc) != MOD_NONE )
	{
		// TODO this should release all the events with this modifier
		// needs a list of trigger events with this particular modifier
		// so that they can be released
		// FIXME it is not possible with our current event system.
		// We have no knowledge of the event states that particular keys or what
		// not are. So it can not be done.

		// Remove modifiers
		// Negate the modifiers, this will produce only once if none of them is released
		// and zero to the one that has been released. So and will produce all the once
		// not released.
		_key_modifiers = (KEY_MOD)(_key_modifiers & (~getModifier(kc)));

		// TODO This needs to replace the released events with ones that
		// don't have the modifier
	}

	// Modifiers can also be used as event triggers.

	// Check if the there is a trigger for this event
	// TODO we need to check all the different modifiers also, not just
	// for the exact match
	if( _game_manager->getEventManager()->hasKeyReleasedTrigger( kc, _key_modifiers ) )
	{
		_game_manager->getEventManager()->getKeyReleasedTrigger( kc, _key_modifiers )->update();
	}
	// Try without modifiers if no event existed
	else if( _game_manager->getEventManager()->hasKeyReleasedTrigger(kc) )
	{
		_game_manager->getEventManager()->getKeyReleasedTrigger(kc)->update();
	}

	return true;
}

bool
vl::Config::_handleMousePressEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id )
{
	return false;
}

bool
vl::Config::_handleMouseReleaseEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id )
{
	return false;
}

bool
vl::Config::_handleMouseMotionEvent( OIS::MouseEvent const &event )
{
	return true;
}
