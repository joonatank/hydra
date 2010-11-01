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
eqOgre::Config::startFrame (const uint32_t frameID)
{
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

	// Process Events
	// TODO test if the speed calculations work really using high and low fps
	static clock_t last_time = 0;
	clock_t time = ::clock();

	// Secs since last frame
	double t = ((double)( time - last_time ))/CLOCKS_PER_SEC;
	last_time = time;
	
	// angular speed in radians/s : 60 deg/s
	Ogre::Real rot_speed = 60*M_PI/180.0;

	// Speed in m/s
	Ogre::Real speed = 1;

	// Update the frame data if the Ogre or Camera is moving
	if( _camera_move_dir != Ogre::Vector3::ZERO )
	{
		SceneNode &camera = _frame_data.getCameraNode();
		Ogre::Vector3 cam_pos = camera.getPosition();
		cam_pos += speed*t*_camera_move_dir.normalisedCopy();
		camera.setPosition( cam_pos );
	}
	if( _camera_rot_dir != Ogre::Quaternion::IDENTITY )
	{
		SceneNode &camera = _frame_data.getCameraNode();
		Ogre::Quaternion cam_rot = camera.getOrientation();

		// TODO camera rotation is relative to the original position
		Ogre::Quaternion q = Ogre::Quaternion::nlerp( rot_speed*t, cam_rot, cam_rot*_camera_rot_dir);

		camera.setOrientation( q );

		_camera_rot_dir = Ogre::Quaternion::IDENTITY;
	}
	if( _ogre_rot_dir != Ogre::Quaternion::IDENTITY )
	{
		// TODO this function makes the ogre sometimes continue the rotation
		SceneNode &ogre = _frame_data.getOgreNode();
		Ogre::Quaternion ogre_rot = ogre.getOrientation();

		// This one is funny :)
		//Ogre::Quaternion q = Ogre::Quaternion::nlerp( rot_speed, ogre_rot, _ogre_rot_dir);
		// This one is correct
		Ogre::Quaternion q = Ogre::Quaternion::nlerp( rot_speed*t, ogre_rot, ogre_rot*_ogre_rot_dir);

		ogre.setOrientation( q );
	}

	uint32_t version = _frame_data.commitAll();

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

		// Move Camera
		// front
		case OIS::KC_W :
			_camera_move_dir += -Ogre::Vector3::UNIT_Z;
			return true;

		// back
		case OIS::KC_S :
			_camera_move_dir += Ogre::Vector3::UNIT_Z;
			return true;

		// left
		case OIS::KC_A :
			_camera_move_dir += -Ogre::Vector3::UNIT_X;
			return true;

		// right
		case OIS::KC_D :
			_camera_move_dir += Ogre::Vector3::UNIT_X;
			return true;

		case OIS::KC_PGUP :
			_camera_move_dir += Ogre::Vector3::UNIT_Y;
			return true;

		case OIS::KC_PGDOWN :
			_camera_move_dir += -Ogre::Vector3::UNIT_Y;
			return true;

		// Rotate Ogre
		// front
		case OIS::KC_UP :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(90) ), Ogre::Vector3::UNIT_Z );
			return true;

		// back
		case OIS::KC_DOWN :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(-90) ), Ogre::Vector3::UNIT_Z );
			return true;

		// left
		case OIS::KC_LEFT :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(90) ), Ogre::Vector3::UNIT_Y );
			return true;

		// right
		case OIS::KC_RIGHT :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(-90) ), Ogre::Vector3::UNIT_Y );
			return true;

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
    switch( key )
    {
		case OIS::KC_SPACE :
			break;

		// Move Camera
		// front
		case OIS::KC_W :
			_camera_move_dir -= -Ogre::Vector3::UNIT_Z;
			return true;

		// back
		case OIS::KC_S :
			_camera_move_dir -= Ogre::Vector3::UNIT_Z;
			return true;

		// left
		case OIS::KC_A :
			_camera_move_dir -= -Ogre::Vector3::UNIT_X;
			return true;

		// right
		case OIS::KC_D :
			_camera_move_dir -= Ogre::Vector3::UNIT_X;
			return true;

		case OIS::KC_PGUP :
			_camera_move_dir -= Ogre::Vector3::UNIT_Y;
			return true;

		case OIS::KC_PGDOWN :
			_camera_move_dir -= -Ogre::Vector3::UNIT_Y;
			return true;

		// Rotate Ogre
		// front
		case OIS::KC_UP :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(-90) ), Ogre::Vector3::UNIT_Z );
			return true;

		// back
		case OIS::KC_DOWN :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(90) ), Ogre::Vector3::UNIT_Z );
			return true;

		// left
		case OIS::KC_LEFT :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(-90) ), Ogre::Vector3::UNIT_Y );
			return true;

		// right
		case OIS::KC_RIGHT :
			_ogre_rot_dir = _ogre_rot_dir * Ogre::Quaternion( Ogre::Radian( Ogre::Degree(90) ), Ogre::Vector3::UNIT_Y );
			return true;

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
	Ogre::Quaternion qy = Ogre::Quaternion::IDENTITY;
	if( event.dx < 0 )
		qy = Ogre::Quaternion( Ogre::Radian( Ogre::Degree(90) ), Ogre::Vector3::UNIT_Y );
	else
	{
		qy = Ogre::Quaternion( Ogre::Radian( Ogre::Degree(-90) ), Ogre::Vector3::UNIT_Y );
	}
	_camera_rot_dir = qy;

	return true;
}

bool
eqOgre::Config::_handleJoystickEvent(const eq::MagellanEvent& event)
{
	return false;
}

