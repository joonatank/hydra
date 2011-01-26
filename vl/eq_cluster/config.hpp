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
// Necessary for the SceneNode functions
// TODO should be removed as soon as they work directly on the current scene
#include "scene_manager.hpp"

#include "base/typedefs.hpp"
#include "cluster/server.hpp"
#include "distributed.hpp"
#include "session.hpp"

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
		virtual bool handleEvent( const eq::ConfigEvent* event );

		virtual uint32_t startFrame( eq::uint128_t const &frameID );

		void setSettings( vl::SettingsRefPtr settings );

		SceneNodePtr createSceneNode( std::string const &name );

		void removeSceneNode( SceneNode *node );

		SceneNodePtr getSceneNode( std::string const &name );

		void setGameManager( vl::GameManagerPtr man );

	protected :
		virtual ~Config (void);

		void _createServer( void );

		void _updateServer( void );

		void _addSceneNode( SceneNode *node );

		/// Tracking
		void _createTracker( vl::SettingsRefPtr settings );

		/// Scene
		void _loadScenes( void );

		/// Events
		void _createQuitEvent( void );

		bool _handleKeyPressEvent( const eq::KeyEvent& event );
		bool _handleKeyReleaseEvent( const eq::KeyEvent& event );
		bool _handleMousePressEvent( const eq::PointerEvent& event );
		bool _handleMouseReleaseEvent( const eq::PointerEvent& event );
		bool _handleMouseMotionEvent( const eq::PointerEvent& event );
		// TODO replace the MagellanEvent with a real JoystickEvent
		bool _handleJoystickEvent( const eq::MagellanEvent& event );

		vl::SettingsRefPtr _settings;

		/// Distributed
		DistributedSettings _distrib_settings;

		vl::GameManagerPtr _game_manager;

		vl::cluster::Server *_server;

	};	// class Config


}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
