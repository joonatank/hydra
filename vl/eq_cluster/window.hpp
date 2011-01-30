/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

#ifndef EQ_OGRE_WINDOW_HPP
#define EQ_OGRE_WINDOW_HPP

#include <eq/client/window.h>

#include "eq_settings.hpp"
#include "player.hpp"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <OGRE/OgreRenderWindow.h>

namespace eqOgre
{
    /**
     * A window represent an OpenGL drawable and context
     *
     * Manages OpenGL-specific data, i.e., it creates the logo texture during
     * initialization and holds a state object for GL object creation. It
     * initializes the OpenGL state and draws the statistics overlay.
     */
    class Window : public eq::Window,
				   public OIS::KeyListener, public OIS::MouseListener
    {
    public:
		Window( eq::Pipe *parent );

		virtual ~Window( void );

		Ogre::RenderWindow *getRenderWindow( void )
		{ return _ogre_window; }

		DistributedSettings const &getSettings( void ) const;

		vl::Player const &getPlayer( void ) const;

		vl::ogre::RootRefPtr getOgreRoot( void );

		void setCamera( Ogre::Camera *camera );

		Ogre::Camera *getCamera( void );

		Ogre::SceneManager *getSceneManager( void );

		void takeScreenshot( std::string const &prefix, std::string const &suffix );

		/// OIS overrides
		bool keyPressed(const OIS::KeyEvent &key);
		bool keyReleased(const OIS::KeyEvent &key);

		bool mouseMoved(const OIS::MouseEvent &evt);
		bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	protected :
		void createOgreWindow( void );

		/// Create the OIS input handling
		/// For now supports mouse and keyboard
		// TODO add joystick support
		void createInputHandling( void );

		void printInputInformation( void );

		/// Equalizer overrides
		virtual bool configInit( const eq::uint128_t& initID );

		virtual bool configExit( void );

		virtual void frameStart( const eq::uint128_t& frameID,
								 const uint32_t frameNumber );

		virtual void frameFinish( const eq::uint128_t& frameID,
								  const uint32_t frameNumber );

		/// Override system window creation because we
		/// use OIS for input handling
		virtual bool configInitSystemWindow( const eq::uint128_t &initID );

		void _sendEvent( vl::cluster::EventData const &event );

		// Ogre
		Ogre::RenderWindow *_ogre_window;

		// OIS variables
		OIS::InputManager *_input_manager;
		OIS::Keyboard *_keyboard;
		OIS::Mouse *_mouse;

    };	// class Window

}	// namespace eqOgre

#endif // EQ_OGRE_WINDOW_HPP
