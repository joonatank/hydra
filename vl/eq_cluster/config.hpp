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
// Necessary for keeping track of statistics
#include "stats.hpp"

#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>

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
	Config( vl::GameManagerPtr man,
			vl::Settings const &settings,
			vl::EnvSettingsRefPtr env );

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
	void _createTracker( vl::EnvSettingsRefPtr settings );

	/// Scene
	void _loadScenes( void );
	void _hideCollisionBarries( void );

	/// Events
	void _createQuitEvent( void );

	void _receiveEventMessages( void );

	/// Input Events
	bool _handleKeyPressEvent( OIS::KeyEvent const &event );
	bool _handleKeyReleaseEvent( OIS::KeyEvent const &event );
	bool _handleMousePressEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id );
	bool _handleMouseReleaseEvent( OIS::MouseEvent const &event, OIS::MouseButtonID id );
	bool _handleMouseMotionEvent( OIS::MouseEvent const &event );
	// TODO add joystick event

	vl::GameManagerPtr _game_manager;

	vl::Settings _settings;

	vl::EnvSettingsRefPtr _env;

	vl::cluster::Server *_server;

	Ogre::Timer _stats_timer;
	vl::Stats _stats;

	bool _running;

};	// class Config

}	// namespace vl

#endif // VL_CONFIG_HPP
