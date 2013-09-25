/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file window.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_WINDOW_HPP
#define HYDRA_WINDOW_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

// Interface
#include "window_interface.hpp"

#include "typedefs.hpp"

// Necessary for Window config and Wall
#include "base/envsettings.hpp"
// Parent
#include "renderer_interface.hpp"
// Child
#include "channel.hpp"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#include <OIS/OISJoyStick.h>

#include <OGRE/OgreRenderTargetListener.h>

#include <OGRE/OgreRenderWindow.h>

namespace vl
{

/**	@class Window represent an OpenGL drawable and context
 *
 */
class HYDRA_API Window : public IWindow, public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
public:
	/// pass Renderer as parent, not ref ptr because this shouldn't hold ownership
	Window(vl::config::Window const &windowConf, vl::RendererInterface *parent);

	virtual ~Window( void );

	Ogre::RenderWindow *getRenderWindow( void )
	{ return _ogre_window; }

	vl::config::EnvSettingsRefPtr getEnvironment(void) const;

	vl::Player const &getPlayer( void ) const;

	vl::Player *getPlayerPtr(void);

	vl::ogre::RootRefPtr getOgreRoot( void );

	void setCamera(vl::CameraPtr camera);

	std::string const &getName( void ) const
	{ return _name; }

	void takeScreenshot( std::string const &prefix, std::string const &suffix );

	/// @brief return the system handle for the window
	uint64_t getHandle(void) const;

	/// @brief Get wether hardware stereo is enabled or not
	/// @return true if the window has stereo enabled
	bool hasStereo(void) const;

	/// Capture input events
	virtual void capture( void );

	/// Instruct the Channels to draw the Scene
	virtual void draw( void );

	/// Swap the back buffer to front
	virtual void swap( void );

	virtual void resize(int w, int h);

	std::vector<Channel *> const &getChannels(void)
	{ return _channels; }

	Ogre::RenderTarget::FrameStats getStatistics(void) const;

	void resetStatistics(void);

	/// OIS callback overrides
	bool keyPressed(const OIS::KeyEvent &key);
	bool keyReleased(const OIS::KeyEvent &key);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	bool buttonPressed(OIS::JoyStickEvent const &evt, int index);
	bool buttonReleased(OIS::JoyStickEvent const &evt, int index);
	bool axisMoved(OIS::JoyStickEvent const &evt, int index);
	bool povMoved(OIS::JoyStickEvent const &evt, int index);
	bool vector3Moved(OIS::JoyStickEvent const &evt, int index);
	bool sliderMoved(OIS::JoyStickEvent const &evt, int index);

protected :

	Ogre::RenderWindow *_createOgreWindow(vl::config::Window const &winConf);

	vl::Channel *_create_channel(vl::config::Channel const &channel, 
		STEREO_EYE stereo_cfg, vl::config::Projection const &projection, uint32_t fsaa);

	/// Create the OIS input handling
	/// For now supports mouse and keyboard
	/// @TODO add joystick support
	void _createInputHandling( void );

	void _printInputInformation( void );

	void _sendEvent( vl::cluster::EventData const &event );

	void _render_to_fbo(void);

	void _update_debug_overlay(Ogre::RenderTarget::FrameStats const &stats);

	void _draw_fbo_to_screen(void);

	void _render_to_screen(void);

	void _initialise_fbo(vl::CameraPtr camera);

	std::string _name;

	vl::RendererInterface *_renderer;

	std::vector<Channel *> _channels;

	// Ogre
	Ogre::RenderWindow *_ogre_window;

	// OIS variables
	OIS::InputManager *_input_manager;
	OIS::Keyboard *_keyboard;
	OIS::Mouse *_mouse;
	std::vector<OIS::JoyStick *> _joysticks;

	vl::config::Renderer::Type _renderer_type;

};	// class Window

}	// namespace vl

#endif // HYDRA_WINDOW_HPP
