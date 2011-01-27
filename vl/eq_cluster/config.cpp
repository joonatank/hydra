/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "config_events.hpp"

#include "dotscene_loader.hpp"

#include "tracker_serializer.hpp"
#include "base/filesystem.hpp"
#include "eq_resource_manager.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include "resource.hpp"

// TODO this is for testing the audio moving to Client
#include "client.hpp"

// Necessary for retrieving other managers
#include "game_manager.hpp"
// HUH?
#include "python.hpp"
// Necessary for retrieving registered triggers, KeyTrigger and FrameTrigger.
#include "event_manager.hpp"
// Necessary for registering Player
#include "player.hpp"

uint16_t const SERVER_PORT = 4699;

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config( parent ), _server(0)
{}

eqOgre::Config::~Config( void )
{}

bool
eqOgre::Config::init( eq::uint128_t const & )
{
	_game_manager->createSceneManager( this );

	_createServer();

	EQASSERT( _server );
	// TODO register the objects
	eqOgre::SceneManager *sm = _game_manager->getSceneManager();
	EQASSERT( sm );

	// FIXME
	// Register SceneManager
	std::cout << "Registering SceneManager" << std::endl;
	registerObjectC( sm );


	// Create the player necessary for Trackers
	// Will be registered later
	// TODO support for multiple observers?
	vl::PlayerPtr player = _game_manager->createPlayer( getObservers().at(0) );

	_loadScenes();

	// Create Tracker needs the SceneNodes for mapping
	_createTracker(_settings);

	std::vector<std::string> scripts = _settings->getScripts();

	EQINFO << "Running " << scripts.size() << " python scripts." << std::endl;

	for( size_t i = 0; i < scripts.size(); ++i )
	{
		// Run init python scripts
		vl::TextResource script_resource;
		_game_manager->getReourceManager()->loadResource( scripts.at(i), script_resource );
		_game_manager->getPython()->executePythonScript( script_resource );
	}

	_createQuitEvent();

	EQINFO << "Registering data." << std::endl;

	eqOgre::ResourceManager *res_man
		= static_cast<eqOgre::ResourceManager *>( _game_manager->getReourceManager() );
	registerObjectC( res_man );

	EQASSERT( player );
	// Registering Player in init
	if( !registerObject( player ) )
	{ return false; }

	_distrib_settings.setSceneManagerID( _game_manager->getSceneManager()->getID() );
	_distrib_settings.setResourceManagerID( res_man->getID() );
	_distrib_settings.setPlayerID( player->getID() );

	EQINFO << "Registering Settings" << std::endl;
	if( !registerObject( &_distrib_settings ) )
	{ return false; }

	if( !eq::Config::init( _distrib_settings.getID() ) )
	{ return false; }

	EQINFO << "Config::init DONE" << std::endl;

	return true;
}

bool
eqOgre::Config::exit( void )
{
	// First let the children clean up
	bool retval = eq::Config::exit();

	EQINFO << "Deregistering distributed data." << std::endl;

	_distrib_settings.setSceneManagerID( vl::ID_UNDEFINED );

	eqOgre::ResourceManager *res_man =
		static_cast<eqOgre::ResourceManager *>( _game_manager->getReourceManager() );

	_distrib_settings.setResourceManagerID( vl::ID_UNDEFINED );

	deregisterObject( _game_manager->getPlayer() );
	_distrib_settings.setPlayerID( eq::base::UUID::ZERO );

	deregisterObject( &_distrib_settings );

	EQINFO << "Config exited." << std::endl;
	return retval;
}

void
eqOgre::Config::setSettings( vl::SettingsRefPtr settings )
{
	EQASSERT( settings );
	EQASSERT( _game_manager );

	_settings = settings;
	_distrib_settings.copySettings(_settings, _game_manager->getReourceManager() );
}

eqOgre::SceneNodePtr
eqOgre::Config::createSceneNode(const std::string& name)
{
	SceneNodePtr node = SceneNode::create( name );
	_addSceneNode( node );
	return node;
}

eqOgre::SceneNode *
eqOgre::Config::getSceneNode(const std::string& name)
{
	return _game_manager->getSceneManager()->getSceneNode(name);
}

void eqOgre::Config::setGameManager(vl::GameManagerPtr man)
{
	EQASSERT( man );
	_game_manager = man;
}


uint32_t
eqOgre::Config::startFrame( eq::uint128_t const &frameID )
{
	// New event interface
	_game_manager->getEventManager()->getFrameTrigger()->update();

	if( !_game_manager->step() )
	{ stopRunning(); }

	_game_manager->getPlayer()->commit();

	// TODO pack scene manager
	_updateServer();

//	eq::uint128_t version = _game_manager->getSceneManager()->commitAll();

	uint32_t retval = eq::Config::startFrame( 0 );

	return retval;
}

/// ------------ Private -------------
void
eqOgre::Config::_createServer( void )
{
	std::cout << "eqOgre::Config::_createServer" << std::endl;

	EQASSERT( !_server );

	// TODO configurable server port
	_server = new vl::cluster::Server( SERVER_PORT );
}

void
eqOgre::Config::_updateServer( void )
{
// 	std::cout << "eqOgre::Config::_updateServer" << std::endl;
	// Handle received messages
	_server->mainloop();

	if( _server->oldClients() )
	{
		// Create SceneGraph updates
		vl::cluster::Message msg( vl::cluster::MSG_UPDATE );
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _registered_objects.begin(); iter != _registered_objects.end();
			++iter )
		{
			if( (*iter)->isDirty() )
			{
// 				std::cout << "eqOgre::Config::_updateServer dirty object found" << std::endl;
				EQASSERT( (*iter)->getID() != vl::ID_UNDEFINED );
// 				std::cout << "Serializing object with id = " << (*iter)->getID() << std::endl;
				msg.write( (*iter)->getID() );
				(*iter)->pack(msg);
			}
		}

		// Send SceneGraph to all listeners
		if( msg.size() > 0 )
		{
// 			std::cout << "Sending updates to all clients." << std::endl;
			_server->sendToAll( msg );
		}
	}

	// New clients need the whole SceneGraph
	if( _server->newClients() )
	{
// 		std::cout << "New clients sending the whole SceneGraph" << std::endl;
		vl::cluster::Message msg( vl::cluster::MSG_UPDATE );
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _registered_objects.begin(); iter != _registered_objects.end();
			++iter )
		{
			EQASSERT( (*iter)->getID() != vl::ID_UNDEFINED );
// 			std::cout << "Serializing object with id = " << (*iter)->getID() << std::endl;
			msg.write( (*iter)->getID() );
			(*iter)->pack( msg, vl::Distributed::DIRTY_ALL );
		}

// 		std::cout << "Message = " << msg << std::endl;
		// Send SceneGraph to all listeners
		_server->sendToNewClients( msg );
	}
}

void
eqOgre::Config::_addSceneNode(eqOgre::SceneNode* node)
{
	// Check that no two nodes have the same name
	// TODO should be in the frame data, as it can neither store multiple
	// SceneNodes with same names
	for( size_t i = 0; i < _game_manager->getSceneManager()->getNSceneNodes(); ++i )
	{
		SceneNodePtr ptr = _game_manager->getSceneManager()->getSceneNode(i);
		if( ptr == node || ptr->getName() == node->getName() )
		{
			// TODO is this the right exception?
			BOOST_THROW_EXCEPTION( vl::duplicate() );
		}
	}

	_game_manager->getSceneManager()->addSceneNode( node );
}

void
eqOgre::Config::_createTracker( vl::SettingsRefPtr settings )
{
	EQINFO << "Creating Trackers." << std::endl;

	vl::ClientsRefPtr clients = _game_manager->getTrackerClients();
	EQASSERT( clients );

	std::vector<std::string> tracking_files = settings->getTrackingFiles();

	EQINFO << "Processing " << tracking_files.size() << " tracking files."
		<< std::endl;

	for( std::vector<std::string>::const_iterator iter = tracking_files.begin();
		 iter != tracking_files.end(); ++iter )
	{
		// Read a file
		EQINFO << "Copy tracking resource : " << *iter << std::endl;

		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( *iter, resource );

		vl::TrackerSerializer ser( clients );
		ser.parseTrackers(resource);
	}

	// Start the trackers
	EQINFO << "Starting " << clients->getNTrackers() << " trackers." << std::endl;
	for( size_t i = 0; i < clients->getNTrackers(); ++i )
	{
		clients->getTracker(i)->init();
	}

	// Create Action
	eqOgre::HeadTrackerAction *action = eqOgre::HeadTrackerAction::create();
	EQASSERT( _game_manager->getPlayer() );
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
		EQINFO << "Creating a fake head tracker" << std::endl;
		vl::TrackerRefPtr tracker( new vl::FakeTracker );
		vl::SensorRefPtr sensor( new vl::Sensor );
		sensor->setDefaultPosition( Ogre::Vector3(0, 1.5, 0) );

		// Create the trigger
		EQINFO << "Creating a fake head tracker trigger" << std::endl;
		vl::TrackerTrigger *head_trigger
			= _game_manager->getEventManager()->createTrackerTrigger(head_trig_name);
		head_trigger->setAction( action );
		sensor->setTrigger( head_trigger );

		EQINFO << "Adding a fake head tracker" << std::endl;
		// Add the tracker
		tracker->setSensor( 0, sensor );
		clients->addTracker(tracker);
	}
	EQINFO << "Trackers created." << std::endl;
}

void
eqOgre::Config::_loadScenes(void )
{
	EQINFO << "Loading Scenes for Project : " << _settings->getProjectName()
		<< std::endl;

	// Get scenes
	std::vector<vl::ProjSettings::Scene> scenes = _settings->getScenes();

	// If we don't have Scenes there is no point loading them
	if( !scenes.size() )
	{
		EQINFO << "Project does not have any scene files." << std::endl;
		return;
	}
	else
	{
		EQINFO << "Project has " << scenes.size() << " scene files."
			<< std::endl;
	}

	// Clean up old scenes
	// TODO this should be implemented

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string scene_file_name = scenes.at(i).getName();

		EQINFO << "Loading scene file = " << scene_file_name << std::endl;

		vl::TextResource resource;
		_game_manager->getReourceManager()->loadResource( scenes.at(i).getFile(), resource );

		vl::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( resource, this );

		EQINFO << "Scene " << scene_file_name << " loaded." << std::endl;
	}
}

void
eqOgre::Config::_createQuitEvent(void )
{
	EQINFO << "Creating QuitEvent" << std::endl;

	// Add a trigger event to Quit the Application
	EQASSERT( _game_manager );
	QuitAction *quit = QuitAction::create();
	quit->data = _game_manager;
	// Add trigger
	vl::KeyTrigger *trig = _game_manager->getEventManager()->createKeyPressedTrigger( OIS::KC_ESCAPE );
	trig->addAction(quit);
}

/// Event Handling
bool
eqOgre::Config::handleEvent( const eq::ConfigEvent* event )
{
	bool redraw = false;
	switch( event->data.type )
	{
		case eq::Event::KEY_PRESS :

			redraw = _handleKeyPressEvent(event->data.keyPress);
			break;

		case eq::Event::KEY_RELEASE :
			redraw = _handleKeyReleaseEvent(event->data.keyRelease);

			break;

		case eq::Event::POINTER_BUTTON_PRESS:
			redraw = _handleMousePressEvent(event->data.pointerButtonPress);
			break;

		case eq::Event::POINTER_BUTTON_RELEASE:
			redraw = _handleMouseReleaseEvent(event->data.pointerButtonRelease);
			break;

		case eq::Event::POINTER_MOTION:
			redraw = _handleMouseMotionEvent(event->data.pointerMotion);
			break;

		case eq::Event::WINDOW_CLOSE :
		case eq::Event::WINDOW_HIDE :
		case eq::Event::WINDOW_EXPOSE :
		case eq::Event::WINDOW_RESIZE :
		case eq::Event::WINDOW_SHOW :
			break;

		default :
			break;
	}

	eq::Config::handleEvent( event );

	return true;
}

bool
eqOgre::Config::_handleKeyPressEvent( const eq::KeyEvent& event )
{
	OIS::KeyCode kc( (OIS::KeyCode )(event.key) );

	// Check if the there is a trigger for this event
	if( _game_manager->getEventManager()->hasKeyPressedTrigger( kc ) )
	{
		_game_manager->getEventManager()->getKeyPressedTrigger( kc )->update();
	}

	return true;
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	OIS::KeyCode kc = (OIS::KeyCode )(event.key);

	// Check if the there is a trigger for this event
	if( _game_manager->getEventManager()->hasKeyReleasedTrigger( kc ) )
	{
		_game_manager->getEventManager()->getKeyReleasedTrigger( kc )->update();
	}

	return true;
}

bool
eqOgre::Config::_handleMousePressEvent(const eq::PointerEvent& event)
{
//	std::cerr << "Config received mouse button press event. Button = "
//		<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseReleaseEvent(const eq::PointerEvent& event)
{
//	std::cerr << "Config received mouse button release event. Button = "
//		<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseMotionEvent(const eq::PointerEvent& event)
{
	return true;
}

bool
eqOgre::Config::_handleJoystickEvent(const eq::MagellanEvent& event)
{
	return false;
}
