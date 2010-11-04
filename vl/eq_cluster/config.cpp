#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "config_python.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#include "frame_data_events.hpp"

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config ( parent )
{}

eqOgre::Config::~Config()
{}

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

bool eqOgre::Config::addEvent(const eqOgre::TransformationEvent& event)
{
	_trans_events.push_back(event);
	return true;
}

bool eqOgre::Config::removeEvent(const eqOgre::TransformationEvent& event)
{
	std::vector<TransformationEvent>::iterator iter;
	for( iter = _trans_events.begin(); iter != _trans_events.end(); ++iter )
	{
		if( *iter == event )
		{
			_trans_events.erase(iter);
			return true;
		}
	}

	return false;
}

// TODO implemente
bool eqOgre::Config::hasEvent(const eqOgre::TransformationEvent& event)
{
	std::cerr << "eqOgre::Config::hasEvent" << " : NOT IMPLEMENTED" << std::endl;
	return false;
}


void eqOgre::Config::addSceneNode(eqOgre::SceneNode* node)
{
	_frame_data.addSceneNode( node );
}

// TODO implemente
void eqOgre::Config::removeSceneNode(eqOgre::SceneNode* node)
{
	std::cerr << "eqOgre::Config::removeSceneNode" << " : NOT IMPLEMENTED" << std::endl;
}


uint32_t 
eqOgre::Config::startFrame( const uint32_t frameID )
{
	//std::cerr << "eqOgre::Config::startFrame" << std::endl;
	// Process Tracking
	// TODO add selectable sensor
	// TODO should be moved to Client where it belongs here it's called
	// by all the Nodes
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

			// Find ogre event so we can toggle it on/off
			// TODO add an Operation to remove TransformationEvents
			for( size_t i = 0; i < _trans_events.size(); ++i )
			{
				SceneNodePtr node = _trans_events.at(i).getSceneNode();
				if( node )
				{
					if( node->getName() == "ogre" )
					{
						_ogre_event = _trans_events.at(i);
						break;
					}
				}
			}

			// TODO add an Operation to quit
			
			// Add a trigger event to reset the Scene
			Trigger *trig = new KeyTrigger( OIS::KC_R, false );
			Operation *oper = new ReloadScene( &_frame_data, 5 );
			Event *event = new Event;
			event->addTrigger(trig);
			event->setOperation(oper);
			_events.push_back( event );
			std::cerr << "Created ReloadScene Event : currently we have "
				<< _events.size() << " events." << std::endl;
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
		inited = true;
	}

	// Really Move the objects
	for( size_t i = 0; i < _trans_events.size(); ++i )
	{
		_trans_events.at(i)();
	}

	uint32_t version = _frame_data.commitAll();
//	std::cout << "FrameData version = " << version << std::endl;

	return eq::Config::startFrame( version );
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

	// Add a global manager i.e. this
	_global["config"] = python::ptr<>( this );
}

void eqOgre::Config::_runPythonScript(const std::string& scriptFile)
{
	std::cout << "running file " << scriptFile << "..." << std::endl;
	// Run a python script in an empty environment.
	python::object result = python::exec_file(scriptFile.c_str(), _global, _global);
}

/// Event Handling
char const *CB_INFO_TEXT = "Config : OIS event received : ";

// TODO the event handling should be separated to multiple functions
// handleKeyPress, handleKeyRelease, handlePointerMotion etc.
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
	// Used for toggle events
	static bool ogre_event_on = true;

	OIS::KeyCode key = (OIS::KeyCode )(event.key);
	for( std::vector<TransformationEvent>::iterator iter = _trans_events.begin();
		iter != _trans_events.end(); ++iter )
	{
		iter->keyPressed(key);
	}

	for( std::vector<Event *>::iterator iter = _events.begin();
		iter != _events.end(); ++iter )
	{
		KeyTrigger trig(key, false);
		(*iter)->processTrigger(&trig);
	}

    switch( key )
    {
		case OIS::KC_ESCAPE :
		case OIS::KC_Q :
			{
				std::cerr << CB_INFO_TEXT << "Escape or Q pressed. Will quit now. " << std::endl;
				// TODO should quit cleanly
				stopRunning();
			}
			return true;

		case OIS::KC_SPACE :
			// TODO should be moved to new class ToggleEvent
			// TODO could be implemented with hasEvent -> removeEvent/addEvent
			if( ogre_event_on )
			{
				removeEvent(_ogre_event);
				ogre_event_on = false;
			}
			else
			{
				addEvent(_ogre_event);
				ogre_event_on = true;
			}
			return true;

        default:
            return false;
    }

    return false;
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	OIS::KeyCode key = (OIS::KeyCode )(event.key);
	for( std::vector<TransformationEvent>::iterator iter = _trans_events.begin();
		iter != _trans_events.end(); ++iter )
	{
		iter->keyReleased(key);
	}

	for( std::vector<Event *>::iterator iter = _events.begin();
		iter != _events.end(); ++iter )
	{
		KeyTrigger trig(key, true);
		(*iter)->processTrigger(&trig);
	}

	return false;
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

