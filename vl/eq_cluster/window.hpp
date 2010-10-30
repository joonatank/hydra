#ifndef EQ_OGRE_WINDOW_H
#define EQ_OGRE_WINDOW_H

#include <eq/client/window.h>

#include "eq_ogre/ogre_root.hpp"
#include "tracker.hpp"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
//#include <OGRE/OgreFrameListener.h>
//#include <OGRE/OgreWindowEventUtilities.h>

namespace eqOgre
{
    /**
     * A window represent an OpenGL drawable and context
     *
     * Manages OpenGL-specific data, i.e., it creates the logo texture during
     * initialization and holds a state object for GL object creation. It
     * initializes the OpenGL state and draws the statistics overlay.
     */
    class Window : public eq::Window, //Ogre::FrameListener, Ogre::WindowEventListener,
						  OIS::KeyListener, OIS::MouseListener
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

		/// OIS overrides

		bool keyPressed(const OIS::KeyEvent &key);
		bool keyReleased(const OIS::KeyEvent &key);

		bool mouseMoved(const OIS::MouseEvent &evt);
		bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

		
	protected :
		void checkX11Events( void );
		
		void createOgreRoot( void );
		void createOgreWindow( void );
		void createTracker( void );

		/// Create the OIS input handling
		void createInputHandling( void );


		
		/// Equalizer overrides
		virtual bool configInit( const uint32_t initID );

		virtual void frameFinish( const uint32_t frameID, const uint32_t frameNumber );

		/// Override system window creation because we
		/// use OIS for input handling
		virtual bool configInitSystemWindow( const uint32_t initID );

		vl::ogre::RootRefPtr _root;
		Ogre::RenderWindow *_ogre_window;
		Ogre::Camera *_camera;
		Ogre::SceneManager *_sm;

		vl::SettingsRefPtr _settings;
		vl::TrackerRefPtr _tracker;

		// OIS variables
		OIS::InputManager *_input_manager;
		OIS::Keyboard *_keyboard;
		OIS::Mouse *_mouse;
		
    };	// class Window

}	// namespace eqOgre

#endif // EQ_OGRE_WINDOW_H
