/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "config_python.hpp"
#include "config_events.hpp"

#include "dotscene_loader.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config ( parent ), _event_manager( new vl::EventManager ),
	  _audio_manager(0), _background_sound(0)
{
	// Add events
	_event_manager->addEventFactory( new vl::BasicEventFactory );
	_event_manager->addEventFactory( new vl::ToggleEventFactory );
	_event_manager->addEventFactory( new eqOgre::TransformationEventFactory );
	// Add triggers
	_event_manager->addTriggerFactory( new vl::KeyTriggerFactory );
	_event_manager->addTriggerFactory( new vl::KeyPressedTriggerFactory );
	_event_manager->addTriggerFactory( new vl::KeyReleasedTriggerFactory );
	_event_manager->addTriggerFactory( new vl::FrameTriggerFactory );
	// Add operations
	_event_manager->addActionFactory( new eqOgre::QuitOperationFactory );
	_event_manager->addActionFactory( new eqOgre::ReloadSceneFactory );
	_event_manager->addActionFactory( new eqOgre::AddTransformOperationFactory );
	_event_manager->addActionFactory( new eqOgre::RemoveTransformOperationFactory );
	_event_manager->addActionFactory( new eqOgre::HideActionFactory );
	_event_manager->addActionFactory( new eqOgre::ShowActionFactory );
	_event_manager->addActionFactory( new eqOgre::ToggleMusicFactory );
	_event_manager->addActionFactory( new eqOgre::ActivateCameraFactory );
}

eqOgre::Config::~Config()
{
	_exitAudio();
}

void
eqOgre::Config::mapData( uint32_t const initDataID )
{
	EQASSERT( _settings );
	if( _settings->getID() == EQ_ID_INVALID )
	{
		EQCHECK( mapObject( _settings.get(), initDataID ));
        unmapObject( _settings.get() ); // data was retrieved, unmap immediately
	}
    else  // appNode, _initData is registered already
    {
        EQASSERT( _settings->getID() == initDataID )
	}
}

// TODO should this be deregister or unmap?
// If it's deregister rename the function and call it from AppNode
// If it's unmap it should be called from Channel not from AppNode
// The unmap is moved to Channel and FrameData
/*
void
eqOgre::Config::unmapData( void )
{
//	std::cerr << "Config::unmapData" << std::endl;
//	_frame_data.deregisterData(this);
}
*/

bool
eqOgre::Config::init( uint32_t const )
{
	/// Register data
	std::cerr << "eqOgre::Config::init : registering data" << std::endl;

	_frame_data.registerData(this);

	_settings->setFrameDataID( _frame_data.getID() );
	std::cerr << "Registering Settings" << std::endl;
	registerObject( _settings.get() );
	std::cerr << "Settings registered" << std::endl;

	if( !eq::Config::init( _settings->getID() ) )
	{ return false; }

	std::cerr << "Config::init DONE" << std::endl;
	return true;
}

void
eqOgre::Config::setSettings(eqOgre::SettingsRefPtr settings)
{
	if( settings )
	{
		_settings = settings;
		_createTracker(_settings);
	}
}

eqOgre::SceneNodePtr
eqOgre::Config::createSceneNode(const std::string& name)
{
	SceneNodePtr node = SceneNode::create( name );
	_addSceneNode( node );
	return node;
}

// TODO implement
void
eqOgre::Config::removeSceneNode(eqOgre::SceneNodePtr node)
{
	std::cerr << "eqOgre::Config::removeSceneNode" << " : NOT IMPLEMENTED" << std::endl;
}

eqOgre::SceneNode *
eqOgre::Config::getSceneNode(const std::string& name)
{
	return _frame_data.getSceneNode(name);
}

// TODO implement
void
eqOgre::Config::resetScene(void )
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
}

void
eqOgre::Config::toggleBackgroundSound()
{
	if( !_background_sound )
	{
		std::cerr << "Config::toggleBackgroundSound DOES NOT EXIST" << std::endl;
		return;
	}

	if( _background_sound->isPlaying() )
	{ _background_sound->pause(); }
	else
	{ _background_sound->play2d(false); }
}


uint32_t
eqOgre::Config::startFrame( const uint32_t frameID )
{
	//std::cerr << "eqOgre::Config::startFrame" << std::endl;

	// Process Tracking
	// If we have a tracker object update it, the update will handle all the
	// callbacks and appropriate updates (head matrix and scene nodes).
	if( _tracker )
	{
		_tracker->mainloop();
	}

	// Init the transformation structures
	// TODO should be moved to Config::init or equivalent
	static bool inited = false;

	std::stringstream ss;

	if( !inited )
	{
		_loadScenes();

		try {
			// Init the embedded python
			_initPython();

			std::vector<std::string> scripts = _settings->getScripts();

			ss.str("");
			ss << "Running " << scripts.size() << " python scripts.";
			Ogre::LogManager::getSingleton().logMessage( ss.str() );

			for( size_t i = 0; i < scripts.size(); ++i )
			{
				ss.str("");
				ss << "Running python script = " << scripts.at(i);
				Ogre::LogManager::getSingleton().logMessage( ss.str() );

				// Run init python scripts
				_runPythonScript( scripts.at(i) );
			}
		}
		// Some error handling so that we can continue the application
		// Will print error in std::cerr
		catch( ... )
		{
			std::cerr << "Exception occured in python script." << std::endl;

		}
		if (PyErr_Occurred())
		{
			PyErr_Print();
		}

		// Find ogre event so we can toggle it on/off
		// TODO add function to find Events
		// Ogre Rotation event, used to toggle the event on/off
		/*	FIXME new design
		TransformationEvent ogre_event;
		for( size_t i = 0; i < _trans_events.size(); ++i )
		{
			SceneNodePtr node = _trans_events.at(i).getSceneNode();
			if( node )
			{
				if( node->getName() == "ogre" )
				{
					ogre_event = _trans_events.at(i);
					break;
				}
			}
		}

		Trigger *trig = new KeyTrigger( OIS::KC_SPACE, false );
		Operation *add_oper = new AddTransformEvent( this, ogre_event );
		Operation *rem_oper = new RemoveTransformEvent( this, ogre_event );
		Event *event = new ToggleEvent( hasEvent(ogre_event), add_oper, rem_oper, trig );
		_events.push_back( event );
		*/

		// Add a trigger event to Quit the Application
		QuitOperation *quit
			= (QuitOperation *)( _event_manager->createAction( "QuitOperation" ) );
		quit->setConfig(this);
		vl::Event *event = _event_manager->createEvent( "Event" );
		event->setAction(quit);
		// Add trigger
		vl::KeyTrigger *trig = (vl::KeyTrigger *)( _event_manager->createTrigger( "KeyTrigger" ) );
		trig->setKey( OIS::KC_ESCAPE );
		event->addTrigger(trig);
		_event_manager->addEvent( event );

		_initAudio();
		inited = true;
	}

	// ProcessEvents does not store the pointer anywhere
	// so it's safe to allocate to the stack
	vl::FrameTrigger frame_trig;
	_event_manager->processEvents( &frame_trig );

	uint32_t version = _frame_data.commitAll();
//	std::cout << "FrameData version = " << version << std::endl;

	return eq::Config::startFrame( version );
}


void
eqOgre::Config::setHeadMatrix( Ogre::Matrix4 const &m )
{
	// Note: real applications would use one tracking device per observer
	const eq::Observers& observers = getObservers();
	for( eq::Observers::const_iterator i = observers.begin();
		i != observers.end(); ++i )
	{
		// When head matrix is set equalizer automatically applies it to the
		// GL Modelview matrix as first transformation
		(*i)->setHeadMatrix( vl::math::convert(m) );
	}
}





/// ------------ Private -------------

void
eqOgre::Config::_addSceneNode(eqOgre::SceneNode* node)
{
	// Implement checking
	// TODO should be in the frame data, as it can neither store multiple
	// SceneNodes with same names
	for( size_t i = 0; i < _frame_data.getNSceneNodes(); ++i )
	{
		SceneNodePtr ptr = _frame_data.getSceneNode(i);
		if( ptr == node || ptr->getName() == node->getName() )
		{
			// TODO is this the right exception?
			BOOST_THROW_EXCEPTION( vl::duplicate() );
		}
	}

	_frame_data.addSceneNode( node );
}

void
eqOgre::Config::_initAudio(void )
{
	//Create an Audio Manager
	_audio_manager = cAudio::createAudioManager(true);

	//Create an audio source and load a sound from a file
	_background_sound = _audio_manager->create("music","The_Dummy_Song.ogg",true);
}


void
eqOgre::Config::_exitAudio(void )
{
	//Shutdown cAudio
	if( _audio_manager )
	{
		_audio_manager->shutDown();
		cAudio::destroyAudioManager(_audio_manager);
	}
}

void
eqOgre::Config::_createTracker(  vl::SettingsRefPtr settings )
{
	// FIXME no tracking config for now
	// TODO this should read the configs (which can be found in EnvSettings)
	// and create all the trackers based upon those.
//	if( settings->trackerOn() )
	/// Creating tracker
	if( true )
	{
		EQINFO << "Creating VRPN Tracker." << std::endl;
		_tracker.reset( new vl::vrpnTracker( "localhost", "glasses" ) );
		//_tracker.reset( new vl::vrpnTracker( settings->getTrackerAddress() ) );
	}
	else
	{
		EQINFO << "Creating Fake Tracker." << std::endl;
		_tracker.reset( new vl::FakeTracker( ) );
	}

	/// Set default values for sensor
	// TODO move to config startFrame init
	Ogre::Quaternion q(Ogre::Quaternion::IDENTITY);// = settings->getTrackerDefaultOrientation()
	Ogre::Vector3 v(0, 1.5, 0 ); //= settings->getTrackerDefaultPosition();
	/// Create sensor
	vl::SensorRefPtr sensor( new vl::Sensor( v,q ) );
	_tracker->setSensor(0, sensor);

	/// Create Action
	eqOgre::HeadTrackerAction *action = new eqOgre::HeadTrackerAction;
	action->setConfig(this);

	/// Create Trigger
	// TODO should be moved to using event manager
	vl::TrackerTrigger *trigger = new vl::TrackerTrigger;
	trigger->setName( "head" );
	trigger->setAction( action );

	sensor->setTrigger(trigger);

	/// Start the tracker
	_tracker->init();
}

void
eqOgre::Config::_loadScenes(void )
{
	std::stringstream ss;

	// Get scenes
	std::vector<vl::ProjSettings::Scene const *> scenes = _settings->getScenes();

	ss << "Loading Scenes for Project : " << _settings->getProjectName();
	Ogre::LogManager::getSingleton().logMessage( ss.str() );
	ss.str("");

	// If we don't have Scenes there is no point loading them
	if( !scenes.size() )
	{
		ss << "Project does not have any scene files.";
		Ogre::LogManager::getSingleton().logMessage( ss.str() );
		return;
	}
	else
	{
		ss << "Project has " << scenes.size() << " scene files.";
		Ogre::LogManager::getSingleton().logMessage( ss.str() );
	}

	// Clean up old scenes
	// TODO this should be implemented

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string const &scene_file = scenes.at(i)->getFile();

		ss.str("");
		std::string message = "Loading scene : " + scene_file;
		Ogre::LogManager::getSingleton().logMessage( message );

		vl::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( scene_file,
							Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
							this );

		message = "Scene loaded";
		Ogre::LogManager::getSingleton().logMessage( message );
	}
}

void
eqOgre::Config::_initPython(void )
{
	Py_Initialize();

	// Add the module to the python interpreter
	// NOTE the name parameter does not rename the module
	// No idea why it's there
	if (PyImport_AppendInittab("eqOgre_python", initeqOgre_python) == -1)
		throw std::runtime_error("Failed to add eqOgre to the interpreter's "
				"builtin modules");

	// Retrieve the main module
	python::object main = python::import("__main__");

	// Retrieve the main module's namespace
	_global = main.attr("__dict__");

	// Import eqOgre module
    python::handle<> ignored(( PyRun_String("from eqOgre_python import *\n"
                                    "print 'eqOgre imported'       \n",
                                    Py_file_input,
                                    _global.ptr(),
                                    _global.ptr() ) ));

	// Add a global managers i.e. this and EventManager
	_global["config"] = python::ptr<>( this );
	_global["event_manager"] = python::ptr<>( _event_manager );
}

void eqOgre::Config::_runPythonScript(const std::string& scriptFile)
{
	std::stringstream ss;
	ss << "Running python script file " << scriptFile << ".";
	Ogre::LogManager::getSingleton().logMessage( ss.str() );

	// Run a python script.
	python::object result = python::exec_file(scriptFile.c_str(), _global, _global);
}

/// Event Handling
char const *CB_INFO_TEXT = "Config : OIS event received : ";

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

	return redraw;
}

bool
eqOgre::Config::_handleKeyPressEvent( const eq::KeyEvent& event )
{
	vl::KeyPressedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _event_manager->processEvents( &trig );
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	vl::KeyReleasedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _event_manager->processEvents( &trig );
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
