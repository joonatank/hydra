#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

// Audio
#include <cAudio/cAudio.h>

#include "frame_data.hpp"
#include "settings.hpp"
#include "eq_settings.hpp"
#include "base/exceptions.hpp"
#include "transform_event.hpp"
#include "event.hpp"

// python
#include "python.hpp"
#include "event_manager.hpp"

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
		void setSettings( eqOgre::SettingsRefPtr settings );

		bool addEvent( TransformationEvent const &event );

		bool removeEvent( TransformationEvent const &event );

		bool hasEvent( TransformationEvent const &event );

		void addSceneNode( SceneNode *node );

		void removeSceneNode( SceneNode *node );

		void updateSceneVersion( void )
		{ _frame_data.updateSceneVersion(); }

	protected :
		virtual ~Config (void);

		/// Audio
		void _initAudio( void );
		void _exitAudio( void );

		/// Tracking
		void _createTracker( vl::SettingsRefPtr settings );
		void _setHeadMatrix( Ogre::Matrix4 const &m );

		/// Python
		void _initPython( void );
		void _runPythonScript( std::string const &scriptFile );

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

		// NOTE we need to use Event pointer because Events can be inherited
		std::vector<Event *> _events;
		std::vector<TransformationEvent> _trans_events;

		EventManager *_event_manager;

		// Python related
		python::object _global;

		// Audio objects
		 cAudio::IAudioManager *_audio_manager;
		 cAudio::IAudioSource *_background_sound;
	};	// class Config


}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
