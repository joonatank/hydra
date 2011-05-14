/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file window.hpp
 */

#include "window.hpp"

#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"

#include "renderer.hpp"

#include "camera.hpp"

#include <OGRE/OgreLogManager.h>

/// GUI
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/elements/CEGUIFrameWindow.h>
#include <CEGUI/elements/CEGUIEditbox.h>
#include <CEGUI/elements/CEGUIMultiColumnList.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>

#include <GL/gl.h>

namespace
{
	/// @todo Missing conversions for the extra buttons in OIS
	/// they could be mapped to other buttons in CEGUI if necessary
	CEGUI::MouseButton OISButtonToGUI( OIS::MouseButtonID button )
	{
		switch( button )
		{
			case OIS::MB_Left :
				return CEGUI::LeftButton;
			case OIS::MB_Right :
				return CEGUI::RightButton;
			case OIS::MB_Middle :
				return CEGUI::MiddleButton;
			case OIS::MB_Button3 :
				return CEGUI::X1Button;
			case OIS::MB_Button4 :
				return CEGUI::X2Button;
			default :
				return CEGUI::NoButton;
		}
	}
}

/// ----------------------------- Public ---------------------------------------
vl::Window::Window( std::string const &name, vl::RendererInterface *parent )
	: _name(name)
	, _renderer( parent )
	, _ogre_window(0)
	, _camera(0)
	, _left_viewport(0)
	, _right_viewport(0)
	, _input_manager(0)
	, _keyboard(0)
	, _mouse(0)
{
	assert( _renderer );

	std::string msg = "vl::Window::Window : " + getName();
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);

	vl::EnvSettings::Window winConf = _renderer->getWindowConf( getName() );

	_ogre_window = _createOgreWindow(winConf);
	_createInputHandling();

	// If the channel has a name we try to find matching wall

	if( !winConf.channel.name.empty() )
	{
		msg = "Finding Wall for channel : " + getName();
		Ogre::LogManager::getSingleton().logMessage(msg);
		_wall = getEnvironment()->findWall( winConf.channel.wall_name );
	}

	// Get the first wall definition if no named one was found
	if( _wall.empty() && getEnvironment()->getWalls().size() > 0 )
	{
		_wall = getEnvironment()->getWall(0);
		msg = "No wall found : using the first one " + _wall.name;
	}
	else
	{
		msg = "Wall " + _wall.name + " found.";
	}
	Ogre::LogManager::getSingleton().logMessage(msg);

	// TODO this should be configurable
	Ogre::ColourValue background_col = Ogre::ColourValue(1.0, 0.0, 0.0, 0.0);

	Ogre::Camera *og_cam = 0;
	if( _camera )
	{ og_cam = (Ogre::Camera *)_camera->getNative(); }

	_left_viewport = _ogre_window->addViewport(og_cam);	
	_left_viewport->setBackgroundColour(background_col);
	// This is necessary because we are using single camera and single viewport
	// to draw to both backbuffers when using stereo.
	_left_viewport->setAutoUpdated(false);

	if(hasStereo())
	{
		_right_viewport = _ogre_window->addViewport(og_cam, 1);
		_right_viewport->setBackgroundColour(background_col);
		_right_viewport->setAutoUpdated(false);
	}
}

vl::Window::~Window( void )
{
	std::string msg("vl::Window::~Window");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);

	msg = "Cleaning out OIS";
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	if( _input_manager )
	{
		msg = "Destroy OIS input manager.";
		Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
        OIS::InputManager::destroyInputSystem(_input_manager);
		_input_manager = 0;
	}

	getOgreRoot()->getNative()->detachRenderTarget(_ogre_window);
}

vl::EnvSettingsRefPtr
vl::Window::getEnvironment( void )
{ return _renderer->getEnvironment(); }

vl::Player const &
vl::Window::getPlayer( void ) const
{ return _renderer->getPlayer(); }

vl::ogre::RootRefPtr
vl::Window::getOgreRoot( void )
{ return _renderer->getRoot(); }

void
vl::Window::setCamera(vl::CameraPtr camera)
{
	if( !camera )
	{ return; }

	_camera = camera;
	
	Ogre::Camera *og_cam = (Ogre::Camera *)_camera->getNative();
	if(_left_viewport)
	{ _left_viewport->setCamera(og_cam); }
	if(_right_viewport)
	{ _right_viewport->setCamera(og_cam); }
}

void
vl::Window::takeScreenshot( const std::string& prefix, const std::string& suffix )
{
	std::string real_suffix;
	if( getName().empty() )
	{ real_suffix = suffix; }
	else
	{ real_suffix = "-" + getName()+ suffix; }

	_ogre_window->writeContentsToTimestampedFile(prefix, real_suffix);
}

bool 
vl::Window::hasStereo(void) const
{
	GLboolean stereo;
	glGetBooleanv( GL_STEREO, &stereo );
	return stereo;
}

/// ------------------------ Public OIS Callbacks ------------------------------
bool
vl::Window::keyPressed( OIS::KeyEvent const &key )
{
	// Check if we have a GUI that uses user input
	if( _renderer->guiShown() )
	{
		switch( key.key )
		{
			// Keys that go to the Application always, used to control the GUI
			// TODO should be configurable from python script
			// and should match the one used there
			case OIS::KC_ESCAPE :
				break;
			case OIS::KC_GRAVE :
				break;
			case OIS::KC_F2 :
				break;
			case OIS::KC_F10 :
				break;
			default :
				// TODO this is missing auto-repeat for text
				CEGUI::System::getSingleton().injectKeyDown(key.key);
				CEGUI::System::getSingleton().injectChar(key.text);
				return true;
		}
	}

	vl::cluster::EventData data( vl::cluster::EVT_KEY_PRESSED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << key;
	_sendEvent( data );

	return true;
}

bool
vl::Window::keyReleased( OIS::KeyEvent const &key )
{
	if( _renderer->guiShown() )
	{
		// TODO should check if GUI window is active
		// TODO this might need translation for the key codes
		switch( key.key )
		{
			// Keys that go to the Application always, used to control the GUI
			// TODO should be configurable from python script
			// and should match the one used there
			case OIS::KC_ESCAPE :
				break;
			case OIS::KC_GRAVE :
				break;
			case OIS::KC_F2 :
				break;
			case OIS::KC_F10 :
				break;
			default :
				CEGUI::System::getSingleton().injectKeyUp(key.key);
// 				CEGUI::System::getSingleton().injectChar(key.text);
				return true;
		}
	}

	vl::cluster::EventData data( vl::cluster::EVT_KEY_RELEASED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << key;
	_sendEvent( data );

	return true;
}

bool
vl::Window::mouseMoved( OIS::MouseEvent const &evt )
{
	if( _renderer->guiShown() )
	{
		// TODO should check if GUI window is active
		CEGUI::System::getSingleton().injectMousePosition(evt.state.X.abs, evt.state.Y.abs);
		// NOTE this has a problem of possibly consuming some mouse movements
		// Assuming that mouse is moved and the wheel is changed at the same time
		// CEGUI might consume either one of them and the other one will not be
		// passed on.
		// This is so rare that we don't care about it.
		if( evt.state.Z.rel != 0 )
		{
			// Dirty hack for scaling the wheel movement
			float z = (float)(evt.state.Z.rel)/100;
			CEGUI::System::getSingleton().injectMouseWheelChange(z);
		}
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_MOVED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		stream << evt;
		_sendEvent( data );
	}

	return true;
}

bool
vl::Window::mousePressed( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	if( _renderer->guiShown() )
	{
		CEGUI::System::getSingleton().injectMouseButtonDown( OISButtonToGUI(id) );
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_PRESSED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		stream << id << evt;
		_sendEvent( data );
	}

	return true;
}

bool
vl::Window::mouseReleased( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	if( _renderer->guiShown() )
	{
		CEGUI::System::getSingleton().injectMouseButtonUp( OISButtonToGUI(id) );
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_RELEASED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		stream << id << evt;
		_sendEvent( data );
	}

	return true;
}

bool 
vl::Window::buttonPressed(OIS::JoyStickEvent const &evt, int button)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_PRESSED);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << button << evt;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::buttonReleased(OIS::JoyStickEvent const &evt, int button)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_RELEASED);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << button << evt;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::axisMoved(OIS::JoyStickEvent const &evt, int axis)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_AXIS);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << axis << evt;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::povMoved(OIS::JoyStickEvent const &evt, int pov)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_POV);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << pov << evt;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::vector3Moved(OIS::JoyStickEvent const &evt, int index)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_VECTOR3);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	stream << index << evt;
	_sendEvent( data );

	return true;
}

/// ----------------------------- Public methods -------------------------------
void
vl::Window::draw(void)
{
	if(!_camera)
	{ return; }

	assert(_left_viewport);

	Ogre::Real c_near = _camera->getNearClipDistance();
	Ogre::Real c_far = _camera->getFarClipDistance();

	Ogre::Camera *og_cam = (Ogre::Camera *)_camera->getNative();

	// if stereo is not enabled ipd should be zero 
	// TODO should it be forced though?
	if(!hasStereo())
	{ _ipd = 0; }

	// @todo is there performance problems with stereo?
	// If they are still present we need to add a separate camera also
	// @todo rendering GUI for both eyes

	vl::Transform const &head = getPlayer().getHeadTransform();

	Ogre::Matrix4 projMat = calculate_projection_matrix(c_near, c_far, _wall, head.position);
	og_cam->setCustomProjectionMatrix( true, projMat );

	Ogre::Quaternion wallRot = orientation_to_wall(_wall);

	// Should not be rotated with wall, all the walls would be out of sync with each other
	Ogre::Vector3 headTrans = head.position;

	Ogre::Vector3 cam_pos = og_cam->getPosition();
	Ogre::Quaternion cam_quat = og_cam->getOrientation();

	/// @todo should really be replaced with a stereo camera setup
	
	/// Use tuples to eliminate code copying
	typedef boost::tuple<Ogre::Viewport *, double, GLenum> view_tuple;
	std::vector<view_tuple> views;
	if(hasStereo() && _left_viewport && _right_viewport)
	{
		views.push_back( view_tuple(_left_viewport, -_ipd/2, GL_BACK_LEFT) );
		views.push_back( view_tuple(_right_viewport, _ipd/2, GL_BACK_RIGHT) );
	}
	else if(_left_viewport)
	{
		views.push_back( view_tuple(_left_viewport, 0, GL_BACK) );
	}

	// Left viewport
	for(size_t i = 0; i < views.size(); ++i)
	{
		view_tuple const &view = views.at(i);
		Ogre::Vector3 eye(view.get<1>(), 0, 0);
		glDrawBuffer(view.get<2>());

		// Combine eye and camera positions
		// Needs to be rotated with head for correct stereo
		// Do not rotate with wall will cause incorrect view for the side walls
		Ogre::Vector3 eye_d = (head.quaternion*cam_quat)*eye 
			+ cam_quat*headTrans + cam_pos;

		// Combine camera and wall orientation to get the projection on correct wall
		// Seems like the wallRotation needs to be inverse for this one, otherwise
		// left and right wall are switched.
		Ogre::Quaternion eye_orientation = cam_quat*wallRot.Inverse();

		og_cam->setPosition(eye_d);
		og_cam->setOrientation(eye_orientation);

		view.get<0>()->update();
	}

	// Push back the original position and orientation
	og_cam->setPosition(_camera->getPosition());
	og_cam->setOrientation(_camera->getOrientation());

	assert(_ogre_window);
	_ogre_window->update(false);
}

void
vl::Window::swap( void )
{
	assert(_ogre_window);
	_ogre_window->swapBuffers();
}

void
vl::Window::capture( void )
{
	if( _keyboard )
	{ _keyboard->capture(); }

	if( _mouse )
	{ _mouse->capture(); }

	for( size_t i = 0; i < _joysticks.size(); ++i )
	{
		_joysticks.at(i)->capture();
	}
}

/// ------------------------------- Protected ----------------------------------
void
vl::Window::_sendEvent( vl::cluster::EventData const &event )
{
	_renderer->sendEvent(event);
}

Ogre::RenderWindow *
vl::Window::_createOgreWindow( vl::EnvSettings::Window const &winConf )
{
	Ogre::NameValuePairList params;

	assert( !winConf.empty() );

	params["left"] = vl::to_string( winConf.x );
	params["top"] = vl::to_string( winConf.y );
	params["border"] = "none";

	std::cout << vl::TRACE << "Creating Ogre RenderWindow : " 
		<< "left = " << winConf.x << " top = " << winConf.y
		<< " width = " << winConf.w << " height = " << winConf.h;

	if( winConf.stereo )
	{
		std::cout << " with stereo : ";
		params["stereo"] = "true";
	}
	if( winConf.nv_swap_sync )
	{
		std::cout << "with NV swap sync, group ";
		params["nvSwapSync"] = "true";
		std::stringstream ss;
		ss << winConf.nv_swap_group;
		std::cout << ss.str() << ", barrier ";
		params["swapGroup"] = ss.str();
		ss.str("");
		ss << winConf.nv_swap_barrier;
		std::cout << ss.str() << " : ";
		params["swapBarrier"] = ss.str();
	}

	if( winConf.vert_sync )
	{
		std::cout << "with vertical sync";
		params["vert_sync"] = "true";
	}
	std::cout << std::endl;

	Ogre::RenderWindow *win = getOgreRoot()->createWindow( "Hydra-"+getName(), winConf.w, winConf.h, params );
	win->setAutoUpdated(false);
	
	return win;
}


void
vl::Window::_createInputHandling( void )
{
	std::string message( "Creating OIS Input system." );
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	assert( _ogre_window );

	std::ostringstream windowHndStr;
	size_t windowHnd = 0;
	_ogre_window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;

	OIS::ParamList pl;
	pl.insert( std::make_pair(std::string("WINDOW"), windowHndStr.str()) );
	// Some extra parameters to avoid getting stuck with the window
	// TODO should move these to the configuration file
	pl.insert( std::make_pair(std::string("x11_keyboard_grab"), std::string("false") ) );
	pl.insert( std::make_pair(std::string("x11_mouse_grab"), std::string("false") ) );

	// Info
	message = "Creating OIS Input Manager";
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	_input_manager = OIS::InputManager::createInputSystem( pl );

	_printInputInformation();

	_keyboard = static_cast<OIS::Keyboard*>(_input_manager->createInputObject(OIS::OISKeyboard, true));
	_keyboard->setEventCallback(this);

	_mouse = static_cast<OIS::Mouse*>(_input_manager->createInputObject(OIS::OISMouse, true));

	_mouse ->getMouseState().height = _ogre_window->getHeight();
	_mouse ->getMouseState().width	= _ogre_window->getWidth();

	_mouse->setEventCallback(this);

	int numSticks = _input_manager->getNumberOfDevices(OIS::OISJoyStick);
	for( int i = 0; i < numSticks; ++i )
	{
		OIS::JoyStick *stick = static_cast<OIS::JoyStick *>(
			_input_manager->createInputObject(OIS::OISJoyStick, true) );

		std::cout << vl::TRACE << "Creating joystick " << i+1
			<< "\n\t" << "Axes: " << stick->getNumberOfComponents(OIS::OIS_Axis)
			<< "\n\t" << "Sliders: " << stick->getNumberOfComponents(OIS::OIS_Slider)
			<< "\n\t" << "POV/HATs: " << stick->getNumberOfComponents(OIS::OIS_POV)
			<< "\n\t" << "Buttons: " << stick->getNumberOfComponents(OIS::OIS_Button)
			<< "\n\t" << "Vector3: " << stick->getNumberOfComponents(OIS::OIS_Vector3)
				<< std::endl;

		stick->setEventCallback(this);
		_joysticks.push_back(stick);	
	}

	message = "Input system created.";
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);
}

void
vl::Window::_printInputInformation( void )
{
	// Print debugging information
	// TODO debug information should go to Ogre Log file
	unsigned int v = _input_manager->getVersionNumber();
	std::stringstream ss;
	ss << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
		<< "\nRelease Name: " << _input_manager->getVersionName()
		<< "\nManager: " << _input_manager->inputSystemName()
		<< "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
		<< "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
		<< "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick)
		<< '\n';
	std::cout << ss.str() << std::endl;

	ss.str("");
	// List all devices
	// TODO should go to Ogre Log file
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ ss << "\n\tDevice: " << " Vendor: " << i->second; }
	std::cout << ss.str() << std::endl;
}
