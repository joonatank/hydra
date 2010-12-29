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
	: eq::Window( parent ), _ogre_window(0), _camera(0), _sm(0),
	_input_manager(0), _keyboard(0), _mouse(0)
{}

eqOgre::Window::~Window(void )
{}

bool
eqOgre::Window::loadScene( void )
{
	// TODO this should be divided to case load scenes function and loadScene function

	// Get scenes
	std::vector<vl::TextResource> scenes = _resource_manager.getSceneResources();
	EQINFO << "Loading Scenes for Project : " << getSettings().getProjectName()
		<< std::endl;

	// If we don't have Scenes there is no point loading them
	if( scenes.empty() )
	{
		EQINFO << "Project does not have any scene files." << std::endl;
		return false;
	}
	else
	{
		EQINFO << "Project has " << scenes.size() << " scene files." << std::endl;
	}

	// Clean up old scenes
	// TODO this should be a loader not a destroyer, move to another function
	_sm->clearScene();
	_sm->destroyAllCameras();

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string const &name = scenes.at(i).getName();

		EQINFO << "Loading scene " << name << "." << std::endl;

		eqOgre::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( scenes.at(i), _sm );

		EQINFO << "Scene " << name << " loaded.";
	}

	/// Get the camera
	// TODO move to separate function

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _sm->getCameraIterator();

	// Grab the first available camera, for now
	if( cameras.hasMoreElements() )
	{
		_camera = cameras.getNext();
		EQINFO << "Using Camera " <<  _camera->getName()
			<< " found from the scene." << std::endl;
	}
	else
	{
		_camera = _sm->createCamera("Cam");
		EQINFO << "No camera in the scene. Using created camera "
			<< _camera->getName() << std::endl;
	}

	return true;
}



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
	return _settings;
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
		EQASSERT( dynamic_cast< eqOgre::Config * >( getConfig() ) );

		EQINFO << "Mapping data." << std::endl;

		// Get the cluster version of data
		if( !getConfig()->mapObject( &_settings, initID ) )
		{
			EQERROR << "Couldn't map Settings." << std::endl;
			return false;
		}
		EQINFO << "Mapping ResourceManager" << std::endl;
		if( !getConfig()->mapObject( &_resource_manager, _settings.getResourceManagerID() ) )
		{
			EQERROR << "Couldn't map ResourceManager." << std::endl;
			return false;
		}
		EQINFO << "Data mapped." << std::endl;

		createOgreRoot();
		createOgreWindow();
		createInputHandling();

		// Resource registration
		std::vector<std::string> const &resources = _resource_manager.getResourcePaths();
		for( size_t i = 0; i < resources.size(); ++i )
		{
			_root->addResource( resources.at(i) );
		}
		_root->setupResources( );
		_root->loadResources();

		_sm = _root->createSceneManager("SceneManager");

		if( !loadScene() )
		{ return false; }
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

	EQINFO << "Cleaning out OGRE" << std::endl;
	if( _root )
	{
		_root.reset();
	}

	EQINFO << "Unmapping Settings." << std::endl;
	getConfig()->unmapObject( &_settings );

	EQINFO << "Unmapping ResourceManager" << std::endl;
	getConfig()->unmapObject( &_resource_manager );

	return retval;
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
		EQERROR << "Mouse or keyboard does not exists! No input handling." << std::endl;
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
eqOgre::Window::createWindowListener(void )
{
	/*	TODO should be added so that we get window events
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
	EQINFO << "Creating Ogre Root" << std::endl;

	_root.reset( new vl::ogre::Root( getSettings().getOgreLogFilePath() ) );
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
