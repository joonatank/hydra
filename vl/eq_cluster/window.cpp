/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *
 */

#include "window.hpp"

#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "eq_ogre/ogre_dotscene_loader.hpp"

#include <OGRE/OgreWindowEventUtilities.h>
#include <OGRE/OgreLogManager.h>

#include "channel.hpp"
#include "pipe.hpp"

/// ----------------------------- Public ---------------------------------------
eqOgre::Window::Window( std::string const &name, eqOgre::Pipe *parent )
	: _name(name), _pipe( parent ), _channel(0), _ogre_window(0),
	_input_manager(0), _keyboard(0), _mouse(0)
{
	assert( _pipe );

	_createOgreWindow();
	_createInputHandling();
	// FIXME this should pass the real name of the Channel
	_channel = new eqOgre::Channel( "", this );

	// Init channel
	// TODO this is idiotic here
	if( getCamera() )
	{ _channel->setCamera( getCamera() ); }

	Ogre::Viewport *viewport = _ogre_window->addViewport( getCamera() );
	// Set some parameters to the viewport
	// TODO this should be configurable
	viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
	viewport->setAutoUpdated(false);
	_channel->setViewport(viewport);
}

eqOgre::Window::~Window(void )
{
	// Should clean out OIS and Ogre
	std::string message = "Cleaning out OIS";
	Ogre::LogManager::getSingleton().logMessage(message);
	if( _input_manager )
	{
		message = "Destroy OIS input manager.";
		Ogre::LogManager::getSingleton().logMessage(message);
        OIS::InputManager::destroyInputSystem(_input_manager);
		_input_manager = 0;
	}
}

vl::EnvSettingsRefPtr
eqOgre::Window::getSettings( void )
{
	return _pipe->getSettings();
}

vl::Player const &
eqOgre::Window::getPlayer( void ) const
{
	return _pipe->getPlayer();
}

vl::ogre::RootRefPtr
eqOgre::Window::getOgreRoot( void )
{
	return _pipe->getRoot();
}

void
eqOgre::Window::setCamera( Ogre::Camera *camera )
{
	assert( _channel );
	_channel->setCamera(camera);
}

Ogre::Camera *
eqOgre::Window::getCamera( void )
{
	return _pipe->getCamera();
}

Ogre::SceneManager *
eqOgre::Window::getSceneManager( void )
{
	return _pipe->getSceneManager();
}

void
eqOgre::Window::takeScreenshot( const std::string& prefix,
								const std::string& suffix )
{
	std::string real_suffix;
	if( getName().empty() )
	{ real_suffix = suffix; }
	else
	{ real_suffix = "-" + getName()+ suffix; }

	_ogre_window->writeContentsToTimestampedFile(prefix, real_suffix);
}

/// ------------------------ Public OIS Callbacks ------------------------------
bool
eqOgre::Window::keyPressed( OIS::KeyEvent const &key )
{
	vl::cluster::EventData data( vl::cluster::EVT_KEY_PRESSED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteStream stream = data.getStream();
	stream << key;
	_sendEvent( data );

	return true;
}

bool
eqOgre::Window::keyReleased( OIS::KeyEvent const &key )
{
	vl::cluster::EventData data( vl::cluster::EVT_KEY_RELEASED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteStream stream = data.getStream();
	stream << key;
	_sendEvent( data );

	return true;
}

bool
eqOgre::Window::mouseMoved( OIS::MouseEvent const &evt )
{
	vl::cluster::EventData data( vl::cluster::EVT_MOUSE_MOVED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteStream stream = data.getStream();
	stream << evt;
	_sendEvent( data );

	return true;
}

bool
eqOgre::Window::mousePressed( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	vl::cluster::EventData data( vl::cluster::EVT_MOUSE_PRESSED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteStream stream = data.getStream();
	stream << id << evt;
	_sendEvent( data );

	return true;
}

bool
eqOgre::Window::mouseReleased( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	vl::cluster::EventData data( vl::cluster::EVT_MOUSE_RELEASED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteStream stream = data.getStream();
	stream << id << evt;
	_sendEvent( data );

	return true;
}


void
eqOgre::Window::draw( void )
{
	// TODO support for multiple channels
	_channel->draw();
}

void
eqOgre::Window::swap( void )
{
	_ogre_window->swapBuffers();
}


void
eqOgre::Window::capture( void )
{
	if( _keyboard && _mouse )
	{
		_keyboard->capture();
		_mouse->capture();
	}
	else
	{
		std::string message("Mouse or keyboard does not exists! No input handling.");
		std::cerr << message << std::endl;
// 		Ogre::LogManager::getSingleton().logMessage(message);
	}
}

void
eqOgre::Window::_sendEvent( vl::cluster::EventData const &event )
{
	_pipe->sendEvent(event);
}

void
eqOgre::Window::_createInputHandling( void )
{
	std::string message( "Creating OIS Input system." );
	std::cerr << message << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage(message);

	assert( _ogre_window );

	std::ostringstream windowHndStr;
	size_t windowHnd = 0;
	_ogre_window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;

	OIS::ParamList pl;
	pl.insert( std::make_pair(std::string("WINDOW"), windowHndStr.str()) );
	// Some extra parameters to avoid getting stuck with the window
	// TODO These can probably be removed
	pl.insert( std::make_pair(std::string("x11_keyboard_grab"), std::string("false") ) );
	pl.insert( std::make_pair(std::string("x11_mouse_grab"), std::string("false") ) );

	// Info
	message = "Creating OIS Input Manager";
	std::cout << message << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage(message);

	_input_manager = OIS::InputManager::createInputSystem( pl );
	message = "OIS Input Manager created";
	std::cout << message << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage(message);

	_printInputInformation();

	_keyboard = static_cast<OIS::Keyboard*>(_input_manager->createInputObject(OIS::OISKeyboard, true));
	_keyboard->setEventCallback(this);

	_mouse = static_cast<OIS::Mouse*>(_input_manager->createInputObject(OIS::OISMouse, true));

	_mouse ->getMouseState().height = _ogre_window->getHeight();
	_mouse ->getMouseState().width	= _ogre_window->getWidth();

	_mouse->setEventCallback(this);

	// Info
	message = "Input system created.";
	std::cout << message << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage(message);
}

void
eqOgre::Window::_printInputInformation( void )
{
	// Print debugging information
	// TODO debug information should go to Ogre Log file
	unsigned int v = _input_manager->getVersionNumber();
	std::stringstream ss;
	ss << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
		<< "\nRelease Name: " << _input_manager->getVersionName()
		<< "\nManager: " << _input_manager->inputSystemName()
		<< "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
		<< "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
		<< "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick)
		<< '\n';
	std::cout << ss.str() << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage( ss.str() );

	ss.str("");
	// List all devices
	// TODO should go to Ogre Log file
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ ss << "\n\tDevice: " << " Vendor: " << i->second; }
	std::cout << ss.str() << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage( ss.str() );
}

void
eqOgre::Window::_createOgreWindow( void )
{
	// Info
	std::string message = "Creating Ogre RenderWindow.";
	std::cout << message << std::endl;
// 	Ogre::LogManager::getSingleton().logMessage( message );

	Ogre::NameValuePairList params;

	vl::EnvSettings::Window winConf = _pipe->getWindowConf( getName() );
	assert( !winConf.empty() );

	params["left"] = winConf.x;
	params["top"] = winConf.y;

	// TODO test stereo
	// If it doesn't work do some custom updates to the Ogre Library
	// They are easy to merge using Mercurial
	// And can be commited to our own Ogre fork.
	// TODO test swap sync
	// same as with stereo
	_ogre_window = getOgreRoot()->createWindow( "Hydra-"+getName(), winConf.w, winConf.h, params );
}
