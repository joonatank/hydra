#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "config_python.hpp"
#include "config_events.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config ( parent ), _event_manager( new EventManager ),
	  _audio_manager(0), _background_sound(0)
{
	// Add events
	_event_manager->addEventFactory( new BasicEventFactory );
	_event_manager->addEventFactory( new ToggleEventFactory );
	_event_manager->addEventFactory( new TransformationEventFactory );
	// Add triggers
	_event_manager->addTriggerFactory( new KeyTriggerFactory );
	_event_manager->addTriggerFactory( new KeyPressedTriggerFactory );
	_event_manager->addTriggerFactory( new KeyReleasedTriggerFactory );
	_event_manager->addTriggerFactory( new FrameTriggerFactory );
	// Add operations
	_event_manager->addOperationFactory( new QuitOperationFactory );
	_event_manager->addOperationFactory( new ReloadSceneFactory );
	_event_manager->addOperationFactory( new AddTransformOperationFactory );
	_event_manager->addOperationFactory( new RemoveTransformOperationFactory );
	_event_manager->addOperationFactory( new HideOperationFactory );
	_event_manager->addOperationFactory( new ShowOperationFactory );
	_event_manager->addOperationFactory( new ToggleMusicFactory );
	// TODO add transformation operations
	
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
	registerObject( _settings.get() );
    
	if( !eq::Config::init( _settings->getID() ) )
	{ return false; }

	return true;
}

void eqOgre::Config::setSettings(eqOgre::SettingsRefPtr settings)
{
	if( settings )
	{
		_settings = settings;
		_createTracker(_settings);
	}
}

// TODO this should be moved to private
void
eqOgre::Config::addSceneNode(eqOgre::SceneNode* node)
{
	_frame_data.addSceneNode( node );
}

// TODO implement
// TODO this should be moved to private
void
eqOgre::Config::removeSceneNode(eqOgre::SceneNode* node)
{
	std::cerr << "eqOgre::Config::removeSceneNode" << " : NOT IMPLEMENTED" << std::endl;
}

// TODO implement
eqOgre::SceneNode *
eqOgre::Config::getSceneNode(const std::string& name)
{
	return 0;
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
	// TODO add selectable sensor
	if( _tracker )
	{
		_tracker->mainloop();
		if( _tracker->getNSensors() > 0 )
		{
			Ogre::Matrix4 head( _tracker->getOrientation(0) );
			head.setTrans( _tracker->getPosition(0) );
			_setHeadMatrix(head);
		}
	}

	// Init the transformation structures
	// TODO should be moved to Config::init or equivalent
	static bool inited = false;

	if( !inited )
	{
		try {
			// Init the embedded python
			_initPython();

			// TODO the script file should not be hard-coded
			// they should be in Settings
			// like Settings::getInitScripts
			// and Settings::getFrameScripts
			const std::string initScript("script.py");

			// Run init python script
			_runPythonScript( initScript);

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
			= (QuitOperation *)( _event_manager->createOperation( "QuitOperation" ) );
		quit->setConfig(this);
		Event *event = _event_manager->createEvent( "Event" );
		event->setOperation(quit);
		// Add trigger
		KeyTrigger *trig = (KeyTrigger *)( _event_manager->createTrigger( "KeyTrigger" ) );
		trig->setKey( OIS::KC_ESCAPE );
		event->addTrigger(trig);
		_event_manager->addEvent( event );

		_initAudio();
		inited = true;
	}

	// ProcessEvents does not store the pointer anywhere
	// so it's safe to allocate to the stack
	FrameTrigger frame_trig;
	_event_manager->processEvents( &frame_trig );

	uint32_t version = _frame_data.commitAll();
//	std::cout << "FrameData version = " << version << std::endl;

	return eq::Config::startFrame( version );
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
	if( settings->trackerOn() )
	{
		EQINFO << "Creating VRPN Tracker." << std::endl;
		_tracker.reset( new vl::vrpnTracker( settings->getTrackerAddress() ) );
	}
	else
	{
		EQINFO << "Creating Fake Tracker." << std::endl;
		_tracker.reset( new vl::FakeTracker( ) );
	}

	_tracker->setOrientation( 0, settings->getTrackerDefaultOrientation() );
	_tracker->setPosition( 0, settings->getTrackerDefaultPosition() );
	_tracker->init();
}

void
eqOgre::Config::_setHeadMatrix( Ogre::Matrix4 const &m )
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

void eqOgre::Config::_initPython(void )
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
	std::cout << "running file " << scriptFile << "..." << std::endl;
	// Run a python script in an empty environment.
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
	KeyPressedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _event_manager->processEvents( &trig );
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	KeyReleasedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _event_manager->processEvents( &trig );
}

bool
eqOgre::Config::_handleMousePressEvent(const eq::PointerEvent& event)
{
//			std::cerr << "Config received mouse button press event. Button = "
//				<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseReleaseEvent(const eq::PointerEvent& event)
{
//			std::cerr << "Config received mouse button release event. Button = "
//				<< event->data.pointer.button << std::endl;
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
