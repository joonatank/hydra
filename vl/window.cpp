/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
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

/// ----------------------------- Window -------------------------------------
/// ----------------------------- Public -------------------------------------
vl::Window::Window(vl::config::Window const &windowConf, vl::RendererInterface *parent)
	: _name(windowConf.name)
	, _renderer( parent )
	, _ogre_window(0)
	, _input_manager(0)
	, _keyboard(0)
	, _mouse(0)
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
		// We already have a window so it should be safe to check for stereo
		// quad buffer stereo
		if(hasStereo())
		{
			_create_channel(windowConf.get_channel(i), HS_LEFT, projection, windowConf.fsaa);
			_create_channel(windowConf.get_channel(i), HS_RIGHT, projection, windowConf.fsaa);
		}
		else if(windowConf.stereo_type == vl::config::ST_SIDE_BY_SIDE)
		{
			std::clog << "Using side by side stereo" << std::endl;
			config::Channel chan_cfg(windowConf.get_channel(i));
			chan_cfg.area.w /= 2;
			_create_channel(chan_cfg, HS_LEFT, projection, windowConf.fsaa);
			chan_cfg.area.x += chan_cfg.area.w;
			_create_channel(chan_cfg, HS_RIGHT, projection, windowConf.fsaa);
		}
		// no stereo
		else
		{
			_create_channel(windowConf.get_channel(i), HS_MONO, projection, windowConf.fsaa);
		}
	}

	std::clog << "Window::Window : done" << std::endl;
}

vl::Window::~Window( void )
{
	std::cout << vl::TRACE << "vl::Window::~Window" << std::endl;

	if( _input_manager )
	{
		std::cout << vl::TRACE << "Destroy OIS input manager." << std::endl;
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

vl::Player *
vl::Window::getPlayerPtr(void)
{
	return _renderer->getPlayerPtr();
}

vl::ogre::RootRefPtr
vl::Window::getOgreRoot( void )
{ return _renderer->getRoot(); }

void
vl::Window::setCamera(vl::CameraPtr camera)
{
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
	return stereo == GL_TRUE;
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
	_lazy_initialisation();

	// Updating FBOs and channel data is separated from drawing to screen
	// because we want to keep them separate for later extensions
	// mainly distributing FBOs, where these two needs to be separated.

	// Update FBOs
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		/// @todo setting player shouldn't be called each frame
		/// it should be implemented with either dirty data or callbacks
		_channels.at(i)->setPlayer(getPlayerPtr());
		_channels.at(i)->update();
	}

	// Draw
	assert(_ogre_window);

	_ogre_window->_beginUpdate();

	Ogre::RenderTarget::FrameStats stats;
	stats.lastFPS = 0;
	stats.avgFPS = 0;
	stats.bestFPS = 0;
	stats.worstFPS = 0;
	stats.bestFrameTime = 0;
	stats.worstFrameTime = 0;
	stats.triangleCount = 0;
	stats.batchCount = 0;

	stats.lastFPS = _ogre_window->getLastFPS();

	// Draw to screen
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		_channels.at(i)->draw();

		// GUI is not rendered as part of the FBO which might become problematic
		// instead it is overlayed on the Window.
		// For post screen effects and distribution of the Channels this shouldn't
		// be a problem, but we should tread carefully anyway.
		if(_renderer->getGui())
		{ _renderer->getGui()->update(); }

		// @todo Batch and triangle counts does not work at all. Always zero.
		stats.triangleCount += _channels.at(i)->getTriangleCount();
		stats.batchCount += _channels.at(i)->getBatchCount();
	}

	if(_tray_mgr)
	{
		_tray_mgr->update(stats);
		_tray_mgr->frameRenderingQueued(Ogre::FrameEvent());
	}

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
	if(winConf.stereo_type == vl::config::ST_QUAD_BUFFER)
	{
		params["stereo"] = vl::to_string(true);
	}
	params["vert_sync"] = vl::to_string(winConf.vert_sync);
	params["FSAA"] = vl::to_string(winConf.fsaa);

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

	if(winConf.stereo_type == vl::config::ST_QUAD_BUFFER)
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
	if(winConf.fsaa)
	{
		std::cout << "\n with FSAA " << winConf.fsaa;
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


vl::Channel *
vl::Window::_create_channel(vl::config::Channel const &chan_cfg, STEREO_EYE stereo_cfg,
			vl::config::Projection const &projection, uint32_t fsaa)
{
	// @todo replace with throwing because this is user controlled
	assert(!chan_cfg.name.empty());

	// Make a copy of channel config and rename it
	vl::config::Channel channel_config(chan_cfg);
	channel_config.name += ("_" + stereo_eye_to_string(stereo_cfg));

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

	/// We don't yet have a valid SceneManager
	/// So we need to wait till the camera is set here
	vl::config::Rect<double> const &rect = channel_config.area;
	assert(rect.valid());
	Ogre::Viewport *view = _ogre_window->addViewport(0, _channels.size(), rect.x, rect.y, rect.w, rect.h);

	bool use_fbo = false;
	if(_renderer_type == vl::config::Renderer::FBO)
	{ use_fbo = true; }

	Channel *channel = new Channel(channel_config, view, use_fbo, fsaa);
	_channels.push_back(channel);

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

	channel->setStereoEyeCfg(stereo_cfg);

	return channel;
}


void
vl::Window::_createInputHandling(void)
{
	std::cout << "Creating OIS Input system." << std::endl;

	assert( _ogre_window );

	std::ostringstream windowHndStr;
	uint64_t windowHnd = getHandle();
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
