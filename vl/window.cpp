/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
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

// For Oculus hacks
#include "channel.hpp"
#include "player.hpp"
#include <gl/GL.h>

/// @todo move
Ogre::Quaternion convert(ovrQuatf const &q)
{ return Ogre::Quaternion(q.w, q.x, q.y, q.z); }

Ogre::Vector3 convert(OVR::Vector3f const &v)
{ return Ogre::Vector3(v.x, v.y, v.z); }

Ogre::Matrix4 convert(OVR::Matrix4f const &m)
{
	Ogre::Matrix4 mm;

	for(size_t i = 0; i < 4; ++i)
	{
		for(size_t j = 0; j < 4; ++j)
		{ mm[i][j] = m.M[i][j]; }
	}

	return mm;
}

vl::Transform convert(ovrPosef const &p)
{
	vl::Transform t;
	t.position = Ogre::Vector3(p.Position.x, p.Position.y, p.Position.z);
	t.quaternion = convert(p.Orientation);

	return t;
}

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
	, _hmd(0)
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
	, _hmd(0)
{
	_renderer->getSession()->registerObject(this, id);
}

vl::Window::~Window( void )
{
	std::clog << "vl::Window::~Window" << std::endl;

	_destroy_oculus();

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
	//if( _renderer->guiShown() )
	//{
		_renderer->getGui()->injectMouseEvent(evt);
	//}
	//else
	//{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_MOVED );
		
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		
		vl::MouseEvent e = vl::convert_ois_to_hydra(evt);
		
		stream << e;
		_sendEvent( data );
	//}

	return true;
}

bool
vl::Window::mousePressed( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	if( _renderer->guiShown() )
	{
		//_renderer->getGui()->injectMouseEvent(evt);
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_PRESSED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		
		vl::MouseEvent e = vl::convert_ois_to_hydra(evt);
		vl::MouseEvent::BUTTON b_id = vl::MouseEvent::BUTTON(id);
				
		//Button ID säilytä se jatkossa, ei tarvi tehdä tsekkauksia myöhemmin!
		stream << b_id << e;
		_sendEvent( data );
		
	}

	return true;
}

bool
vl::Window::mouseReleased( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	if( _renderer->guiShown() )
	{
		//_renderer->getGui()->injectMouseEvent(evt);
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_RELEASED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteDataStream stream = data.getStream();
		
		// @warning added custom mouse event conversion so we can get camera projection for mouse picking.
		vl::MouseEvent e = vl::convert_ois_to_hydra(evt);
		vl::MouseEvent::BUTTON b_id = vl::MouseEvent::BUTTON(id);
				
		//Button ID säilytä se jatkossa, ei tarvi tehdä tsekkauksia myöhemmin!
		stream << b_id << e;
		_sendEvent( data );
	}

	return true;
}

bool 
vl::Window::buttonPressed(OIS::JoyStickEvent const &evt, int index)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_PRESSED);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	vl::JoystickEvent e = vl::convert_ois_to_hydra(evt);
	e.type = vl::JoystickEvent::BUTTON_PRESSED;
	stream << index << e;
	_sendEvent(data);

	return true;
}

bool 
vl::Window::buttonReleased(OIS::JoyStickEvent const &evt, int index)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_RELEASED);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	vl::JoystickEvent e = vl::convert_ois_to_hydra(evt);
	e.type = vl::JoystickEvent::BUTTON_RELEASED;
	stream << index << e;
	_sendEvent( data );

	return true;
}

bool 
vl::Window::axisMoved(OIS::JoyStickEvent const &evt, int index)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_AXIS);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	vl::JoystickEvent e = vl::convert_ois_to_hydra(evt);
	e.type = vl::JoystickEvent::AXIS;
	stream << index << e;
	_sendEvent( data );
	return true;
}

bool 
vl::Window::povMoved(OIS::JoyStickEvent const &evt, int index)
{
	/* @todo there is no POV data in the event
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_POV);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	vl::JoystickEvent e = vl::convert_ois_to_hydra(evt);
	e.type = vl::JoystickEvent::POV;
	stream << index << e;
	_sendEvent( data );
	*/

	return true;
}

bool 
vl::Window::vector3Moved(OIS::JoyStickEvent const &evt, int index)
{
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_VECTOR3);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	vl::JoystickEvent e = vl::convert_ois_to_hydra(evt);
	e.type = vl::JoystickEvent::VECTOR;
	stream << index << e;
	_sendEvent( data );

	return true;
}


bool 
vl::Window::sliderMoved(OIS::JoyStickEvent const &evt, int index)
{
	/* @todo there is no Slider in the Joystick event
	vl::cluster::EventData data(vl::cluster::EVT_JOYSTICK_SLIDER);
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteDataStream stream = data.getStream();
	vl::JoystickEvent e = vl::convert_ois_to_hydra(evt);
	e.type = vl::JoystickEvent::SLIDER;
	stream << index << e;
	_sendEvent( data );
	*/

	return true;
}


/// ----------------------------- Public methods -------------------------------
void
vl::Window::draw(void)
{
	// for now just assert since this should never happen
	assert(_channels.size() > 0);

	// Dirty hacking to get the Oculus rendering working
	bool hmd = false;
	if(_channels.at(0)->getRenderMode() == RM_OCULUS)
	{ hmd = true; }

	if(hmd)
	{ _begin_frame_oculus(); }

	// Updating FBOs and channel data is separated from drawing to screen
	// because we want to keep them separate for later extensions
	// mainly distributing FBOs, where these two needs to be separated.

	// Update FBOs
	for(size_t i = 0; i < _channels.size(); ++i)
	{ _channels.at(i)->update(); }

	// Draw
	assert(_ogre_window);

	_ogre_window->_beginUpdate();

	// Draw to screen
	//
	// doesn't have any effect on the HMD weather this be disabled or not, odd
	if(!_hmd)
	{
		for(size_t i = 0; i < _channels.size(); ++i)
		{ _channels.at(i)->draw(); }
	}

	// GUI is not rendered as part of the FBO which might become problematic
	// instead it is overlayed on the Window.
	// For post screen effects and distribution of the Channels this shouldn't
	// be a problem, but we should tread carefully anyway.
	//
	// why isn't it part of the FBO?
	// oh because it doesn't work with Deferred Rendering?
	// or it might get distorted with Oculus?
	//
	// @todo
	// Do we want to overlay the GUI for all Channels
	// Probably not, but how do we decide it?
	// And should we bind the GUI to Channel?
	//
	// @todo this doesn't do what I thought it would
	// it has nothing to do with Rendering it just updates the variables
	// for Rendering, so it probably should be first in the Rendering loop
	// where is the GUI rendered?
	//
	// For Oculus Rift
	// GUI doesn't work, it has nothing to do with this though
	// we need to figure out where the GUI is drawn and what exactly does update do.
	if(_renderer->getGui())
	{ _renderer->getGui()->update(); }

	// Update the performance overlay
	// @todo should be somewhere else, like a separte function in Renderer
	// Also problematic since we can have multiple windows.
	if(_renderer->getGui() && _renderer->getGui()->getPerformanceOverlay())
	{
		Ogre::RenderTarget::FrameStats stats = getStatistics();

		gui::PerformanceOverlayRefPtr overlay = _renderer->getGui()->getPerformanceOverlay();
		overlay->setFrameFrame(stats.avgFPS);
		overlay->setLastBatchCount(stats.batchCount);
		overlay->setLastTriangleCount(stats.triangleCount);
	}

	if(hmd)
	{ _end_frame_oculus(); }

	_ogre_window->_endUpdate();
}


void
vl::Window::swap( void )
{
	// Swapping buffers here doesn't make a difference with Oculus
	// we shouldn't call it but if it makes no difference no reason to waste time
	// with branching.
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

vl::Rect<int>
vl::Window::getArea() const
{
	int left, top;
	uint32_t width, height, depth;
	_ogre_window->getMetrics(width, height, depth, left, top);

	return Rect<int>(left, top, width, height);
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
vl::Window::_create_channels(void)
{
	/// @todo Channel creation should be in Renderer and we should 
	/// attach channels to Windows. It's more logical.
	/// This will also allow dynamic channel creation and destruction.
	for(size_t i = 0; i < _window_config.get_n_channels(); ++i)
	{
		config::Channel channel_config = _window_config.get_channel(i);

		config::Renderer::Type renderer_type = _window_config.renderer.type;

		RENDER_MODE rend_mode(RM_WINDOW);
		if(renderer_type == vl::config::Renderer::FBO)
		{ rend_mode = RM_FBO; }
		else if(renderer_type == vl::config::Renderer::DEFERRED)
		{ rend_mode = RM_DEFERRED; }
	
		if(_window_config.stereo_type == vl::config::ST_OCULUS)
		{ rend_mode = RM_OCULUS; }

		// For now lets try if side-by-side stereo is good enough for Oculus
		if(_window_config.stereo_type == vl::config::ST_SIDE_BY_SIDE
			|| _window_config.stereo_type == vl::config::ST_OCULUS)
		{
			std::clog << "Using side by side stereo" << std::endl;

			// @todo
			// calculate the proper channel size instead of using the config
			// for side by side stereo it should never be anything other than half of the
			// window anyway

			std::clog << "Oculus channel texture size = (" 
				<< channel_config.texture_size.x << ", "
				<< channel_config.texture_size.y << ")"
				<< std::endl;

			channel_config.area.w /= 2;

			// left channel
			std::string base_name = channel_config.name;
			channel_config.name = base_name + "_left";
			_create_channel(channel_config, HS_LEFT, rend_mode, _window_config.fsaa);
			
			// right channel
			channel_config.name = base_name + "_right";
			channel_config.area.x += channel_config.area.w;
			_create_channel(channel_config, HS_RIGHT, rend_mode, _window_config.fsaa);

		}
		// We already have a window so it should be safe to check for stereo
		// quad buffer stereo
		else if(hasStereo())
		{
			std::string base_name = channel_config.name;
			channel_config.name = base_name + "_left";
			_create_channel(channel_config, HS_LEFT, rend_mode, _window_config.fsaa);
			channel_config.name = base_name + "_right";
			_create_channel(channel_config, HS_RIGHT, rend_mode, _window_config.fsaa);
		}
		// no stereo
		else
		{
			_create_channel(channel_config, HS_MONO, rend_mode, _window_config.fsaa);
		}
	}
}

void
vl::Window::_createNative(void)
{
	std::clog << "vl::Window::_createNative" << std::endl;

	if(_window_config.type == config::WT_OCULUS)
	{
		int w, h;
		_initialise_oculus();
		w = _hmd->Resolution.w;
		h = _hmd->Resolution.h;

		_ogre_window = _createOgreWindow(_window_config);
		HWND window;
		_ogre_window->getCustomAttribute("WINDOW", &window);

		_configure_oculus(_window_config);
		
		// for Oculus direct rendering
		bool res = ovrHmd_AttachToWindow(_hmd, window, NULL, NULL);
		assert(res);
	}
	else
	{
		_ogre_window = _createOgreWindow(_window_config);
	}

	assert(_ogre_window);

	_createInputHandling();

	_create_channels();

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
			RENDER_MODE render_mode, uint32_t fsaa)
{
	/// We don't yet have a valid SceneManager
	/// So we need to wait till the camera is set here
	vl::Rect<double> const &rect = chan_cfg.area;
	assert(rect.valid());
	Ogre::Viewport *view = _ogre_window->addViewport(0, _channels.size(), rect.x, rect.y, rect.w, rect.h);

	Channel *channel = new Channel(chan_cfg, view, render_mode, fsaa, stereo_cfg, this);
	_channels.push_back(channel);

	// set the aspect ratio
	// @todo this is still rather hackish
	// we should update the Channel which should update the Camera etc.
	//
	// channel size is in homogenous coordinates so we need the window size
	// to calculate the aspect ratio
	//
	// also uses the config size not the real window size
	// also the aspect ratio is recalculated after all the channels have been created
	Rect<double> size = channel->getSize();
	vl::scalar aspect = _window_config.area.w*size.w / (_window_config.area.h*size.h);
	channel->getCamera().getFrustum().setAspect(aspect);

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

void
vl::Window::_initialise_oculus(void)
{
	std::clog << "vl::Window::_initialise_oculus" << std::endl;
	assert(!_hmd);
	ovr_Initialize();
	_hmd = ovrHmd_Create(0);
	// when Oculus is not connected for debugging
	if(!_hmd)
	{
		_hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
	}

	assert(_hmd);
}

void
vl::Window::_configure_oculus(config::Window &window_cfg)
{
	std::clog << "vl::Window::_configure_oculus" << std::endl;
	assert(_hmd);

	// @todo config stuff

	// Dirty hack to get Native window ID and DC
	// Window is the window handle
	// DC is the OpenGL context
	HWND window;
	assert(_ogre_window);
	_ogre_window->getCustomAttribute("WINDOW", &window);
	// cant use getCustomAttribute for the Context because it doesn't expose the HDC
	// Not sure if we are setting the right context but it is working
	HDC dc = wglGetCurrentDC();

	ovrGLConfig cfg;
	cfg.OGL.Header.API			= ovrRenderAPI_OpenGL;
	// RT size? render texture size is it
	// then we should have halved resolution since we are using double RTs
	// interesting if we halve it it halves the output
	cfg.OGL.Header.RTSize		= _hmd->Resolution;
	// for now zero
	cfg.OGL.Header.Multisample	= _ogre_window->getFSAA();
	// Don't need to set window it uses current one automatically
	cfg.OGL.Window				= window;
	// don't need to set context it uses new one automatically
	// actually we get no picture if we don't set it
	// yea no picture at all, not even the Warning overlay on either Oculus or Monitor
	cfg.OGL.DC					= dc;

	// distortionCaps is distortion flags
	// need flip because the FBO output is upside down
	// @todo add ovrDistortionCap_SRGB for gamma correction
	unsigned int distortionCaps = ovrDistortionCap_FlipInput;
	
	ovrFovPort eyeFov[2];
	eyeFov[0] = _hmd->DefaultEyeFov[0];
	eyeFov[1] = _hmd->DefaultEyeFov[1];
	ovrBool result = ovrHmd_ConfigureRendering(_hmd, &cfg.Config, 
		distortionCaps, eyeFov, _eye_render_desc);
	assert(result);

	// Render texture size, needs to be stored and used for creating the FBO channel

	// Configure Stereo settings.
	OVR::Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(_hmd, ovrEye_Left, 
		eyeFov[0], 1.0f);
	OVR::Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(_hmd, ovrEye_Right, 
		eyeFov[1], 1.0f);

	OVR::Sizei renderTargetSize;
	renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
	renderTargetSize.h = max ( recommenedTex0Size.h, recommenedTex1Size.h );

	/// Seems like the target is much larger than the window/physical device (2364, 1461)
	/// would need to update the Channel info with that
	/// but it should just cause distortions in the texture not completely corrupt it
	/// and there is no guarantie that the Ogre viewport would end up large enough anyway
	std::cout << "Recommended FBO size : left = "
		<< "(" << recommenedTex0Size.w << ", " << recommenedTex0Size.h << ")"
		<< " right = "
		<< "(" << recommenedTex1Size.w << ", " << recommenedTex1Size.h << ")" << "\n"
		<< " target size = "
		<< "(" << renderTargetSize.w << ", " << renderTargetSize.h << ")"
		<< std::endl;

	// @fixme this assumes the left and right eye textures are the same size
	// we have only tested single channel, I'd assume multiple wouldn't work properly
	assert(window_cfg.get_n_channels() == 1);
	window_cfg.get_channel(0).texture_size = vl::vec2i(renderTargetSize.w/2, renderTargetSize.h);

	/// Create the texture, well we use different method but just for reference
	//pRendertargetTexture = pRender->CreateTexture(
	//		Texture_RGBA | Texture_RenderTarget | eyeRenderMultisample,
	//		renderTargetSize.w, renderTargetSize.h, NULL);

	// The actual RT size may be different due to HW limits.
	//renderTargetSize.w = pRendertargetTexture->GetWidth();
	//renderTargetSize.h = pRendertargetTexture->GetHeight();

	/// Oculus head tracker (cyro)
	// Start the sensor which provides the Rift’s pose and motion.
	// ovrTrackingCap_Position, not using the position tracking for now
	// @todo how do we check that this is successful?
	bool res = ovrHmd_ConfigureTracking(_hmd, ovrTrackingCap_Orientation | 
		ovrTrackingCap_MagYawCorrection, 0);
	if(!res)
	{
		const char *last_err = ovrHmd_GetLastError(_hmd);
		std::clog << "Oculus ERROR : " << last_err << std::endl;
		assert(false);
	}
}

void
vl::Window::_destroy_oculus()
{
	if(!_hmd)
	{ return; }

	std::clog << "vl::Window::_destroy_oculus" << std::endl;

	// Remove OVR devices
	ovrHmd_Destroy(_hmd);

	ovr_Shutdown();
}

void
vl::Window::_begin_frame_oculus()
{
	assert(_hmd);
	// @todo move camera based on the head orientation 
	// (since we are not using the normal tracking)

	// we should use Eye render order for better performance
	// but our channel system does not like it

	ovrFrameTiming hmdFrameTiming = ovrHmd_BeginFrame(_hmd, 0);

	// Just to check since we shouldn't have other channels than Left and Right
	// for this window
	assert(_channels.size() == 2);

	// Health and Safety Warning display state.
	ovrHSWDisplayState hswDisplayState;
	ovrHmd_GetHSWDisplayState(_hmd, &hswDisplayState);
	
	// Dismiss the Health and Safety Warning
	if (hswDisplayState.Displayed)
	{
		ovrHmd_DismissHSWDisplay(_hmd);
	}

	// We can't use the "_renderer" member variable since it's actaully a slave session 
	// not the renderer... we should rename it
	// it does not exist on the master
	assert(getPipe() && getPipe()->getRenderer());
	Player *player = getPipe()->getRenderer()->getPlayer();
	assert(player);

	// We want the position info from head tracker.
	// We also want the camera transformation.
	Transform head_t = player->getHeadTransform();
	Vector3 camera_pos = player->getCamera()->getWorldPosition();
	Quaternion camera_q = player->getCamera()->getWorldOrientation();

	// Get the orientation from Oculus
	// Query the HMD for the current tracking state.
	ovrTrackingState ts = ovrHmd_GetTrackingState(_hmd, ovr_GetTimeInSeconds());
	// have to check here because it can be not available for some reason
	if(ts.StatusFlags & (ovrStatus_OrientationTracked));
	{
		// we only need the current pose
		ovrPosef ovr_head_pose = ts.HeadPose.ThePose;
		// override head orientation with Oculus information
		head_t.quaternion = convert(ovr_head_pose.Orientation);
	}

	// Calculate the view and projection matrices for 
	for(size_t eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
	{
		ovrEyeType eye = _hmd->EyeRenderOrder[eyeIndex];
		// hard coded eye order (left, right)
		// coordinate system is correct, the otherone (left handed) gives no image
		OVR::Matrix4f ovr_proj = ovrMatrix4f_Projection(_eye_render_desc[eye].Fov, 0.01f, 10000.0f, true);

		Ogre::Matrix4 proj = convert(ovr_proj);

		// Needed as a member variable for end frame
		_eye_pose[eye] = ovrHmd_GetHmdPosePerEye(_hmd, eye);

		// view matrix

		OVR::Vector3f v(_eye_pose[eye].Position);
		Ogre::Vector3 eye_lp(v.x, v.y, v.z);
		Ogre::Vector3 eye_p = camera_q*(head_t.quaternion*eye_lp + head_t.position) + camera_pos;
		Ogre::Quaternion eye_q = camera_q*head_t.quaternion;

		Ogre::Matrix4 view = Ogre::Math::makeViewMatrix(eye_p, eye_q);

		Ogre::Vector3 adjust = convert(_eye_render_desc[eye].HmdToEyeViewOffset);
		Ogre::Matrix4 view_adjust = Ogre::Matrix4::IDENTITY;
		view_adjust.setTrans(adjust);
		
		_channels.at(eyeIndex)->setCustomViewMatrix(true, view_adjust * view);
		_channels.at(eyeIndex)->setCustomProjMatrix(true, proj);
	}
}

void
vl::Window::_end_frame_oculus()
{
	assert(_hmd);

	// renderTargetSize (calculated in initialisation (see PDF doc)
	// textureId OpenGL identifier (retrieve from Channel)
	//
	// update render target size
	// @todo all of these calculations should be in initialisation since they are
	// never changed
	OVR::Sizei render_target_size;
	Rect<uint32_t> size = _channels.at(0)->getTextureSize();
	// Just making sure since we assume this for the rest of this code
	assert(size == _channels.at(1)->getTextureSize());
	render_target_size.w = size.w;
	render_target_size.h = size.h;

	// We are using separate render textures
	_eye_render_viewport[0].Pos  = OVR::Vector2i(0,0);
	_eye_render_viewport[0].Size = render_target_size;
	_eye_render_viewport[1].Pos  = OVR::Vector2i(0, 0);
	_eye_render_viewport[1].Size = render_target_size;

	// array of two for both eyes
	ovrGLTexture eyeTextures[2];
	eyeTextures[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	// TextureSize is the actual size of the texture
	// RenderViewport is the portion of the texture
	eyeTextures[0].OGL.Header.TextureSize = render_target_size;
	eyeTextures[0].OGL.Header.RenderViewport = _eye_render_viewport[0];
	eyeTextures[0].OGL.TexId = _channels.at(0)->getTextureID();
		
	eyeTextures[1].OGL.Header.API = eyeTextures[0].OGL.Header.API;
	eyeTextures[1].OGL.Header.TextureSize = render_target_size;
	eyeTextures[1].OGL.Header.RenderViewport = _eye_render_viewport[1];
	eyeTextures[1].OGL.TexId = _channels.at(1)->getTextureID();

	ovrTexture gl_tex[2];
	gl_tex[0] = eyeTextures[0].Texture;
	gl_tex[1] = eyeTextures[1].Texture;

	ovrHmd_EndFrame(_hmd, _eye_pose, gl_tex);
}
