#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "frame_data.hpp"
#include "settings.hpp"

#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector3.h>
#include "eq_settings.hpp"

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

		void mapData( uint32_t const initDataID );

		/// These are mostly called from other eqOgre classes
		/// which need the settings to be distributed so it does not make sense
		/// to return the base class at this point.
		eqOgre::SettingsRefPtr getSettings( void )
		{ return _settings; }

		eqOgre::SettingsRefPtr const getSettings( void ) const
		{ return _settings; }

		// TODO this could be done with vl::SettingsRefPtr or vl::Settings
		// if we have a valid copy constructor
		void setSettings( eqOgre::SettingsRefPtr settings )
		{
			if( settings )
			{
				_settings = settings;
				_createTracker(_settings);
			}
		}
		
	protected :
		virtual ~Config (void);

		void _createTracker( vl::SettingsRefPtr settings );
		
		bool _handleKeyPressEvent( const eq::KeyEvent& event );
		bool _handleKeyReleaseEvent( const eq::KeyEvent& event );
		bool _handleMousePressEvent( const eq::PointerEvent& event );
		bool _handleMouseReleaseEvent( const eq::PointerEvent& event );
		bool _handleMouseMotionEvent( const eq::PointerEvent& event );
		// TODO replace the MagellanEvent with a real JoystickEvent
		bool _handleJoystickEvent( const eq::MagellanEvent& event );

		vl::TrackerRefPtr _tracker;

		eqOgre::SettingsRefPtr _settings;

		FrameData _frame_data;

		// Some test variables for input events
		Ogre::Vector3 _camera_move_dir;
		Ogre::Quaternion _camera_rot_dir;
		Ogre::Quaternion _ogre_rot_dir;
    };	// class Config

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
