#include "config.hpp"

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
	if( _init_data.getID() == EQ_ID_INVALID )
	{
		EQCHECK( mapObject( &_init_data, initDataID ));
        unmapObject( &_init_data ); // data was retrieved, unmap immediately
	}
    else  // appNode, _initData is registered already
    {
        EQASSERT( _init_data.getID() == initDataID )
	}
}

void
eqOgre::Config::unmapData( void )
{
	// Nothing to do as init data is only mapped once and then unmapped
}

bool
eqOgre::Config::init( uint32_t const )
{
	// Register data
	registerObject( &_frame_data );

	_init_data.setFrameDataID( _frame_data.getID() );
	registerObject( &_init_data );
    
	if( !eq::Config::init( _init_data.getID() ) )
    { return false; }

    return true;
}

uint32_t 
eqOgre::Config::startFrame (const uint32_t frameID)
{
	uint32_t version;
	if( _frame_data.isDirty() )
	{ version = _frame_data.commit(); }
	else
	{ version = _frame_data.getVersion(); }

	return eq::Config::startFrame( version );
}

char const *CB_INFO_TEXT = "Config : OIS event received : ";

// TODO the event handling should be separated to multiple functions
// handleKeyPress, handleKeyRelease, handlePointerMotion etc.
bool
eqOgre::Config::handleEvent( const eq::ConfigEvent* event )
{
	Ogre::Quaternion camera_rot = _frame_data.getCameraRotation();
	Ogre::Quaternion qx;
	Ogre::Quaternion qy;
	Ogre::Real scale = 0.01;

	OIS::KeyCode key;
	bool redraw = false;
	switch( event->data.type )
	{
		case eq::Event::KEY_PRESS :
			key = (OIS::KeyCode )(event->data.key.key);
			if(  key == OIS::KC_ESCAPE || key == OIS::KC_Q )
			{
				std::cerr << CB_INFO_TEXT << "Escape or Q pressed. Will quit now. " << std::endl;
				// TODO should quit cleanly
				abort();
			}
			else if( key == OIS::KC_W )
			{
				std::cerr << CB_INFO_TEXT << "W pressed. " << std::endl;
			}
			else if( key == OIS::KC_SPACE )
			{
				std::cerr << CB_INFO_TEXT << "Space pressed. " << std::endl;
			}
			else
			{
				std::cerr << CB_INFO_TEXT << "Key = " << key << " pressed." << std::endl;
			}
			redraw = true;
			break;

		case eq::Event::KEY_RELEASE :
			key = (OIS::KeyCode )(event->data.key.key);
			if( key == OIS::KC_ESCAPE || key == OIS::KC_Q )
			{
			}
			else if( key == OIS::KC_W )
			{
				std::cerr << CB_INFO_TEXT << "W released. " << std::endl;
			}
			else if( key== OIS::KC_SPACE )
			{
				std::cerr << CB_INFO_TEXT << "Space released." << std::endl;
			}
			else
			{
				std::cerr << CB_INFO_TEXT << "Key = " << key << " released." << std::endl;
			}

			redraw = true;
			break;

		case eq::Event::POINTER_BUTTON_PRESS:
//			std::cerr << "Config received mouse button press event. Button = "
//				<< event->data.pointer.button << std::endl;
			break;

		case eq::Event::POINTER_BUTTON_RELEASE:
//			std::cerr << "Config received mouse button release event. Button = "
//				<< event->data.pointer.button << std::endl;
			break;
		case eq::Event::POINTER_MOTION:
//			std::cerr << "Config received mouse motion event. Coords = "
//				<< event->data.pointer.x << " " << event->data.pointer.y << std::endl;
/*	TODO needs dx and dy added to pointer motion event
			qy = Ogre::Quaternion( Ogre::Radian( scale * event->data.pointerMotion.dx ), Ogre::Vector3::UNIT_Y );
			//qx = Ogre::Quaternion( Ogre::Radian( scale * event->data.pointerMotion.dy ), Ogre::Vector3::UNIT_X );
			qx = Ogre::Quaternion::IDENTITY;
			camera_rot = camera_rot * qx * qy;
			_frame_data.setCameraRotation( camera_rot );
*/
			break;
	}
	return redraw;
}

bool
eqOgre::Config::_handleKeyEvent( const eq::KeyEvent& event )
{
	Ogre::Vector3 cam_pos = _frame_data.getCameraPosition();
	Ogre::Vector3 ogre_pos = _frame_data.getOgrePosition();
	Ogre::Quaternion ogre_rot = _frame_data.getOgreRotation();

	Ogre::Real scale = 0.1;

	// Used for toggle events
	static clock_t last_time = 0;
	clock_t time = ::clock();

    switch( event.key )
    {
		// Move Camera
		// front
		case 'w':
        case 'W':
			cam_pos += scale*(-Ogre::Vector3::UNIT_Z);
			_frame_data.setCameraPosition( cam_pos );
			return true;

		// back
		case 's':
		case 'S':
			cam_pos += scale*(Ogre::Vector3::UNIT_Z);
			_frame_data.setCameraPosition( cam_pos );
			return true;

		// left
		case 'a':
		case 'A':
			cam_pos += scale*(-Ogre::Vector3::UNIT_X);
			_frame_data.setCameraPosition( cam_pos );
			return true;

		// right
		case 'd':
		case 'D':
			cam_pos += scale*(Ogre::Vector3::UNIT_X);
			_frame_data.setCameraPosition( cam_pos );
			return true;

		case eq::KC_PAGE_UP :
			cam_pos += scale*(Ogre::Vector3::UNIT_Y);
			_frame_data.setCameraPosition( cam_pos );
			return true;

		case eq::KC_PAGE_DOWN :
			cam_pos += scale*(-Ogre::Vector3::UNIT_Y);
			_frame_data.setCameraPosition( cam_pos );
			return true;

		// Rotate Ogre
		// front
		case eq::KC_UP :
			ogre_rot = ogre_rot * Ogre::Quaternion( Ogre::Radian( scale), Ogre::Vector3::UNIT_Z );
			_frame_data.setOgreRotation( ogre_rot );
			return true;

		// back
		case eq::KC_DOWN :
			ogre_rot = ogre_rot * Ogre::Quaternion( Ogre::Radian( -scale), Ogre::Vector3::UNIT_Z );
			_frame_data.setOgreRotation( ogre_rot );
			return true;

		// left
		case eq::KC_LEFT :
			ogre_rot = ogre_rot * Ogre::Quaternion( Ogre::Radian( scale), Ogre::Vector3::UNIT_Y );
			_frame_data.setOgreRotation( ogre_rot );
			return true;

		// right
		case eq::KC_RIGHT :
			ogre_rot = ogre_rot * Ogre::Quaternion( Ogre::Radian( -scale), Ogre::Vector3::UNIT_Y );
			_frame_data.setOgreRotation( ogre_rot );
			return true;

		case 'r':
		case 'R':
			// Reload the scene

			// We need to wait five secs
			if( ( last_time - time )/CLOCKS_PER_SEC < 5 )
			{
				_frame_data.updateSceneVersion();
				last_time = time;
			}
			return true;

        case eq::KC_F1:
        case 'h':
        case 'H':
            //_frameData.toggleHelp();
            return true;

        case 'l':
        case 'L':
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
}
