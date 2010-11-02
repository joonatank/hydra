#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "frame_data.hpp"
#include "settings.hpp"
#include "eq_settings.hpp"
#include "transform_event.hpp"

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
/*
		bool addEvent( TransformationEvent const &event )
		{
			_events.push_back(event);
			return true;
		}

		// TODO add pop event
		bool removeEvent( TransformationEvent const &event )
		{
			// TODO test the operator== in TransformationEvent
			std::vector<TransformationEvent>::iterator iter;
			for( iter = _events.begin(); iter != _events.end(); ++iter )
			{
				if( *iter == event )
				{
					_events.erase(iter);
					return true;
				}
			}

			return false;
		}
*/
		void addSceneNode( SceneNode *node )
		{
			_frame_data.addSceneNode( node );
		}

		// TODO add remove SceneNode
	protected :
		virtual ~Config (void);

		void _createTracker( vl::SettingsRefPtr settings );
		void _setHeadMatrix( Ogre::Matrix4 const &m );
		
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
		// TODO should be moved to a vector
		// TODO should add dynamical addition
		// TODO should expose an interface to outsiders
//		std::vector<TransformationEvent> _events;
		TransformationEvent _camera_trans;
		TransformationEvent _ogre_trans;

    };	// class Config

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
