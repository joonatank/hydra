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
	EQINFO << "eqOgre::Window::configInit" << std::endl;

	if( !eq::Window::configInit( initID ))
	{
		EQERROR << "eq::Window::configInit failed" << std::endl;
		return false;
	}

	try {
		createOgreWindow();
		createInputHandling();
	}
	catch( vl::exception &e )
	{
		EQERROR << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		return false;
	}
	catch( Ogre::Exception const &e)
	{
		EQERROR << "Ogre Exception: " << e.what() << std::endl;
		return false;
	}
	catch( std::exception const &e )
	{
		EQERROR << "STD Exception: " << e.what() << std::endl;
		return false;
	}
	catch( ... )
	{
		std::string err_msg( "eqOgre::Window::configInit : Exception thrown." );
		EQERROR << err_msg << std::endl;
		return false;
	}

	EQINFO << "eqOgre::Window::init : done" << std::endl;
	return true;
}

bool eqOgre::Window::configExit(void )
{
	// Cleanup children first
	bool retval = eq::Window::configExit();

	// Should clean out OIS and Ogre
	EQINFO << "Cleaning out OIS" << std::endl;
	if( _input_manager )
	{
		EQINFO << "Destroy OIS input manager." << std::endl;
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
		EQINFO << "Create a Ogre Viewport for each Channel" << std::endl;

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
		EQERROR << "Mouse or keyboard does not exists! No input handling."
			<< std::endl;
	}
}

bool
eqOgre::Window::configInitSystemWindow(const eq::uint128_t &initID)
{
	const eq::Pipe* pipe = getPipe();
	eq::SystemWindow* systemWindow = 0;

	switch( pipe->getWindowSystem( ))
	{
#ifdef GLX
		case eq::WINDOW_SYSTEM_GLX:
		EQINFO << "Using eqOgre::GLXWindow" << std::endl;
			systemWindow = new eqOgre::GLXWindow( this );
		break;
#endif

#ifdef AGL
		case eq::WINDOW_SYSTEM_AGL:
		EQINFO << "Using eqOgre::AGLWindow" << std::endl;
		systemWindow = new eqOgre::AGLWindow( this );
		break;
#endif

#ifdef WGL
		case eq::WINDOW_SYSTEM_WGL:
		EQINFO << "Using eqOgre::WGLWindow" << std::endl;
		systemWindow = new eqOgre::WGLWindow( this );
		break;
#endif

	default:
		EQERROR << "Window system " << pipe->getWindowSystem()
			<< " not implemented or supported" << std::endl;
		return false;
	}

	EQASSERT( systemWindow );
	if( !systemWindow->configInit( ))
	{
		EQWARN << "System Window initialization failed: " << std::endl;
		delete systemWindow;
		return false;
	}

	setSystemWindow( systemWindow );
	return true;
}

void
eqOgre::Window::createInputHandling( void )
{
	EQINFO << "Creating OIS Input system." << std::endl;

	std::ostringstream ss;
#if defined OIS_WIN32_PLATFORM
	eq::WGLWindow *os_win = dynamic_cast<eq::WGLWindow *>( getSystemWindow() );
	if( !os_win )
	{
		EQERROR << "Couldn't get WGL system window" << std::endl;
	}
	else
	{
		// It's mandatory to cast HWND to size_t for OIS, otherwise OIS will crash
		ss << (size_t)(os_win->getWGLWindowHandle());
		EQINFO << "Got window handle for OIS : " << ss.str() << std::endl;
	}
#elif defined OIS_LINUX_PLATFORM
	// TODO AGL support is missing
	eq::GLXWindow *os_win = dynamic_cast<eq::GLXWindow *>( getSystemWindow() );
	if( !os_win )
	{
		EQERROR << "Couldn't get GLX system window" << std::endl;
	}
	else
	{
		ss << os_win->getXDrawable();
	}
#endif

	OIS::ParamList pl;
	pl.insert(std::make_pair(std::string("WINDOW"), ss.str()));

	EQINFO << "Creating OIS Input Manager" << std::endl;

	_input_manager = OIS::InputManager::createInputSystem( pl );
	EQINFO << "OIS Input Manager created" << std::endl;

	printInputInformation();

	_keyboard = static_cast<OIS::Keyboard*>(_input_manager->createInputObject(OIS::OISKeyboard, true));
	_keyboard->setEventCallback(this);

	_mouse = static_cast<OIS::Mouse*>(_input_manager->createInputObject(OIS::OISMouse, true));

	_mouse ->getMouseState().height = getPixelViewport().h;
	_mouse ->getMouseState().width	= getPixelViewport().w;

	_mouse->setEventCallback(this);

	EQINFO << "Input system created." << std::endl;
}

void
eqOgre::Window::printInputInformation( void )
{
	// Print debugging information
	// TODO debug information should go to Ogre Log file
	unsigned int v = _input_manager->getVersionNumber();
	EQINFO << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
		<< "\nRelease Name: " << _input_manager->getVersionName()
		<< "\nManager: " << _input_manager->inputSystemName()
		<< "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
		<< "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
		<< "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick)
		<< std::endl;

	// List all devices
	// TODO should go to Ogre Log file
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ EQINFO << "\n\tDevice: " << " Vendor: " << i->second; }
	EQINFO << std::endl;
}

void
eqOgre::Window::createOgreWindow( void )
{
	EQINFO << "Creating Ogre RenderWindow." << std::endl;

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
