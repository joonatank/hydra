#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

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
			_frame_data.setHeadPosition( _tracker->getPosition( 0 ) );
			_frame_data.setHeadOrientation( _tracker->getOrientation( 0 ) );
		}
	}

	// Init the transformation structures
	// TODO should be moved to Config::init or equivalent
	if( !_camera_trans.getSceneNode() )
	{
		SceneNode *node = new SceneNode( "CameraNode" );
		addSceneNode( node );
		_camera_trans.setSceneNode( node );
		if( !_camera_trans.getSceneNode() )
		{
			std::cerr << "No CameraNode found!" << std::endl;
			EQASSERT(false);
		}

		_camera_trans.setTransXKeys( OIS::KC_D, OIS::KC_A );
		_camera_trans.setTransYKeys( OIS::KC_PGUP, OIS::KC_PGDOWN );
		_camera_trans.setTransZKeys( OIS::KC_S, OIS::KC_W );

		// TODO camera needs yaw and pitch, but our current system does not allow
		// for yaw (forbidden in Channel) and they need to be in separate Nodes.
		_camera_trans.setRotYKeys( OIS::KC_RIGHT, OIS::KC_LEFT );
	}

	if( !_ogre_trans.getSceneNode() )
	{
		std::cerr << "Creating Ogre SceneNode" << std::endl;
		SceneNode *node = new SceneNode( "ogre" );
		addSceneNode( node );
		_ogre_trans.setSceneNode( node );
		if( !_ogre_trans.getSceneNode() )
		{
			std::cerr << "No OgreNode found!" << std::endl;
			EQASSERT(false);
		}

		// TODO break the Ogre Node to two SceneNodes and rotate each individually
		// to get a cleaner looking rotation (axises don't keep changing).
		_ogre_trans.setRotYKeys( OIS::KC_NUMPAD6, OIS::KC_NUMPAD4 );
		_ogre_trans.setRotZKeys( OIS::KC_NUMPAD8 , OIS::KC_NUMPAD5 );
	}


	// Really Move the objects
	_camera_trans();
	_ogre_trans();

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
	static clock_t last_time = 0;
	clock_t time = ::clock();

	OIS::KeyCode key = (OIS::KeyCode )(event.key);
	_camera_trans.keyPressed(key);
	_ogre_trans.keyPressed(key);
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
			break;

		case OIS::KC_R :
			// Reload the scene

			// We need to wait at least five secs before issuing the command again
			if( ( last_time - time )/CLOCKS_PER_SEC < 5 )
			{
				_frame_data.updateSceneVersion();
				last_time = time;
			}
			return true;

        case OIS::KC_F1:
            //_frameData.toggleHelp();
            return true;

        case OIS::KC_L :
        {
			/*
            if( !_currentCanvas )
                return true;

            _frameData.setCurrentViewID( EQ_ID_INVALID );

            uint32_t index = _currentCanvas->getActiveLayoutIndex() + 1;
            const eq::LayoutVector& layouts = _currentCanvas->getLayouts();
            EQASSERT( !layouts.empty( ))

            if( index >= layouts.size( ))
                index = 0;

            _currentCanvas->useLayout( index );
            
            const eq::Layout* layout = _currentCanvas->getLayouts()[index];
            std::ostringstream stream;
            stream << "Layout ";
            if( layout )
            {
                const std::string& name = layout->getName();
                if( name.empty( ))
                    stream << index;
                else
                    stream << name;
            }
            else
                stream << "NONE";
            
            stream << " active";
            _setMessage( stream.str( ));
			*/
            return true;
        }

        default:
            return false;
    }

    return false;
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	OIS::KeyCode key = (OIS::KeyCode )(event.key);
	_camera_trans.keyReleased(key);
	_ogre_trans.keyReleased(key);
    switch( key )
    {
		case OIS::KC_SPACE :
			break;
		default :
			return false;
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

