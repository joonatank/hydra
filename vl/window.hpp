/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file window.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
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
// Base class
#include "cluster/distributed.hpp"
// Typedefs
#include "typedefs.hpp"
// Necessary for Window config and Wall
#include "base/config.hpp"
// Child
#include "channel.hpp"

// OIS
#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#include <OIS/OISJoyStick.h>

// Implementation
#include <OGRE/OgreRenderWindow.h>

namespace vl
{

/**	@class Window represent an OpenGL drawable and context
 *
 */
class HYDRA_API Window : public IWindow, public Distributed, public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
public:
	/// @brief Constructor
	/// @param windowConf config for this window
	/// @param parent Pipe that created this window
	Window(vl::config::Window const &windowConf, vl::PipePtr parent);

	/// Slave constructor
	/// @param parent
	/// @param renderer We need renderer for creating Ogre window
	/// @param id 
	/// pass Renderer as parent, not ref ptr because this shouldn't hold ownership
	Window(vl::RendererPtr renderer, uint64_t id);

	virtual ~Window( void );

	Ogre::RenderWindow *getRenderWindow( void )
	{ return _ogre_window; }

	/// @brief set the same camera to all Channels
	void setCamera(vl::CameraPtr camera);

	std::string const &getName( void ) const
	{ return _name; }

	// conveniance function because Pipe differs for slaves and masters
	vl::Pipe *getPipe(void);

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

	// Interesting thing the resize allows us create larger than screen windows.
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

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_PIPE = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CONFIG = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_CHANNELS = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 4,
	};

	/// Called from Pipe when window is atteched
	void _createNative(void);

private :
	virtual void serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	virtual void deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	/// @todo should Ogre::RenderWindow be passed to constructor or created here?
	Ogre::RenderWindow *_createOgreWindow(vl::config::Window const &winConf);

	/// @brief
	/// @todo wall is tied to projection we should not need them both
	/// @todo this should be public so we can move channel creation to Renderer
	vl::Channel *_create_channel(vl::config::Channel const &channel, 
		STEREO_EYE stereo_cfg, RENDER_MODE render_mode, uint32_t fsaa);

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

	config::Window _window_config;

	/// Pipe used on Master
	vl::PipePtr _pipe;
	/// Pipe ID used on slaves to determine if the Pipe is on this Node
	uint64_t _pipe_id;

	// Need to have Renderer on Slaves for creating Ogre window
	vl::RendererPtr _renderer;

	std::vector<Channel *> _channels;

	// Ogre
	Ogre::RenderWindow *_ogre_window;

	// OIS
	OIS::InputManager *_input_manager;
	OIS::Keyboard *_keyboard;
	OIS::Mouse *_mouse;
	std::vector<OIS::JoyStick *> _joysticks;

};	// class Window

}	// namespace vl

#endif // HYDRA_WINDOW_HPP
