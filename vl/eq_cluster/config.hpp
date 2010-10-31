#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "frame_data.hpp"
#include "init_data.hpp"
#include "settings.hpp"

#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector3.h>

namespace eqOgre
{
    class Config : public eq::Config
    {
    public:
		Config( eq::base::RefPtr< eq::Server > parent );

		/** @sa eq::Config::init. */
		virtual bool init( uint32_t const initID );

		/** @sa eq::Config::exit. */
//		virtual bool exit (void);

        /** @sa eq::Config::handleEvent */
		virtual bool handleEvent( const eq::ConfigEvent* event );

		virtual uint32_t startFrame (const uint32_t frameID);

		void setInitData( InitData const &data )
		{ _init_data = data; }

		InitData const &getInitData( void )
		{ return _init_data; }

		void mapData( uint32_t const initDataID );

		void unmapData( void );
		
		void setSettings( vl::SettingsRefPtr set )
		{ _settings = set; }

		vl::SettingsRefPtr getSettings( void )
		{ return _init_data.getSettings(); }

	protected :
		virtual ~Config (void);

		bool _handleKeyPressEvent( const eq::KeyEvent& event );
		bool _handleKeyReleaseEvent( const eq::KeyEvent& event );
		bool _handleMousePressEvent( const eq::PointerEvent& event );
		bool _handleMouseReleaseEvent( const eq::PointerEvent& event );
		bool _handleMouseMotionEvent( const eq::PointerEvent& event );
		// TODO replace the MagellanEvent with a real JoystickEvent
		bool _handleJoystickEvent( const eq::MagellanEvent& event );

		vl::SettingsRefPtr _settings;

		InitData _init_data;
		FrameData _frame_data;

		// Some test variables for input events
		Ogre::Vector3 _camera_move_dir;
		Ogre::Quaternion _camera_rot_dir;
		Ogre::Quaternion _ogre_rot_dir;
    };	// class Config

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
