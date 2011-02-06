/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include "settings.hpp"
#include "base/exceptions.hpp"

#include "typedefs.hpp"
#include "cluster/server.hpp"
#include "session.hpp"

#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>


namespace eqOgre
{

	class Config : public vl::Session
	{
	public:
		Config( vl::GameManagerPtr man,
				vl::Settings const &settings,
				vl::EnvSettingsRefPtr env );

		virtual bool init( uint64_t const &initID );

		virtual bool exit (void);

		virtual uint32_t startFrame( uint64_t const &frameID );

		virtual void finishFrame( void );

		virtual bool isRunning( void )
		{ return _running; }

		virtual void stopRunning( void )
		{ _running = false; }

	protected :
		virtual ~Config (void);

		void _createServer( void );
		void _updateServer( void );
		void _sendEnvironment( void );
		void _sendProject( void );

		/// Tracking
		void _createTracker( vl::EnvSettingsRefPtr settings );

		/// Scene
		void _loadScenes( void );

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

		bool _running;

	};	// class Config


}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
