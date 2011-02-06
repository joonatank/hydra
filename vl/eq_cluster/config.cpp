/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "actions_misc.hpp"

#include "dotscene_loader.hpp"

#include "tracker_serializer.hpp"
#include "base/filesystem.hpp"
#include "distrib_resource_manager.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include "resource.hpp"

// Necessary for retrieving other managers
#include "game_manager.hpp"
// HUH?
#include "python.hpp"
// Necessary for retrieving registered triggers, KeyTrigger and FrameTrigger.
#include "event_manager.hpp"
// Necessary for registering Player
#include "player.hpp"
#include <distrib_settings.hpp>

eqOgre::Config::Config( vl::GameManagerPtr man, vl::Settings const & settings, vl::EnvSettingsRefPtr env )
	: _game_manager(man), _settings(settings), _env(env), _server(0), _running(true)
{
	std::cout << "eqOgre::Config::Config" << std::endl;
	assert( _game_manager && _env );
	// TODO assert that the settings are valid
	assert( _env->isMaster() );
}

eqOgre::Config::~Config( void )
{}

bool
eqOgre::Config::init( uint64_t const & )
{
	std::cout << "eqOgre::Config::init" << std::endl;

	_game_manager->createSceneManager( this );

	_createServer();

	assert( _server );

	// Send the Environment
	_sendEnvironment();

	// Send the project
	_sendProject();

	// TODO register the objects
	// There needs to be a certain order in which they are registered
	// so that they can be unpacked correctly in the rendering thread
	// or we need an update structure that we can set the ids into.
	// or we can use object types in the message so that they can be dynamically
	// created in the rendering threads.

	// Create the player necessary for Trackers
	vl::PlayerPtr player = _game_manager->createPlayer();

	assert( player );
	// Registering Player in init
	registerObject( player );

	vl::SceneManager *sm = _game_manager->getSceneManager();
	assert( sm );

	// Register SceneManager
	std::cout << "Registering SceneManager" << std::endl;
	registerObject( sm );

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

	_updateServer();

	return true;
}

bool
eqOgre::Config::exit( void )
{
	std::cout << "eqOgre::Config::exit" << std::endl;
	std::cout << "Deregistering distributed data." << std::endl;

	//	TODO add cleanup server

	std::cout << "Config exited." << std::endl;
	return true;
}

uint32_t
eqOgre::Config::startFrame( uint64_t const &frameID )
{
// 	std::cout << "eqOgre::Config::startFrame" << std::endl;
	/// Process a time step in the game
	// New event interface
	_game_manager->getEventManager()->getFrameTrigger()->update();

	if( !_game_manager->step() )
	{ stopRunning(); }

	/// Provide the updates to slaves
	_updateServer();

	/// Render the scene
	_server->render();

	// TODO where the receive input messages should be?
	_receiveEventMessages();

	return true;
}

void
eqOgre::Config::finishFrame( void )
{
}

/// ------------ Private -------------
void
eqOgre::Config::_createServer( void )
{
	std::cout << "eqOgre::Config::_createServer" << std::endl;

	assert( !_server );

	_server = new vl::cluster::Server( _env->getServer().port );
}

void
eqOgre::Config::_updateServer( void )
{
// 	std::cout << "eqOgre::Config::_updateServer" << std::endl;
	// Handle received messages
	_server->receiveMessages();

	{
		// Create SceneGraph updates
		vl::cluster::Message msg( vl::cluster::MSG_UPDATE );
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _registered_objects.begin(); iter != _registered_objects.end();
			++iter )
		{
			if( (*iter)->isDirty() )
			{
				assert( (*iter)->getID() != vl::ID_UNDEFINED );
// 				std::cout << "eqOgre::Config::_updateServer dirty object found" << std::endl;
// 				std::cout << "Serializing object with id = " << (*iter)->getID() << std::endl;
				vl::cluster::ObjectData data( (*iter)->getID() );
				vl::cluster::ByteStream stream = data.getStream();
				(*iter)->pack(stream);
				data.copyToMessage(&msg);
			}
		}

		// Send SceneGraph to all listeners
		if( msg.size() > 0 )
		{
// 			std::cout << "Sending updates to all clients." << std::endl;
			_server->sendUpdate( msg );
		}
	}

	// New clients need the whole SceneGraph
	// TODO this is not good here
	// Provide a functor that can create the initial message
	if( _server->needsInit() )
	{
// 		std::cout << "New clients sending the whole SceneGraph" << std::endl;
		vl::cluster::Message msg( vl::cluster::MSG_INITIAL_STATE );
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _registered_objects.begin(); iter != _registered_objects.end();
			++iter )
		{
			assert( (*iter)->getID() != vl::ID_UNDEFINED );
// 			vl::cluster::UpdateMessageSerializer ser(&msg);
// 			std::cout << "Serializing object with id = " << (*iter)->getID() << std::endl;
// 			msg.write( (*iter)->getID() );
// 			vl::cluster::ObjectStream stream = ser.openObject( (*iter)->getID() );
			vl::cluster::ObjectData data( (*iter)->getID() );
			vl::cluster::ByteStream stream = data.getStream();
			(*iter)->pack( stream, vl::Distributed::DIRTY_ALL );
			data.copyToMessage(&msg);
		}

// 		std::cout << "Message = " << msg << std::endl;
		_server->sendInit( msg );
	}
}

void
eqOgre::Config::_sendEnvironment ( void )
{
	std::cout << "eqOgre::Config::_sendEnvironment" << std::endl;
	assert( _server );

	vl::SettingsByteData data;
	vl::cluster::ByteStream stream( &data );
	stream << _env;

	vl::cluster::Message msg( vl::cluster::MSG_ENVIRONMENT );
	data.copyToMessage( &msg );
	_server->sendEnvironment(msg);
}

void
eqOgre::Config::_sendProject ( void )
{
	std::cout << "eqOgre::Config::_sendProject" << std::endl;
	assert( _server );

	vl::SettingsByteData data;
	vl::cluster::ByteStream stream( &data );
	stream << _settings;

	vl::cluster::Message msg( vl::cluster::MSG_PROJECT );
	data.copyToMessage( &msg );
	_server->sendProject(msg);
}

void
eqOgre::Config::_createTracker( vl::EnvSettingsRefPtr settings )
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
eqOgre::Config::_loadScenes(void )
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
}

void
eqOgre::Config::_createQuitEvent(void )
{
	std::cout << "Creating QuitEvent" << std::endl;

	// Add a trigger event to Quit the Application
	assert( _game_manager );
	vl::QuitAction *quit = vl::QuitAction::create();
	quit->data = _game_manager;
	// Add trigger
	vl::KeyTrigger *trig = _game_manager->getEventManager()->createKeyPressedTrigger( OIS::KC_ESCAPE );
	trig->addAction(quit);
}

/// Event Handling
void
eqOgre::Config::_receiveEventMessages( void )
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
					std::cout << "eqOgre::Config::_receiveEventMessages : "
						<< "Unhandleded message type." << std::endl;
					break;
			}
		}
		delete msg;
	}
}

bool
eqOgre::Config::_handleKeyPressEvent( OIS::KeyEvent const &event )
{
	OIS::KeyCode kc = event.key;
	// Check if the there is a trigger for this event
	if( _game_manager->getEventManager()->hasKeyPressedTrigger( kc ) )
	{
		_game_manager->getEventManager()->getKeyPressedTrigger( kc )->update();
	}

	return true;
}

bool
eqOgre::Config::_handleKeyReleaseEvent( OIS::KeyEvent const &event )
{
	OIS::KeyCode kc = event.key;
	// Check if the there is a trigger for this event
	if( _game_manager->getEventManager()->hasKeyReleasedTrigger( kc ) )
	{
		_game_manager->getEventManager()->getKeyReleasedTrigger( kc )->update();
	}

	return true;
}

bool
eqOgre::Config::_handleMousePressEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id )
{
	return false;
}

bool
eqOgre::Config::_handleMouseReleaseEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id )
{
	return false;
}

bool
eqOgre::Config::_handleMouseMotionEvent( OIS::MouseEvent const &event )
{
	return true;
}
