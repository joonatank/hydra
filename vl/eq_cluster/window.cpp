#include "window.hpp"

#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"
#include "dotscene_loader.hpp"

#include <OGRE/OgreWindowEventUtilities.h>

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


// A test function
void eqOgre::Window::checkX11Events(void )
{
	Display *xDisp = 0;
	eq::GLXWindow *os_win = dynamic_cast<eq::GLXWindow *>( getSystemWindow() );
	if( !os_win )
	{
		EQERROR << "Couldn't get GLX system window" << std::endl;
		return;
	}
	else
	{
		xDisp = os_win->getXDisplay();
	}

//	std::cerr << "Checking X events." << std::endl;
	XEvent event;
    //Poll x11 for events (keyboard and mouse events are caught here)
    while( XPending(xDisp) > 0 )
    {
        XNextEvent(xDisp, &event);

		//Handle Resize events
        if( event.type == ConfigureNotify )
        {
            if( _mouse )
            {
                const OIS::MouseState &ms = _mouse->getMouseState();
                ms.width = event.xconfigure.width;
                ms.height = event.xconfigure.height;
            }
        }
        else if( event.type == KeyPress )
		{
			if( XLookupKeysym( &event.xkey, 0 ) == XK_space )
			{
				std::cout << "X : Space pressed. " << std::endl;
			}
			else if( XLookupKeysym( &event.xkey, 0 ) == XK_Escape )
			{
				std::cout << "X : Escape pressed. Should shutdown. " << std::endl;
			}
			else if( XLookupKeysym( &event.xkey, 0 ) == XK_q )
			{
				std::cout << "X : Q pressed. Should shutdown. " << std::endl;
			}
			else if( XLookupKeysym( &event.xkey, 0 ) == XK_w )
			{
				std::cout << "X : W pressed." << std::endl;
			}
			else
				std::cout << "Key Pressed : key = " << event.xkey.keycode << std::endl;
		}
        else if( event.type == KeyRelease )
		{
			if( XLookupKeysym( &event.xkey, 0 ) == XK_space )
			{
				std::cout << "X : Space released. " << std::endl;
			}
			else if( XLookupKeysym( &event.xkey, 0 ) == XK_Escape )
			{
				std::cout << "X : Escape released. Should shutdown. " << std::endl;
			}
			else if( XLookupKeysym( &event.xkey, 0 ) == XK_q )
			{
				std::cout << "X : Q released. Should shutdown. " << std::endl;
			}
			else if( XLookupKeysym( &event.xkey, 0 ) == XK_w )
			{
				std::cout << "X : W released." << std::endl;
			}
			else
				std::cout << "X : Key Released : key = " << event.xkey.keycode << std::endl;
		}
        else if( event.type == ButtonPress )
		{
			std::cout << "X : Mouse button pressed : button = " << event.xbutton.button << std::endl;
		}
		else if( event.type == ButtonRelease )
		{
			std::cout << "X : Mouse button released : button = " << event.xbutton.button << std::endl;
		}
		else if( event.type == FocusIn )
		{
			std::cout << "Mouse focus IN." << std::endl;
		}
		else if( event.type == FocusOut )
		{
			std::cout << "Mouse focus OUT." << std::endl;
		}
		else if( event.type == MotionNotify )
		{
		}
		else if( event.type == EnterNotify )
		{
			std::cout << "Entered." << std::endl;
		}
		else if( event.type == LeaveNotify )
		{
			std::cout << "Left." << std::endl;
		}
		
        else if( event.type == DestroyNotify )
        {
            std::cout << "Exiting...\n";
        }
        else
            std::cout << "\nUnknown X Event: " << event.type << std::endl;
    }
}

/// Public
eqOgre::Window::Window(eq::Pipe *parent)
	: eq::Window( parent ), _ogre_window(0), _camera(0), _sm(0),
	_input_manager(0), _keyboard(0), _mouse(0)
{}

eqOgre::Window::~Window(void )
{}

bool
eqOgre::Window::loadScene( void )
{
	std::vector<vl::Settings::Scene> const &scenes = _settings->getScenes();
	if( scenes.empty() )
	{ return false; }

	// Clean up old scenes
	_sm->clearScene();
	_sm->destroyAllCameras();

	// TODO support for multiple scene files
	std::string message = "Load scene = " + scenes.at(0).file;
	Ogre::LogManager::getSingleton().logMessage( message );

	DotSceneLoader loader;
	loader.parseDotScene( scenes.at(0).file,
						  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
						  _sm );

	message = "Scene loaded";
	Ogre::LogManager::getSingleton().logMessage( message );

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _sm->getCameraIterator();

	// Grab the first available camera, for now
	if( cameras.hasMoreElements() )
	{
		message = "Camera found from the scene.";
		Ogre::LogManager::getSingleton().logMessage( message );
		_camera = cameras.getNext();
	}
	else
	{
		message = "Creating camera";
		Ogre::LogManager::getSingleton().logMessage( message );
		_camera = _sm->createCamera("Cam");
	}

	return true;
}



/// Public Callbacks
char const *CB_INFO_TEXT = "OIS callback : ";

bool
eqOgre::Window::keyPressed(const OIS::KeyEvent& key)
{
	if( key.key == OIS::KC_ESCAPE || key.key == OIS::KC_Q )
	{
		std::cerr << CB_INFO_TEXT << "Escape or Q pressed. Should quit now. " << std::endl;
	}
	else if( key.key == OIS::KC_W )
	{
		std::cerr << CB_INFO_TEXT << "W pressed. " << std::endl;
	}
	else if( key.key == OIS::KC_SPACE )
	{
		std::cerr << CB_INFO_TEXT << "Space pressed. " << std::endl;
	}
	else
	{
		std::cerr << CB_INFO_TEXT << "Key = " << key.key << " pressed." << std::endl;
	}
	return true;
}

bool
eqOgre::Window::keyReleased(const OIS::KeyEvent& key)
{
	if( key.key == OIS::KC_ESCAPE || key.key == OIS::KC_Q )
	{
		std::cerr << CB_INFO_TEXT << "Escape or Q released. Should quit now. " << std::endl;
		abort();
	}
	else if( key.key == OIS::KC_W )
	{
		std::cerr << CB_INFO_TEXT << "W released. " << std::endl;
	}
	else if( key.key == OIS::KC_SPACE )
	{
		std::cerr << CB_INFO_TEXT << "Space released." << std::endl;
	}
	else
	{
		std::cerr << CB_INFO_TEXT << "Key = " << key.key << " released." << std::endl;
	}
	return true;
}

bool
eqOgre::Window::mouseMoved(const OIS::MouseEvent& evt)
{
	std::cerr << "OIS MouseMoved : Mouse = " << std::endl;
	return true;
}

bool
eqOgre::Window::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
	std::cerr << "OIS MousePressed " << std::endl;
	return true;
}

bool
eqOgre::Window::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
	std::cerr << "OIS MouseReleased " << std::endl;
	return true;
}



/// Protected
// ConfigInit can not throw, it must return false on error. CONFIRMED
bool
eqOgre::Window::configInit( const uint32_t initID )
{
	if( !eq::Window::configInit( initID ))
	{
		EQERROR << "eq::Window::configInit failed" << std::endl;
		return false; 
	}

	eqOgre::Config *config = dynamic_cast< eqOgre::Config * >( getConfig() );
	if( !config )
	{
		EQERROR << "config is not type eqOgre::Config" << std::endl;
		return false;
	}
	
	// Get the cluster version of data
	config->mapData( initID );
	_settings = config->getSettings();
	if( !_settings )
	{
		EQERROR << "Config has no settings!" << std::endl;
		return false;
	}
	
	createOgreRoot();
	createOgreWindow();
	createInputHandling();
	createTracker();
	
	// Resource registration
	_root->setupResources( );
	_root->loadResources();

	_sm = _root->createSceneManager("SceneManager");

	if( !loadScene() )
	{ return false; }

	std::cerr << "eqOgre::Window::configInit done" << std::endl;
	return true;
}

void
eqOgre::Window::frameFinish(const uint32_t frameID, const uint32_t frameNumber)
{
//	std::cerr << "eqOgre::Window::frameFinish" << std::endl;

	// This gets called every frame though nothing happens

	EQASSERT( _keyboard && _mouse );
//		EQINFO << "Capturing input events from keyboard and mouse." << std::endl;
	_keyboard->capture();
	// Seems to be buffered
	if( !_keyboard->buffered() )
	{
		std::cerr << "Keyboard is NOT buffered" << std::endl;
	}
	// Does not react in anyway what so ever
	if( _keyboard->isKeyDown( OIS::KC_ESCAPE ) )
	{
		std::cerr << "ESC down" << std::endl;
	}

	_mouse->capture();

	checkX11Events();

	eq::Window::frameFinish(frameID, frameNumber);
}


bool
eqOgre::Window::configInitSystemWindow(const uint32_t initID)
{
	const eq::Pipe* pipe = getPipe();
	eq::SystemWindow* systemWindow = 0;

	switch( pipe->getWindowSystem( ))
	{
#ifdef GLX
		case eq::WINDOW_SYSTEM_GLX:
		EQINFO << "Using GLXWindow" << std::endl;
			systemWindow = new eqOgre::GLXWindow( this );
		break;
#endif

#ifdef AGL
		case eq::WINDOW_SYSTEM_AGL:
		EQINFO << "Using AGLWindow" << std::endl;
		systemWindow = new eqOgre::AGLWindow( this );
		break;
#endif

#ifdef WGL
		case eq::WINDOW_SYSTEM_WGL:
		EQINFO << "Using WGLWindow" << std::endl;
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
		EQWARN << "System Window initialization failed: " << getErrorMessage() << std::endl;
		delete systemWindow;
		return false;
	}

	setSystemWindow( systemWindow );
	return true;
}

void
eqOgre::Window::createInputHandling( void )
{
	std::cerr << "Creating OIS Input system" << std::endl;

	void *win_handle = 0;
	std::ostringstream ss;
#if defined OIS_WIN32_PLATFORM
	eq::WGLWindow *os_win = dynamic_cast<eq::WGLWindow *>( getSystemWindow() );
	if( !os_win )
	{
		EQERROR << "Couldn't get WGL system window" << std::endl;
	}
	else
	{
		ss << os_win->getWGLWindowHandle();
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
		Display *xDisp = os_win->getXDisplay();
		XID xWin = os_win->getXDrawable();
		std::cerr << "XDisplay = " << xDisp
			<< " : XDrawable = " << std::hex << xWin
			<< std::endl;
/*
		// Don't receive events in equalizer display
		if( XSelectInput(xDisp, xWin, NoEventMask) == BadWindow )
		{
			std::cerr << "Couldn't set the Equalizer display not to receive events."
				<< std::endl;
			EQASSERT( false );
		}
		else
		{
			std::cerr << "The Equalizer display will NOT receive events."
				<< std::endl;
		}
		*/

		ss << xWin;
		win_handle = (void *)(xWin);
	}
#endif

	OIS::ParamList pl;
//	EQASSERT( win_handle );
//	ss << win_handle;
	std::cerr << "Window handle dec = " << std::dec << (size_t)win_handle << " = " << ss.str() << std::endl;
	pl.insert(std::make_pair(std::string("WINDOW"), ss.str()));

	// Non exclusive input, taken from Ogre Wiki - Using OIS
	// Necessary and Working for Linux
	// TODO test if necessary and working for Windows
	// TODO no support for MAC

/*
#if defined OIS_WIN32_PLATFORM
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
	pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif
*/

	std::cerr << "Creating input manager." << std::endl;
//	_input_manager = OIS::InputManager::createInputSystem( (size_t)(win_handle) );
	_input_manager = OIS::InputManager::createInputSystem( pl );
	EQASSERT( _input_manager );

	//Print debugging information
    unsigned int v = _input_manager->getVersionNumber();
    std::cout << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
        << "\nRelease Name: " << _input_manager->getVersionName()
        << "\nManager: " << _input_manager->inputSystemName()
        << "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
        << "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
        << "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick);

    //List all devices
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ std::cout << "\n\tDevice: " << " Vendor: " << i->second; }
	std::cout << std::endl;


	std::cerr << "Creating keyboard." << std::endl;
	_keyboard = static_cast<OIS::Keyboard*>(_input_manager->createInputObject(OIS::OISKeyboard, true));
	EQASSERT( _keyboard );
	_keyboard->setEventCallback(this);
	
	std::cerr << "Creating mouse." << std::endl;
	_mouse = static_cast<OIS::Mouse*>(_input_manager->createInputObject(OIS::OISMouse, true));
	EQASSERT( _mouse );

	_mouse ->getMouseState().height = getPixelViewport().h;
	_mouse ->getMouseState().width	= getPixelViewport().w;
	std::cerr << "Mouse state : width = " << std::dec << getPixelViewport().w
		<< " : height = " << std::dec << getPixelViewport().h << "." << std::endl;

	_mouse->setEventCallback(this);

	std::cerr << "Input system created." << std::endl;
/*
	std::cerr << "Adding frame listener." << std::endl;
	Ogre::WindowEventUtilities::addWindowEventListener(_ogre_window, this);

	Ogre::Root *root = _root->getNative();
	root->addFrameListener( this );

	std::cerr << "Frame listener added." << std::endl;
	*/
}

void 
eqOgre::Window::createOgreRoot( void )
{
	_root.reset( new vl::ogre::Root( _settings ) );
	// Initialise ogre
	_root->createRenderSystem();
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
	_ogre_window = _root->createWindow( "win", 800, 600, params );
}

void 
eqOgre::Window::createTracker( void )
{
	if( _settings->trackerOn() )
	{
		EQINFO << "Creating VRPN Tracker." << std::endl;
		_tracker.reset( new vl::vrpnTracker( _settings->getTrackerAddress() ) );
	}
	else
	{
		EQINFO << "Creating Fake Tracker." << std::endl;
		_tracker.reset( new vl::FakeTracker( ) );
	}
		
	_tracker->setOrientation( 0, _settings->getTrackerDefaultOrientation() );
	_tracker->setPosition( 0, _settings->getTrackerDefaultPosition() );
	_tracker->init();
}

