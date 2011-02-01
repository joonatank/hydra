/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

#include "window.hpp"

#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "eq_ogre/ogre_dotscene_loader.hpp"

#include <OGRE/OgreWindowEventUtilities.h>
#include <OGRE/OgreLogManager.h>

// System window specific includes
#include <eq/client/systemWindow.h>
#include <eq/client/windowSystem.h>

#ifdef GLX
#include "eq_cluster/glxWindow.hpp"
#endif

#ifdef WGL
#include "eq_cluster/wglWindow.hpp"
#endif

#include "channel.hpp"
#include "pipe.hpp"

/// ----------------------------- Public ---------------------------------------
eqOgre::Window::Window(eq::Pipe *parent)
	: eq::Window( parent ), _ogre_window(0),
	_input_manager(0), _keyboard(0), _mouse(0)
{}

eqOgre::Window::~Window(void )
{}

eqOgre::DistributedSettings const &
eqOgre::Window::getSettings( void ) const
{
	assert( dynamic_cast<eqOgre::Pipe const *>( getPipe() ) );
	return static_cast<eqOgre::Pipe const *>(getPipe())->getSettings();
}

vl::Player const &
eqOgre::Window::getPlayer( void ) const
{
	assert( dynamic_cast<eqOgre::Pipe const *>( getPipe() ) );
	return static_cast<eqOgre::Pipe const *>(getPipe())->getPlayer();
}

vl::ogre::RootRefPtr
eqOgre::Window::getOgreRoot( void )
{
	assert( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
	return static_cast<eqOgre::Pipe *>(getPipe())->getRoot();
}

void
eqOgre::Window::setCamera(Ogre::Camera* camera)
{
	Channels const &chanlist = getChannels();
	for( size_t i = 0; i < chanlist.size(); ++i )
	{
		assert( dynamic_cast<eqOgre::Channel *>( chanlist.at(i) ) );
		eqOgre::Channel *channel =
			static_cast<eqOgre::Channel *>( chanlist.at(i) );
		channel->setCamera( camera );
	}
}

Ogre::Camera *
eqOgre::Window::getCamera( void )
{
	assert( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
	return static_cast<eqOgre::Pipe *>(getPipe())->getCamera();
}

Ogre::SceneManager *
eqOgre::Window::getSceneManager(void )
{
	assert( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
	return static_cast<eqOgre::Pipe *>(getPipe())->getSceneManager();
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


/// -------------------------- Protected ---------------------------------------
// ConfigInit can not throw, it must return false on error. CONFIRMED
bool
eqOgre::Window::configInit( const eq::uint128_t& initID )
{
	std::string message = "eqOgre::Window::configInit";
	Ogre::LogManager::getSingleton().logMessage(message);

	if( !eq::Window::configInit( initID ) )
	{
		// TODO add error status flag
		message = "eq::Window::configInit failed";
		Ogre::LogManager::getSingleton().logMessage(message);
		return false;
	}

	try {
		createOgreWindow();
		createInputHandling();
	}
	catch( vl::exception &e )
	{
		// TODO add error status flag
		message = "VL Exception : " + boost::diagnostic_information<>(e);
		Ogre::LogManager::getSingleton().logMessage(message);
		return false;
	}
	catch( Ogre::Exception const &e)
	{
		// TODO add error status flag
		message = std::string("Ogre Exception: ") + e.what();
		Ogre::LogManager::getSingleton().logMessage(message);
		return false;
	}
	catch( std::exception const &e )
	{
		// TODO add error status flag
		message = std::string("STD Exception: ") + e.what();
		Ogre::LogManager::getSingleton().logMessage(message);
		return false;
	}
	catch( ... )
	{
		// TODO add error status flag
		message = "eqOgre::Window::configInit : Exception thrown.";
		Ogre::LogManager::getSingleton().logMessage(message);
		return false;
	}

	message = "eqOgre::Window::init : done";
	Ogre::LogManager::getSingleton().logMessage(message);
	return true;
}

bool eqOgre::Window::configExit(void )
{
	// Cleanup children first
	bool retval = eq::Window::configExit();

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

	return retval;
}

void
eqOgre::Window::frameStart(const eq::uint128_t& frameID, const uint32_t frameNumber)
{
	// We need to set the Viewport here because the Camera doesn't exists in
	// configInit
	static bool inited = false;
	if( !inited )
	{
		std::string message = "Create a Ogre Viewport for each Channel";
		Ogre::LogManager::getSingleton().logMessage(message);

		Channels chanlist = getChannels();
		for( size_t i = 0; i < chanlist.size(); ++i )
		{
			assert( _ogre_window && getCamera() );
			Ogre::Viewport *viewport = _ogre_window->addViewport( getCamera() );
			assert( dynamic_cast<eqOgre::Channel *>( chanlist.at(i) ) );
			eqOgre::Channel *channel =
				static_cast<eqOgre::Channel *>( chanlist.at(i) );

			// Set some parameters to the viewport
			// TODO this should be configurable from DotScene
			viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
			viewport->setAutoUpdated(false);

			// Cleanup old viewport
			if( channel->getViewport() )
			{ _ogre_window->removeViewport( channel->getViewport()->getZOrder() ); }

			// Set the new viewport
			channel->setViewport( viewport );
		}

		inited = true;
	}

	eq::Window::frameStart(frameID, frameNumber);
}

void
eqOgre::Window::frameFinish(const eq::uint128_t &frameID, const uint32_t frameNumber)
{
	eq::Window::frameFinish(frameID, frameNumber);

	if( _keyboard && _mouse )
	{
		_keyboard->capture();
		_mouse->capture();
	}
	else
	{
		std::string message("Mouse or keyboard does not exists! No input handling.");
		Ogre::LogManager::getSingleton().logMessage(message);
	}
}

bool
eqOgre::Window::configInitSystemWindow(const eq::uint128_t &initID)
{
	const eq::Pipe* pipe = getPipe();
	eq::SystemWindow* systemWindow = 0;

	std::string message;
#if defined GLX
	systemWindow = new eqOgre::GLXWindow( this );
#elif defined WGL
	systemWindow = new eqOgre::WGLWindow( this );
#endif

	assert( systemWindow );
	if( !systemWindow->configInit() )
	{
		message = "System Window initialization failed: ";
		Ogre::LogManager::getSingleton().logMessage(message);
		delete systemWindow;
		return false;
	}

	setSystemWindow( systemWindow );
	return true;
}

void
eqOgre::Window::_sendEvent( vl::cluster::EventData const &event )
{
	assert( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
	eqOgre::Pipe *pipe = static_cast<eqOgre::Pipe *>( getPipe() );
	pipe->sendEvent(event);
}

void
eqOgre::Window::createInputHandling( void )
{
	std::string message( "Creating OIS Input system." );
	Ogre::LogManager::getSingleton().logMessage(message);

	std::ostringstream ss;
#if defined OIS_WIN32_PLATFORM
	assert( dynamic_cast<eq::WGLWindowIF *>( getSystemWindow() ) );
	eq::WGLWindowIF *os_win = static_cast<eq::WGLWindowIF *>( getSystemWindow() );
	if( !os_win )
	{
		// TODO add error status
		message = "Couldn't get WGL system window";
		Ogre::LogManager::getSingleton().logMessage(message);
	}
	else
	{
		// It's mandatory to cast HWND to size_t for OIS, otherwise OIS will crash
		ss << (size_t)(os_win->getWGLWindowHandle());
		// Info
		message = "Got window handle for OIS : " + ss.str();
		Ogre::LogManager::getSingleton().logMessage(message);
	}
#elif defined OIS_LINUX_PLATFORM
	// TODO AGL support is missing
	assert( dynamic_cast<eq::GLXWindowIF *>( getSystemWindow() ) );
	eq::GLXWindowIF *os_win = static_cast<eq::GLXWindowIF *>( getSystemWindow() );
	if( !os_win )
	{
		// TODO add error status
		message = "Couldn't get GLX system window";
		Ogre::LogManager::getSingleton().logMessage(message);
	}
	else
	{
		ss << os_win->getXDrawable();
	}
#endif

	OIS::ParamList pl;
	pl.insert(std::make_pair(std::string("WINDOW"), ss.str()));

	// Info
	message = "Creating OIS Input Manager";
	Ogre::LogManager::getSingleton().logMessage(message);

	_input_manager = OIS::InputManager::createInputSystem( pl );
	message = "OIS Input Manager created";
	Ogre::LogManager::getSingleton().logMessage(message);

	printInputInformation();

	_keyboard = static_cast<OIS::Keyboard*>(_input_manager->createInputObject(OIS::OISKeyboard, true));
	_keyboard->setEventCallback(this);

	_mouse = static_cast<OIS::Mouse*>(_input_manager->createInputObject(OIS::OISMouse, true));

	_mouse ->getMouseState().height = getPixelViewport().h;
	_mouse ->getMouseState().width	= getPixelViewport().w;

	_mouse->setEventCallback(this);

	// Info
	message = "Input system created.";
	Ogre::LogManager::getSingleton().logMessage(message);
}

void
eqOgre::Window::printInputInformation( void )
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

	Ogre::LogManager::getSingleton().logMessage( ss.str() );

	ss.str("");
	// List all devices
	// TODO should go to Ogre Log file
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ ss << "\n\tDevice: " << " Vendor: " << i->second; }
	Ogre::LogManager::getSingleton().logMessage( ss.str() );
}

void
eqOgre::Window::createOgreWindow( void )
{
	// Info
	std::string message = "Creating Ogre RenderWindow.";
	Ogre::LogManager::getSingleton().logMessage( message );

	Ogre::NameValuePairList params;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	eq::WGLWindow *os_win = (eq::WGLWindow *)( getSystemWindow() );
	std::stringstream ss( std::stringstream::in | std::stringstream::out );
	ss << os_win->getWGLWindowHandle();
	params["externalWindowHandle"] = ss.str();
	ss.str("");
	params["externalGLControl"] = std::string("True");
	ss << os_win->getWGLContext();
	params["externalGLContext"] = ss.str();
#else
	params["currentGLContext"] = std::string("True");
#endif
	_ogre_window = getOgreRoot()->createWindow( "win", 800, 600, params );
}
