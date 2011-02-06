/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *
 *
 */

#ifndef EQ_OGRE_WINDOW_HPP
#define EQ_OGRE_WINDOW_HPP

#include "player.hpp"
#include "typedefs.hpp"

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <OGRE/OgreRenderWindow.h>

namespace eqOgre
{

class Channel;
class Pipe;

/**	@class Window represent an OpenGL drawable and context
 *
 */
class Window : public OIS::KeyListener, public OIS::MouseListener
{
public:
	Window( std::string const &name, eqOgre::Pipe *parent );

	virtual ~Window( void );

	Ogre::RenderWindow *getRenderWindow( void )
	{ return _ogre_window; }

	vl::EnvSettingsRefPtr getSettings( void );

	vl::Player const &getPlayer( void ) const;

	vl::ogre::RootRefPtr getOgreRoot( void );

	void setCamera( Ogre::Camera *camera );

	Ogre::Camera *getCamera( void );

	Ogre::SceneManager *getSceneManager( void );

	std::string const &getName( void ) const
	{ return _name; }

	void takeScreenshot( std::string const &prefix, std::string const &suffix );

	/// Capture input events
	virtual void capture( void );

	/// OIS callback overrides
	bool keyPressed(const OIS::KeyEvent &key);
	bool keyReleased(const OIS::KeyEvent &key);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	/// Instruct the Channels to draw the Scene
	virtual void draw( void );

	/// Swap the back buffer to front
	virtual void swap( void );

protected :
	void _createOgreWindow( void );

	/// Create the OIS input handling
	/// For now supports mouse and keyboard
	/// @TODO add joystick support
	void _createInputHandling( void );

	void _printInputInformation( void );

	void _sendEvent( vl::cluster::EventData const &event );

	std::string _name;

	eqOgre::Pipe *_pipe;
	/// @TODO multi channel support?
	eqOgre::Channel *_channel;

	// Ogre
	Ogre::RenderWindow *_ogre_window;

	// OIS variables
	OIS::InputManager *_input_manager;
	OIS::Keyboard *_keyboard;
	OIS::Mouse *_mouse;

};	// class Window

}	// namespace eqOgre

#endif // EQ_OGRE_WINDOW_HPP
