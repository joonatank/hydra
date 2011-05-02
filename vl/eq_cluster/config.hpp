/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-10
 *	@file config.hpp
 */

#ifndef VL_CONFIG_HPP
#define VL_CONFIG_HPP

// Necessary for the project settings
#include "settings.hpp"
#include "base/exceptions.hpp"

// Necessary for the ref pointers
#include "typedefs.hpp"
// Necessary for the message passing server, owned by Config
#include "cluster/server.hpp"
// A base class for the Config
#include "session.hpp"

#include "gui/gui.hpp"

#include "trigger.hpp"

#include "logger.hpp"

#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISJoyStick.h>

// Necessary for timing statistics
#include <OGRE/OgreTimer.h>

namespace vl
{

/**	@class Config
 *
 */
class Config : public vl::Session
{
public:
	Config( vl::Settings const &settings,
			vl::EnvSettingsRefPtr env,
			vl::Logger &logger );

	virtual ~Config (void);

	/// @todo this should send initialisation messages to all the rendering
	/// threads
	virtual void init( void );

	virtual void exit (void);

	virtual void render( void );

	virtual bool isRunning( void )
	{ return _running; }

	virtual void stopRunning( void )
	{ _running = false; }

protected :
	void _updateServer( void );
	void _sendEnvironment( void );
	void _sendProject( void );

	/// Tracking
	void _createTrackers(vl::EnvSettingsRefPtr settings);

	/// Scene
	void _loadScenes( void );
	void _hideCollisionBarries( void );

	/// Resources
	void _createResourceManager( vl::Settings const &settings, vl::EnvSettingsRefPtr env );


	/// Events
	void _createQuitEvent( void );

	void _receiveEventMessages( void );
	void _receiveCommandMessages( void );

	/// Input Events
	/// Keyboard
	void _handleKeyPressEvent( OIS::KeyEvent const &event );
	void _handleKeyReleaseEvent( OIS::KeyEvent const &event );
	/// Mouse
	void _handleMousePressEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id );
	void _handleMouseReleaseEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id );
	void _handleMouseMotionEvent( OIS::MouseEvent const &event );
	/// Joystick
	void _handleJoystickButtonPressedEvent( OIS::JoyStickEvent const &event, int button );
	void _handleJoystickButtonReleasedEvent( OIS::JoyStickEvent const &event, int button );
	void _handleJoystickAxisMovedEvent( OIS::JoyStickEvent const &event, int axis );
	void _handleJoystickPovMovedEvent( OIS::JoyStickEvent const &event, int pov );
	void _handleJoystickVector3MovedEvent( OIS::JoyStickEvent const &event, int index );

	vl::GameManagerPtr _game_manager;

	vl::Settings _settings;

	vl::EnvSettingsRefPtr _env;

	vl::cluster::ServerRefPtr _server;

	vl::gui::GUI *_gui;

	Ogre::Timer _stats_timer;

	bool _running;

};	// class Config

}	// namespace vl

#endif // VL_CONFIG_HPP
