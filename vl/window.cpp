/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file window.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */


// Interface
#include "window.hpp"

#include "base/exceptions.hpp"

#include "camera.hpp"
// Necessary for calculating projection matrix
#include "math/frustum.hpp"

#include "ogre_root.hpp"
// Necessary for copying ipd
#include "player.hpp"

#include "input/joystick_event.hpp"
#include "input/ois_converters.hpp"

/// GUI
#include "gui/gui.hpp"
// Necessary for glDrawBuffer
#include <GL/gl.h>

#include <stdlib.h>

/// -------------------- StereoRenderTargetListener --------------------------
void
vl::StereoRenderTargetListener::postRenderTargetUpdate(Ogre::RenderTargetEvent const &evt)
{
}

void
vl::StereoRenderTargetListener::preViewportUpdate(Ogre::RenderTargetViewportEvent const &evt)
{
	std::clog << "vl::StereoRenderTargetListener::preViewportUpdate" << std::endl;
	if(stereo)
	{
		std::clog << "Switching eye" << std::endl;
		// No need to check the Draw buffer because we always have either
		// GL_BACK_RIGHT or GL_BACK here.
		if(_left)
		{ glDrawBuffer(GL_BACK_LEFT); }
		else
		{ glDrawBuffer(GL_BACK_RIGHT); }
		_left = !_left;
	}
	else
	{
		GLint draw_mode;
		glGetIntegerv(GL_DRAW_BUFFER, &draw_mode);
		// We need to test for the mode here
		// because setting draw buffer to the previous value will
		// raise OpenGL INVALID_OPERATION error
		// Here we can either have GL_BACK or GL_BACK_RIGHT and
		// for stereo rendering -> mono rendering we need to change the buffer
		//
		// This seems to cause the error under some circumstances so enable it
		// only after testing with a stereo enabled system.
		//if(draw_mode != GL_BACK)
		//{ glDrawBuffer(GL_BACK); }
	}
}

void
vl::StereoRenderTargetListener::postViewportUpdate(Ogre::RenderTargetViewportEvent const &evt)
{
}

void
vl::StereoRenderTargetListener::preRenderTargetUpdate(Ogre::RenderTargetEvent const &evt)
{
}



/// -------------------------- StereoCamera ----------------------------------
vl::StereoCamera::StereoCamera(void)
	: _camera(0)
	, _ogre_camera(0)
{}

vl::StereoCamera::~StereoCamera(void)
{
}

void
vl::StereoCamera::setHead(vl::Transform const &head)
{
	_head = head;
}

void
vl::StereoCamera::setCamera(vl::CameraPtr cam)
{
	_camera = cam;
	if(cam)
	{
		assert(_camera->getNative());
		_ogre_camera = (Ogre::Camera *)_camera->getNative();
	}
	else
	{ _ogre_camera = 0; }
}


void
vl::StereoCamera::update(vl::scalar eye_x)
{
	assert(_camera);
	assert(_ogre_camera);

	_frustum.setClipping(_camera->getNearClipDistance(), _camera->getFarClipDistance());

	Ogre::Quaternion wallRot = orientation_to_wall(_frustum.getWall());

	Ogre::Vector3 cam_pos = _camera->getPosition();
	Ogre::Quaternion cam_quat = _camera->getOrientation();

	Ogre::Vector3 eye(eye_x, 0, 0);

	Ogre::Matrix4 projMat = _frustum.getProjectionMatrix(eye_x);
	_ogre_camera->setCustomProjectionMatrix(true, projMat);

	// Combine eye and camera positions
	// Needs to be rotated with head for correct stereo
	// Do not rotate with wall will cause incorrect view for the side walls
	//
	// cam*head*eye
	Ogre::Vector3 eye_d = cam_quat*(_head.quaternion*eye + _head.position) + cam_pos;

	// Combine camera and wall orientation to get the projection on correct wall
	// Seems like the wallRotation needs to be inverse for this one, otherwise
	// left and right wall are switched.
	Ogre::Quaternion eye_orientation = cam_quat*wallRot.Inverse();

	_ogre_camera->setPosition(eye_d);
	_ogre_camera->setOrientation(eye_orientation);
}


/// ----------------------------- Window -------------------------------------
/// ----------------------------- Public -------------------------------------
vl::Window::Window(vl::config::Window const &windowConf, vl::RendererInterface *parent)
	: _name(windowConf.name)
	, _renderer( parent )
	, _ogre_window(0)
	, _input_manager(0)
	, _keyboard(0)
	, _mouse(0)
	, _window_listener(0)
	, _tray_mgr(0)
{
	assert( _renderer );

	std::cout << vl::TRACE << "vl::Window::Window : " << getName() << std::endl;

	_ogre_window = _createOgreWindow(windowConf);
	if(windowConf.input_handler)
	{ _createInputHandling(); }

	if(windowConf.renderer.projection.use_asymmetric_stereo)
	{
		std::cout << "EXPERIMENTAL : Using asymmetric stereo frustum." << std::endl;
	}

	_renderer_type = windowConf.renderer.type;

	if(_renderer_type == vl::config::Renderer::FBO)
	{
		std::cout << "EXPERIMENTAL : Should Render to FBO." << std::endl;
	}

	vl::config::Projection const &projection = windowConf.renderer.projection;
	for(size_t i = 0; i < windowConf.get_n_channels(); ++i)
	{
		_create_channel(windowConf.get_channel(i), projection);
	}
	std::clog << "All channel created." << std::endl;

	// Set listener
	// @todo add the stereo attribute
	_window_listener = new StereoRenderTargetListener(false);
	_ogre_window->addListener(_window_listener);

	std::clog << "Window::Window : done" << std::endl;
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
	// The render target can not be destroyed if we are still using the
	// context, this is a problem with Ogres GL system. 
	// Or more specifically GLSL.
	//getOgreRoot()->getNative()->destroyRenderTarget(_ogre_window);
	_ogre_window->setHidden(true);
}

vl::config::EnvSettingsRefPtr
vl::Window::getEnvironment(void) const
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
	// @todo does not allow removing the camera, should it?
//	if(!camera)
//	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	for(size_t i = 0; i < _channels.size(); ++i)
	{
		_channels.at(i)->setCamera(camera);
	}
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

Ogre::RenderTarget::FrameStats const &
vl::Window::getStatistics(void) const
{
	if(_ogre_window)
	{ return _ogre_window->getStatistics(); }

	return Ogre::RenderTarget::FrameStats();
}

void
vl::Window::resetStatistics(void)
{
	if(_ogre_window)
	{ _ogre_window->resetStatistics(); }
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
			case OIS::KC_LMENU :
				break;
			case OIS::KC_RMENU :
				break;
			default :
				_renderer->getGui()->injectKeyDown(key);
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
				_renderer->getGui()->injectKeyUp(key);
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
		_renderer->getGui()->injectMouseEvent(evt);
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
		_renderer->getGui()->injectMouseEvent(evt);
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
		_renderer->getGui()->injectMouseEvent(evt);
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
	JoystickEvent e = vl::convert_ois_to_hydra(evt);
	stream << button << e;
	_sendEvent(data);

	return true;
}

bool 
vl::Window::buttonReleased(OIS::JoyStickEvent const &evt, int button)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_RELEASED);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	JoystickEvent e = vl::convert_ois_to_hydra(evt);
	stream << button << e;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::axisMoved(OIS::JoyStickEvent const &evt, int axis)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_AXIS);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	JoystickEvent e = vl::convert_ois_to_hydra(evt);
	stream << axis << e;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::povMoved(OIS::JoyStickEvent const &evt, int pov)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_POV);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	JoystickEvent e = vl::convert_ois_to_hydra(evt);
	stream << pov << e;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::vector3Moved(OIS::JoyStickEvent const &evt, int index)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_VECTOR3);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	JoystickEvent e = vl::convert_ois_to_hydra(evt);
	stream << index << e;
	_sendEvent( data );

	return true;
}

/// ----------------------------- Public methods -------------------------------
void
vl::Window::draw(void)
{
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		_channels.at(i)->update(getPlayer());
	}

	_lazy_initialisation();

	// if stereo is not enabled ipd should be zero 
	// TODO should it be forced though?
	vl::scalar ipd = getPlayer().getIPD();
	std::vector<vl::scalar> eyes;
	// Left is first, so it's negative
	if(ipd != 0)
	{
		eyes.push_back(-ipd/2);
		eyes.push_back(ipd/2);
		_window_listener->stereo = true;
	}
	else
	{
		eyes.push_back(0);
		_window_listener->stereo = false;
	}

	// Draw
	assert(_ogre_window);
	_ogre_window->_beginUpdate();

	for(size_t i = 0; i < _channels.size(); ++i)
	{
		assert(eyes.size() > 0);
		{ _channels.at(i)->draw(eyes.at(0), true); }
		if(eyes.size() > 1)
		{ _channels.at(i)->draw(eyes.at(1), false); }

		// Keeps track of the batches and triangles
		_ogre_window->_updateViewport(_channels.at(i)->viewport, true);

		// @todo test with stereo setup if this really renders the gui for
		// both left and right eye
		if(_renderer->getGui())
		{ _renderer->getGui()->update(); }
	}

	if(_tray_mgr)
	{ _tray_mgr->frameRenderingQueued(Ogre::FrameEvent()); }

	_ogre_window->_endUpdate();
}


void
vl::Window::swap( void )
{
	assert(_ogre_window);
	_ogre_window->swapBuffers(false);
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

void
vl::Window::resize(int w, int h)
{
	// works with both Ogre and external windows
	_ogre_window->resize(w, h);
	_ogre_window->windowMovedOrResized();
	// @todo does not handle correctly multiple channels
	if(_channels.size() > 0)
	{
		_channels.at(0)->camera.getFrustum().setAspect(vl::scalar(w)/vl::scalar(h));
	}
}

uint64_t
vl::Window::getHandle(void) const
{
	uint64_t ogreWinId = 0x0;
	_ogre_window->getCustomAttribute( "WINDOW", &ogreWinId );

	return ogreWinId;
}

/// ------------------------------- Protected ----------------------------------
void
vl::Window::_sendEvent( vl::cluster::EventData const &event )
{
	_renderer->sendEvent(event);
}

Ogre::RenderWindow *
vl::Window::_createOgreWindow(vl::config::Window const &winConf)
{
	Ogre::NameValuePairList params;

	assert( !winConf.empty() );

	params["left"] = vl::to_string(winConf.rect.x);
	params["top"] = vl::to_string(winConf.rect.y);
	params["border"] = "none";
	params["gamma"] = vl::to_string(winConf.renderer.hardware_gamma);
	params["stereo"] = vl::to_string(winConf.stereo);
	params["vert_sync"] = vl::to_string(winConf.vert_sync);

	std::cout << vl::TRACE << "Creating Ogre RenderWindow : " 
		<< "left = " << winConf.rect.x << " top = " << winConf.rect.y
		<< " width = " << winConf.rect.w << " height = " << winConf.rect.h;

	// @todo we should overload print operator for window config
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

	if(winConf.stereo)
	{
		std::cout << "\n with quad buffer stereo";
	}
	if(winConf.renderer.hardware_gamma)
	{
		std::cout << "\n with hardware gamma correction";
	}
	if(winConf.vert_sync)
	{
		std::cout << "\n with vertical sync";
	}
	
	std::cout << std::endl;

	// Add user defined params
	for(NamedParamList::const_iterator iter = winConf.params.begin();
		iter != winConf.params.end(); ++iter)
	{
		std::clog << "Adding user param : " << iter->first << " with value : " << iter->second << std::endl;
		params[iter->first] = iter->second;
	}
	
	Ogre::RenderWindow *win = getOgreRoot()->createWindow( "Hydra-"+getName(),
			winConf.rect.w, winConf.rect.h, params );
	win->setAutoUpdated(false);
	
	return win;
}


void
vl::Window::_create_channel(vl::config::Channel const &channel_config,
			vl::config::Projection const &projection)
{
	assert(!channel_config.name.empty());
	std::clog << "Creating channel : " << channel_config.name << std::endl;

	std::cout << vl::TRACE << "Finding Wall for channel : " << channel_config.name << std::endl;
	Wall wall = getEnvironment()->findWall(channel_config.wall_name);

	// Get the first wall definition if no named one was found
	if(wall.empty() && getEnvironment()->getWalls().size() > 0)
	{
		wall = getEnvironment()->getWall(0);
		std::cout << vl::TRACE << "No wall found : using the first one " << wall.name << std::endl;
	}
	else
	{
		std::cout << vl::TRACE << "Wall " << wall.name << " found." << std::endl;
	}

	// TODO this should be configurable
	Ogre::ColourValue background_col = Ogre::ColourValue(1.0, 0.0, 0.0, 0.0);
	/// We don't yet have a valid SceneManager
	/// So we need to wait till the camera is set here
	vl::config::Rect<double> const &rect = channel_config.area;
	assert(rect.valid());
	Ogre::Viewport *view = _ogre_window->addViewport(0, _channels.size(), rect.x, rect.y, rect.w, rect.h);

	// @todo these should be moved to channel
	view->setBackgroundColour(background_col);
	view->setAutoUpdated(false);

	bool use_fbo = false;
	if(_renderer_type == vl::config::Renderer::FBO)
	{ use_fbo = true; }

	Channel *channel = new Channel(channel_config, view, use_fbo);
	_channels.push_back(channel);

	std::clog << "Channel created." << std::endl;

	/// Set frustum
	channel->camera.getFrustum().setWall(wall);
	channel->camera.getFrustum().setFov(Ogre::Degree(projection.fov));
	if(projection.perspective_type == vl::config::Projection::FOV)
	{
		std::clog << "Setting channel " << channel->getName() << " to use FOV frustum." << std::endl;
		channel->camera.getFrustum().setType(Frustum::FOV);
	}
	else
	{
		std::clog << "Setting channel " << channel->getName() << " to use Wall frustum." << std::endl;
		channel->camera.getFrustum().setType(Frustum::WALL);
	}
}


void
vl::Window::_createInputHandling(void)
{
	std::cout << "Creating OIS Input system." << std::endl;

	assert( _ogre_window );

	std::ostringstream windowHndStr;
	uint64_t windowHnd = getHandle();
	//size_t windowHnd = 0;
	//_ogre_window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;

	OIS::ParamList pl;
	pl.insert( std::make_pair(std::string("WINDOW"), windowHndStr.str()) );
	// Some extra parameters to avoid getting stuck with the window
	// TODO should move these to the configuration file
	pl.insert( std::make_pair(std::string("x11_keyboard_grab"), std::string("false") ) );
	pl.insert( std::make_pair(std::string("x11_mouse_grab"), std::string("false") ) );

	// Info
	std::cout << vl::TRACE << "Creating OIS Input Manager" << std::endl;

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

		std::cout << "Creating joystick " << i+1
			<< "\n\t" << "Axes: " << stick->getNumberOfComponents(OIS::OIS_Axis)
			<< "\n\t" << "Sliders: " << stick->getNumberOfComponents(OIS::OIS_Slider)
			<< "\n\t" << "POV/HATs: " << stick->getNumberOfComponents(OIS::OIS_POV)
			<< "\n\t" << "Buttons: " << stick->getNumberOfComponents(OIS::OIS_Button)
			<< "\n\t" << "Vector3: " << stick->getNumberOfComponents(OIS::OIS_Vector3)
				<< std::endl;

		stick->setEventCallback(this);
		_joysticks.push_back(stick);	
	}

	std::cout << vl::TRACE << "Input system created." << std::endl;
}

void
vl::Window::_printInputInformation(void)
{
	// Print debugging information
	unsigned int v = _input_manager->getVersionNumber();

	std::cout << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
		<< "\nRelease Name: " << _input_manager->getVersionName()
		<< "\nManager: " << _input_manager->inputSystemName()
		<< "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
		<< "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
		<< "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick)
		<< std::endl;

	// List all devices
	// TODO should go to Ogre Log file
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ std::cout << "\n\tDevice: " << " Vendor: " << i->second; }
	std::cout << std::endl;
}

void
vl::Window::_lazy_initialisation(void)
{
	if(!_tray_mgr && _renderer->isDebugOverlayEnabled())
	{
		std::string name = "InterfaceName/" + _name;
		assert(_ogre_window && _mouse);
		Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
		Ogre::FontManager::getSingleton().getByName("SdkTrays/Value")->load();
		_tray_mgr = new OgreBites::SdkTrayManager(name, _ogre_window, _mouse);
		_tray_mgr->showFrameStats(OgreBites::TL_BOTTOMRIGHT);
		_tray_mgr->hideCursor();
	}
}
