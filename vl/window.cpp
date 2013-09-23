/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file window.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
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
#include "input/mouse_event.hpp"

/// GUI
#include "gui/gui.hpp"
// Necessary for updating frame statistics
#include "gui/performance_overlay.hpp"
//Necessary for serializing stuff from namespace config
#include "distrib_settings.hpp"
// Parent
#include "pipe.hpp"
// Need Renderer on Slaves
#include "renderer.hpp"

// Necessary for checking for stereo support
// @todo should really be in the GLWindow implemetation
#include <GL/gl.h>

//#include <stdlib.h>

/// ----------------------------- Window -------------------------------------
/// ----------------------------- Public -------------------------------------
vl::Window::Window(vl::config::Window const &windowConf, vl::PipePtr parent)
	: _name(windowConf.name)
	, _window_config(windowConf)
	, _pipe(parent)
	, _pipe_id(vl::ID_UNDEFINED)
	, _renderer(0)
	, _ogre_window(0)
	, _input_manager(0)
	, _keyboard(0)
	, _mouse(0)
{
	assert(_pipe);

	_pipe->getSession()->registerObject(this, OBJ_WINDOW);

	std::cout << vl::TRACE << "vl::Window::Window : " << getName() << std::endl;

	if(windowConf.renderer.type == vl::config::Renderer::FBO)
	{
		std::cout << "EXPERIMENTAL : Forward Render to FBO." << std::endl;
	}
	else if(windowConf.renderer.type == vl::config::Renderer::DEFERRED)
	{
		std::cout << "EXPERIMENTAL : Deferred Renderer." << std::endl;
	}
	else if(windowConf.renderer.type == vl::config::Renderer::WINDOW)
	{
		std::cout << "Traditional Forward Renderer." << std::endl;
	}
}


vl::Window::Window(vl::RendererPtr renderer, uint64_t id)
	: _name()
	, _pipe(0)
	, _pipe_id(vl::ID_UNDEFINED)
	, _renderer(renderer)
	, _ogre_window(0)
	, _input_manager(0)
	, _keyboard(0)
	, _mouse(0)
{
	_renderer->getSession()->registerObject(this, id);
}

vl::Window::~Window( void )
{
	std::clog << "vl::Window::~Window" << std::endl;

	if( _input_manager )
	{
		std::clog << "Destroy OIS input manager." << std::endl;
		OIS::InputManager::destroyInputSystem(_input_manager);
		_input_manager = 0;
	}

	// @todo do we need this
	// We should test this (destroying a window), but we don't have a test
	// case for it.
	//_renderer->getRoot()->getNative()->detachRenderTarget(_ogre_window);

	// The render target can not be destroyed if we are still using the
	// context, this is a problem with Ogres GL system. 
	// Or more specifically GLSL.
	//getOgreRoot()->getNative()->destroyRenderTarget(_ogre_window);
	if(_ogre_window)
	{ _ogre_window->setHidden(true); }
}

void
vl::Window::setCamera(vl::CameraPtr camera)
{
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		_channels.at(i)->setCamera(camera);
	}
}

vl::Pipe *
vl::Window::getPipe(void)
{
	if(_pipe)
	{ return _pipe; }

	assert(_renderer);
	return _renderer->getPipe();
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

Ogre::RenderTarget::FrameStats 
vl::Window::getStatistics(void) const
{
	if(!_ogre_window)
	{ return Ogre::RenderTarget::FrameStats(); }

	/// @todo should we store the frame stats instead of calculating them here?

	/// Compine the statistics from Window 
	/// (all viewports that render directly to the window).
	/// And more specialised channels that render first to FBO.
	Ogre::RenderTarget::FrameStats stats = _ogre_window->getStatistics();

	/// Append statistics from other than window channels
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		// @todo Batch and triangle counts does not work at all. Always zero.
		stats.triangleCount += _channels.at(i)->getTriangleCount();
		stats.batchCount += _channels.at(i)->getBatchCount();
	}

	return stats;
}

void
vl::Window::resetStatistics(void)
{
	if(_ogre_window)
	{ _ogre_window->resetStatistics(); }

	for(size_t i = 0; i < _channels.size(); ++i)
	{
		_channels.at(i)->resetStatistics();
	}
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
		/*
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_MOVED );
		
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		
		vl::MouseEvent e = vl::convert_ois_to_hydra(evt);
		
		stream << e;
		_sendEvent( data );
		*/
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
		
		/*
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_PRESSED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		
		vl::MouseEvent e = vl::convert_ois_to_hydra(evt);
		
		stream << id << e;
		_sendEvent( data );
		*/
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
		
		// @warning added custom mouse event conversion so we can get camera projection for mouse picking.
		vl::MouseEvent e = vl::convert_ois_to_hydra(evt);
		vl::MouseEvent::BUTTON b_id = vl::MouseEvent::BUTTON(id);

		CameraPtr cam = _channels.at(0)->getCamera().getCamera();
		e.head_position = cam->getPosition();
		e.head_orientation = cam->getOrientation();

		//Should the ipd argument be -_ipd/2 or 0?
		e.view_projection = _channels.at(0)->getCamera().getFrustum().getProjectionMatrix();

		std::clog << " SENT EVENT: " << std::endl << e << std::endl;
		//Button ID säilytä se jatkossa, ei tarvi tehdä tsekkauksia myöhemmin!
		stream << b_id << e;
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
	// Updating FBOs and channel data is separated from drawing to screen
	// because we want to keep them separate for later extensions
	// mainly distributing FBOs, where these two needs to be separated.

	// Update FBOs
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		/// @todo setting player shouldn't be called each frame
		/// it should be implemented with either dirty data or callbacks
		_channels.at(i)->setPlayer(_renderer->getPlayer());
		_channels.at(i)->update();
	}

	// Draw
	assert(_ogre_window);

	_ogre_window->_beginUpdate();

	// Draw to screen
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		_channels.at(i)->draw();

		// GUI is not rendered as part of the FBO which might become problematic
		// instead it is overlayed on the Window.
		// For post screen effects and distribution of the Channels this shouldn't
		// be a problem, but we should tread carefully anyway.
		//
		// @todo
		// Do we want to overlay the GUI for all Channels
		// Probably not, but how do we decide it?
		// And should we bind the GUI to Channel?
		if(_renderer->getGui())
		{ _renderer->getGui()->update(); }
	}

	if(_renderer->getGui() && _renderer->getGui()->getPerformanceOverlay())
	{
		Ogre::RenderTarget::FrameStats stats = getStatistics();

		gui::PerformanceOverlayRefPtr overlay = _renderer->getGui()->getPerformanceOverlay();
		overlay->setFrameFrame(stats.avgFPS);
		overlay->setLastBatchCount(stats.batchCount);
		overlay->setLastTriangleCount(stats.triangleCount);
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

	for(std::vector<Channel *>::iterator iter = _channels.begin();
		iter != _channels.end(); ++iter)
	{
		Rect<double> size = (*iter)->getSize();
		vl::scalar aspect = w*size.w / (h*size.h);
		std::clog << "Resizing channel : " << (*iter)->getName()
			<< " size = " << size << " aspect ratio = " << aspect << std::endl;
		// @todo this is still rather hackish
		// we should update the Channel which should update the Camera etc.
		(*iter)->getCamera().getFrustum().setAspect(aspect);
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
	// Should only be called on slaves where we have Renderer
	assert(_renderer);
	_renderer->sendEvent(event);
}

void
vl::Window::serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
	if( DIRTY_NAME & dirtyBits )
	{
		msg << _name;
	}

	if( DIRTY_PIPE & dirtyBits )
	{
		assert(_pipe && _pipe->getID() != vl::ID_UNDEFINED);
		msg << _pipe->getID();
	}

	if( DIRTY_CONFIG & dirtyBits )
	{
		msg << _window_config;
	}

	if( DIRTY_CHANNELS & dirtyBits )
	{
	}
}

void
vl::Window::deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	// Using create variable because we need to create the Window only after
	// all the data has been gotten from the message.
	bool create = false;
	if( DIRTY_NAME & dirtyBits )
	{
		msg >> _name;
		// Name can't be changed so if it's dirty this Window was just created
		create = true;
	}

	if( DIRTY_PIPE & dirtyBits )
	{
		/// @todo should check that this is never reseted
		msg >> _pipe_id;
	}

	if( DIRTY_CONFIG & dirtyBits )
	{
		msg >> _window_config;
	}

	if( DIRTY_CHANNELS & dirtyBits )
	{
	}

	if(create)
	{
		assert(_renderer && _renderer->getPipe() 
			&& _renderer->getPipe()->getID() != vl::ID_UNDEFINED
			&& _pipe_id != vl::ID_UNDEFINED);

		if(_renderer->getPipe()->getID() != _pipe_id)
		{ return; }

		_createNative();

		// Needs to be after Channels and Ogre::Viewports are created
		// for the of change that window does not have Channel
		if(_channels.size() > 0)
		{
			_renderer->initialiseGUI(_channels.at(0));
		}
	}
}

void
vl::Window::_createNative(void)
{
	std::clog << "vl::Window::_createNative" << std::endl;

	_ogre_window = _createOgreWindow(_window_config);

	_createInputHandling();

	/// @todo Channel creation should be in Renderer and we should 
	/// attach channels to Windows. It's more logical.
	/// This will also allow dynamic channel creation and destruction.
	vl::config::Projection const &projection = _window_config.renderer.projection;
	for(size_t i = 0; i < _window_config.get_n_channels(); ++i)
	{
		config::Channel channel_config = _window_config.get_channel(i);

		if(channel_config.wall.empty())
		{ std::cout << vl::TRACE << "No wall for channel " << channel_config.name << std::endl; }

		vl::Wall const &wall = channel_config.wall;
		config::Renderer::Type renderer_type = _window_config.renderer.type;

		RENDER_MODE rend_mode(RM_WINDOW);
		if(renderer_type == vl::config::Renderer::FBO)
		{ rend_mode = RM_FBO; }
		else if(renderer_type == vl::config::Renderer::DEFERRED)
		{ rend_mode = RM_DEFERRED; }
	
		if(_window_config.stereo_type == vl::config::ST_OCULUS)
		{ rend_mode = RM_OCULUS; }

		// We already have a window so it should be safe to check for stereo
		// quad buffer stereo
		if(hasStereo())
		{
			_create_channel(channel_config, HS_LEFT, projection, 
				rend_mode, _window_config.fsaa);
			_create_channel(channel_config, HS_RIGHT, projection, 
				rend_mode, _window_config.fsaa);
		}
		// For now lets try if side-by-side stereo is good enough for Oculus
		else if(_window_config.stereo_type == vl::config::ST_SIDE_BY_SIDE
			|| _window_config.stereo_type == vl::config::ST_OCULUS)
		{
			std::clog << "Using side by side stereo" << std::endl;
			channel_config.area.w /= 2;
			_create_channel(channel_config, HS_LEFT, projection, 
				rend_mode, _window_config.fsaa);
			channel_config.area.x += channel_config.area.w;
			_create_channel(channel_config, HS_RIGHT, projection, 
				rend_mode, _window_config.fsaa);
		}
		// no stereo
		else
		{
			_create_channel(channel_config, HS_MONO, projection, 
				rend_mode, _window_config.fsaa);
		}
	}

	// We need to set the aspect ratio for the cameras
	// @todo not the proper place to set them though
	// they should be set in either Channel creation 
	// when Camera is set to Channel
	int left, top;
	uint32_t width, height, depth;
	_ogre_window->getMetrics(width, height, depth, left, top);
	resize(width, height);

	std::clog << "Window::_createNative : done" << std::endl;
}

Ogre::RenderWindow *
vl::Window::_createOgreWindow(vl::config::Window const &winConf)
{
	Ogre::NameValuePairList params;

	assert( !winConf.empty() );
	// Should only be called on Slaves where we have Renderer available.
	assert(_renderer);

	params["left"] = vl::to_string(winConf.area.x);
	params["top"] = vl::to_string(winConf.area.y);
	params["border"] = "none";
	params["gamma"] = vl::to_string(winConf.renderer.hardware_gamma);
	// Default to Quad buffer stereo
	if(winConf.stereo_type == vl::config::ST_QUAD_BUFFER
		|| winConf.stereo_type == vl::config::ST_DEFAULT)
	{
		params["stereo"] = vl::to_string(true);
	}
	params["vert_sync"] = vl::to_string(winConf.vert_sync);
	params["FSAA"] = vl::to_string(winConf.fsaa);

	std::cout << vl::TRACE << "Creating Ogre RenderWindow : " 
		<< "left = " << winConf.area.x << " top = " << winConf.area.y
		<< " width = " << winConf.area.w << " height = " << winConf.area.h;

	/// @todo replace with proper printing of Window options
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

	Ogre::RenderWindow *win = _renderer->getRoot()->createWindow( "Hydra-"+getName(),
			winConf.area.w, winConf.area.h, params );
	win->setAutoUpdated(false);
	
	// If this is the first window we initialise resources here
	// this is kinda wonky but since we need a RenderWindow before initialising
	// initialise resources
	// needs to be after Window creation because this needs OpenGL context (meh)
	// @todo should not call this multiple times.
	// actually we might move this to Ogre Window creation
	_renderer->getRoot()->loadResources();

	return win;
}


vl::Channel *
vl::Window::_create_channel(vl::config::Channel const &chan_cfg, STEREO_EYE stereo_cfg,
			vl::config::Projection const &projection,
			RENDER_MODE render_mode, uint32_t fsaa)
{
	// @todo replace with throwing because this is user controlled
	assert(!chan_cfg.name.empty());

	Wall const &wall = chan_cfg.wall;

	// Make a copy of channel config and rename it
	vl::config::Channel channel_config(chan_cfg);
	channel_config.name += ("_" + stereo_eye_to_string(stereo_cfg));

	/// We don't yet have a valid SceneManager
	/// So we need to wait till the camera is set here
	vl::Rect<double> const &rect = channel_config.area;
	assert(rect.valid());
	Ogre::Viewport *view = _ogre_window->addViewport(0, _channels.size(), rect.x, rect.y, rect.w, rect.h);

	Channel *channel = new Channel(channel_config, view, render_mode, fsaa, this);
	_channels.push_back(channel);

	/// Set frustum

	channel->getCamera().getFrustum().enableAsymmetricStereoFrustum(projection.use_asymmetric_stereo);
	/// @todo these can be removed when we have checked that this config is working
	if(channel->getCamera().getFrustum().isAsymmetricStereoFrustum())
	{
		std::clog << "EXPERIMENTAL : Using asymmetric stereo frustum." << std::endl;
	}
	else
	{
		std::clog << "NOT Using asymmetric stereo frustum." << std::endl;
	}

	channel->getCamera().getFrustum().setWall(wall);
	channel->getCamera().getFrustum().setFov(Ogre::Degree(projection.fov));
	if(projection.perspective_type == vl::config::Projection::FOV)
	{
		std::clog << "Setting channel " << channel->getName() << " to use FOV frustum." << std::endl;
		channel->getCamera().getFrustum().setType(Frustum::FOV);
	}
	else if(projection.perspective_type == vl::config::Projection::USER)
	{
		std::clog << "Setting channel " << channel->getName() << " to use USER frustum." << std::endl;
		channel->getCamera().getFrustum().setType(Frustum::USER);
	}
	else
	{
		std::clog << "Setting channel " << channel->getName() << " to use Wall frustum." << std::endl;
		channel->getCamera().getFrustum().setType(Frustum::WALL);
	}

	// set the aspect ratio
	// @todo this is still rather hackish
	// we should update the Channel which should update the Camera etc.
	//
	// channel size is in homogenous coordinates so we need the window size
	// to calculate the aspect ratio
	Rect<double> size = channel->getSize();
	vl::scalar aspect = _window_config.area.w*size.w / (_window_config.area.h*size.h);
	channel->getCamera().getFrustum().setAspect(aspect);

	// @todo what does this do and why?
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
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ std::cout << "\n\tDevice: " << " Vendor: " << i->second; }
	std::cout << std::endl;
}
