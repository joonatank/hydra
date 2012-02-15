/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file window.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_WINDOW_HPP
#define HYDRA_WINDOW_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

// Interface
#include "window_interface.hpp"

#include "player.hpp"
#include "typedefs.hpp"

// Necessary for Window config and Wall
#include "base/envsettings.hpp"

#include "renderer_interface.hpp"
// Necessary for frustum type used in projection calculation
#include "math/frustum.hpp"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#include <OIS/OISJoyStick.h>

#include <OGRE/OgreRenderWindow.h>

// GUI
#include <CEGUI/CEGUIEventArgs.h>

#include <OGRE/SdkTrays.h>

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

	vl::ogre::RootRefPtr getOgreRoot( void );

	void setCamera(vl::CameraPtr camera);

	std::string const &getName( void ) const
	{ return _name; }

	void takeScreenshot( std::string const &prefix, std::string const &suffix );

	void setIPD(double ipd)
	{ _ipd = ipd; }

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

	Ogre::RenderTarget::FrameStats const &getStatistics(void) const;

	void resetStatistics(void);

	/// OIS callback overrides
	bool keyPressed(const OIS::KeyEvent &key);
	bool keyReleased(const OIS::KeyEvent &key);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	bool buttonPressed(OIS::JoyStickEvent const &evt, int button);
	bool buttonReleased(OIS::JoyStickEvent const &evt, int button);
	bool axisMoved(OIS::JoyStickEvent const &evt, int axis);
	bool povMoved(OIS::JoyStickEvent const &evt, int pov);
	bool vector3Moved(OIS::JoyStickEvent const &evt, int index);

protected :
	Ogre::RenderWindow *_createOgreWindow(vl::config::Window const &winConf);

	/// Create the OIS input handling
	/// For now supports mouse and keyboard
	/// @TODO add joystick support
	void _createInputHandling( void );

	void _printInputInformation( void );

	void _sendEvent( vl::cluster::EventData const &event );

	void _lazy_initialisation(void);

	std::string _name;

	vl::RendererInterface *_renderer;

	vl::Frustum _frustum;

	double _ipd;

	vl::Camera *_camera;

	// Ogre
	Ogre::RenderWindow *_ogre_window;
	Ogre::Viewport *_left_viewport;
	Ogre::Viewport *_right_viewport;

	// OIS variables
	OIS::InputManager *_input_manager;
	OIS::Keyboard *_keyboard;
	OIS::Mouse *_mouse;
	std::vector<OIS::JoyStick *> _joysticks;

	OgreBites::SdkTrayManager *_tray_mgr;

};	// class Window

}	// namespace vl

#endif // HYDRA_WINDOW_HPP
