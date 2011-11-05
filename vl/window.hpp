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
#include <OGRE/OgreRenderTargetListener.h>

// GUI
#include <CEGUI/CEGUIEventArgs.h>

namespace vl
{

/// Toggles stereo mode for OpenGL
/// The render target needs to be renderer twice for this to work
class StereoRenderTargetListener : public Ogre::RenderTargetListener
{
public :
	StereoRenderTargetListener(bool s)
		: stereo(s)
	{}

	virtual void preRenderTargetUpdate(Ogre::RenderTargetEvent const &evt);

	virtual void postRenderTargetUpdate(Ogre::RenderTargetEvent const &evt)
	{
	}

	bool stereo;
	bool _left;

};	// class StereoRenderTargetListener

/// @todo this should be implemented as an Ogre Movable object
/// a replacement for the Ogre::Camera class
/// before rendering it should be updated with 
///		head matrix
///		cyclop transformation
///		ipd
class StereoCamera
{
public :
	StereoCamera(void);

	~StereoCamera(void);

	void setHead(vl::Transform const &head);

	void setCamera(vl::CameraPtr cam);

	vl::CameraPtr getCamera(void) const
	{ return _camera; }

	/// @brief updates the Ogre::Camera's projection and view matrices
	/// @param eye_x the eye x direction used for this rendering
	/// For stereo rendering this needs to be called twice with different
	/// eye_x values. Also you need to call Viewport::update also.
	void update(vl::scalar eye_x);

	/// @todo should this store the ipd value and have separate updateLeft
	/// and updateRight methods for the left and right eye?
//	void setIPD(vl::scalar ipd);

	vl::Frustum &getFrustum(void)
	{ return _frustum; }

	vl::Frustum const &getFrustum(void) const
	{ return _frustum; }

	void setFrustum(vl::Frustum const &f)
	{ _frustum = f; }

private :
	vl::CameraPtr _camera;

	Ogre::Camera *_ogre_camera;

	vl::Transform _head;

	vl::Frustum _frustum;

};	// class StereoCamera


/**	@class Window represent an OpenGL drawable and context
 *
 */
class Window : public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
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

	/// @brief Get wether hardware stereo is enabled or not
	/// @return true if the window has stereo enabled
	bool hasStereo(void) const;

	/// Capture input events
	virtual void capture( void );

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

	/// Instruct the Channels to draw the Scene
	virtual void draw( void );

	/// Swap the back buffer to front
	virtual void swap( void );

protected :
	Ogre::RenderWindow *_createOgreWindow(vl::config::Window const &winConf);

	/// Create the OIS input handling
	/// For now supports mouse and keyboard
	/// @TODO add joystick support
	void _createInputHandling( void );

	void _printInputInformation( void );

	void _sendEvent( vl::cluster::EventData const &event );

	void _render_to_fbo(void);

	void _draw_fbo_to_screen(void);

	void _render_to_screen(void);

	void _initialise_fbo(vl::CameraPtr camera);

	std::string _name;

	vl::RendererInterface *_renderer;

	vl::StereoCamera _stereo_camera;

	double _ipd;

	// Ogre
	Ogre::RenderWindow *_ogre_window;

	// OIS variables
	OIS::InputManager *_input_manager;
	OIS::Keyboard *_keyboard;
	OIS::Mouse *_mouse;
	std::vector<OIS::JoyStick *> _joysticks;

	vl::config::Renderer::Type _renderer_type;
	Ogre::RenderTexture *_fbo;
	Ogre::MaterialPtr _fbo_material;
	Ogre::TexturePtr _fbo_texture;

	StereoRenderTargetListener *_window_listener;

};	// class Window

}	// namespace vl

#endif // HYDRA_WINDOW_HPP
