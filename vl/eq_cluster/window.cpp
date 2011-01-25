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

#ifdef AGL
#include "eq_cluster/aglWindow.hpp"
#endif

#ifdef WGL
#include "eq_cluster/wglWindow.hpp"
#endif

#include "channel.hpp"
#include "pipe.hpp"

namespace {

void copyMouse( eq::Event &sink, OIS::MouseEvent const &src )
{
	// Copy abs
	sink.pointer.x = src.state.X.abs;
	sink.pointer.y = src.state.Y.abs;

	// Copy rel
	sink.pointer.dx = src.state.X.rel;
	sink.pointer.dy = src.state.Y.rel;

	// Copy wheel
	sink.pointer.yAxis = src.state.Z.rel;

	// Copy buttons
	// TODO make a binary copy (casting int to uint will change the bits)
	sink.pointer.buttons = src.state.buttons;
}

}

/// Public
eqOgre::Window::Window(eq::Pipe *parent)
	: eq::Window( parent ), _ogre_window(0),
	_input_manager(0), _keyboard(0), _mouse(0)
{}

eqOgre::Window::~Window(void )
{}


/// Public OIS Callbacks

bool
eqOgre::Window::keyPressed(const OIS::KeyEvent& key)
{
	eq::ConfigEvent event;
	event.data.type = eq::Event::KEY_PRESS;
	event.data.key.key = key.key;

	getConfig()->sendEvent(event);

	return true;
}

bool
eqOgre::Window::keyReleased(const OIS::KeyEvent& key)
{
	eq::ConfigEvent event;
	event.data.type = eq::Event::KEY_RELEASE;
	event.data.key.key = key.key;

	getConfig()->sendEvent(event);

	return true;
}

bool
eqOgre::Window::mouseMoved(const OIS::MouseEvent& evt)
{
	eq::ConfigEvent event;
	event.data.type = eq::Event::POINTER_MOTION;

	copyMouse( event.data, evt );

	getConfig()->sendEvent(event);

	return true;
}

bool
eqOgre::Window::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
	eq::ConfigEvent event;
	event.data.type = eq::Event::POINTER_BUTTON_PRESS;

	copyMouse( event.data, evt );
	event.data.pointerMotion.button = id;

	getConfig()->sendEvent(event);

	return true;
}

bool
eqOgre::Window::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
	eq::ConfigEvent event;
	event.data.type = eq::Event::POINTER_BUTTON_RELEASE;

	copyMouse( event.data, evt );
	event.data.pointerMotion.button = id;

	getConfig()->sendEvent(event);

	return true;
}

eqOgre::DistributedSettings const &
eqOgre::Window::getSettings( void ) const
{
	EQASSERT( dynamic_cast<eqOgre::Pipe const *>( getPipe() ) );
	return static_cast<eqOgre::Pipe const *>(getPipe())->getSettings();
}

vl::Player const &
eqOgre::Window::getPlayer( void ) const
{
	EQASSERT( dynamic_cast<eqOgre::Pipe const *>( getPipe() ) );
	return static_cast<eqOgre::Pipe const *>(getPipe())->getPlayer();
}

vl::ogre::RootRefPtr
eqOgre::Window::getOgreRoot( void )
{
	EQASSERT( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
	return static_cast<eqOgre::Pipe *>(getPipe())->getRoot();
}

void
eqOgre::Window::setCamera(Ogre::Camera* camera)
{
	Channels const &chanlist = getChannels();
	for( size_t i = 0; i < chanlist.size(); ++i )
	{
		EQASSERT( dynamic_cast<eqOgre::Channel *>( chanlist.at(i) ) );
		eqOgre::Channel *channel =
			static_cast<eqOgre::Channel *>( chanlist.at(i) );
		channel->setCamera( camera );
	}
}

Ogre::Camera *
eqOgre::Window::getCamera( void )
{
	EQASSERT( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
	return static_cast<eqOgre::Pipe *>(getPipe())->getCamera();
}

Ogre::SceneManager *
eqOgre::Window::getSceneManager(void )
{
	EQASSERT( dynamic_cast<eqOgre::Pipe *>( getPipe() ) );
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


/// Protected


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
			EQASSERT( _ogre_window && getCamera() );
			Ogre::Viewport *viewport = _ogre_window->addViewport( getCamera() );
			EQASSERT( dynamic_cast<eqOgre::Channel *>( chanlist.at(i) ) );
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
	switch( pipe->getWindowSystem( ))
	{
#ifdef GLX
		case eq::WINDOW_SYSTEM_GLX:
		message = "Using eqOgre::GLXWindow";
		Ogre::LogManager::getSingleton().logMessage(message);
		systemWindow = new eqOgre::GLXWindow( this );
		break;
#endif

#ifdef AGL
		case eq::WINDOW_SYSTEM_AGL:
		message = "Using eqOgre::AGLWindow";
		Ogre::LogManager::getSingleton().logMessage(message);
		systemWindow = new eqOgre::AGLWindow( this );
		break;
#endif

#ifdef WGL
		case eq::WINDOW_SYSTEM_WGL:
			message = "Using eqOgre::WGLWindow";
			Ogre::LogManager::getSingleton().logMessage(message);
			systemWindow = new eqOgre::WGLWindow( this );
		break;
#endif
		default:
			message = "Window system not implemented or supported";
			Ogre::LogManager::getSingleton().logMessage(message);
			return false;
	}

	EQASSERT( systemWindow );
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
eqOgre::Window::createInputHandling( void )
{
	std::string message( "Creating OIS Input system." );
	Ogre::LogManager::getSingleton().logMessage(message);

	std::ostringstream ss;
#if defined OIS_WIN32_PLATFORM
	EQASSERT( dynamic_cast<eq::WGLWindowIF *>( getSystemWindow() ) );
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
	EQASSERT( dynamic_cast<eq::GLXWindowIF *>( getSystemWindow() ) );
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
