#ifndef EQ_OGRE_WINDOW_H
#define EQ_OGRE_WINDOW_H

#include <eq/client/window.h>

#include "eq_ogre/ogre_root.hpp"
#include "tracker.hpp"

namespace eqOgre
{
	class Camera;

    /**
     * A window represent an OpenGL drawable and context
     *
     * Manages OpenGL-specific data, i.e., it creates the logo texture during
     * initialization and holds a state object for GL object creation. It
     * initializes the OpenGL state and draws the statistics overlay.
     */
    class Window : public eq::Window
    {
    public:
		Window( eq::Pipe *parent );

		virtual ~Window( void );
		
		Ogre::RenderWindow *getRenderWindow( void )
		{ return _ogre_window; }

		Ogre::Camera *getCamera( void )
		{ return _camera; }

		vl::TrackerRefPtr getTracker( void )
		{ return _tracker; }

		Ogre::SceneManager *getSceneManager( void )
		{ return _sm; }

		bool loadScene( void );

	protected :
		void createOgreRoot( void );
		void createOgreWindow( void );
		void createTracker( void );

		// Equalizer overrides
		virtual bool configInit( const uint32_t initID );
		virtual bool configInitSystemWindow( const uint32_t initID );

		vl::ogre::RootRefPtr _root;
		Ogre::RenderWindow *_ogre_window;
		Ogre::Camera *_camera;
		Ogre::SceneManager *_sm;

		vl::SettingsRefPtr _settings;
		vl::TrackerRefPtr _tracker;
    };	// class Window

}	// namespace eqOgre

#endif // EQ_OGRE_WINDOW_H
