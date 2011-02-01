/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "settings.hpp"
#include "eq_settings.hpp"
#include "base/exceptions.hpp"

#include "typedefs.hpp"
#include "cluster/server.hpp"
#include "distributed.hpp"
#include "session.hpp"

#include <OIS/OISMouse.h>
#include <OIS/OISKeyboard.h>


namespace eqOgre
{

	class Config : public eq::Config, vl::Session
	{
	public:
		Config( eq::base::RefPtr< eq::Server > parent );

		/** @sa eq::Config::init. */
		virtual bool init( eq::uint128_t const &initID );

		/** @sa eq::Config::exit. */
		virtual bool exit (void);

		/** @sa eq::Config::handleEvent */
		virtual bool handleEvent( const eq::ConfigEvent* event )
		{ return true; }

		virtual uint32_t startFrame( eq::uint128_t const &frameID );

		void setSettings( vl::SettingsRefPtr settings );

		void setGameManager( vl::GameManagerPtr man );

	protected :
		virtual ~Config (void);

		void _createServer( void );

		void _updateServer( void );

		/// Tracking
		void _createTracker( vl::SettingsRefPtr settings );

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

		vl::SettingsRefPtr _settings;

		/// Distributed
		DistributedSettings _distrib_settings;

		vl::GameManagerPtr _game_manager;

		vl::cluster::Server *_server;

	};	// class Config


}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
