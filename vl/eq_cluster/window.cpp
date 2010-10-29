#include "window.hpp"

#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"
#include "dotscene_loader.hpp"

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

eqOgre::Window::Window(eq::Pipe *parent)
	: eq::Window( parent ), _ogre_window(0), _camera(0), _sm(0)
{}

eqOgre::Window::~Window(void )
{}

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
	createTracker();
	
	// Resource registration
	_root->setupResources( );
	_root->loadResources();

	_sm = _root->createSceneManager("SceneManager");

	if( !loadScene() )
	{ return false; }

	return true;
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
eqOgre::Window::createOgreRoot( void )
{
	_root.reset( new vl::ogre::Root( _settings ) );
	// Initialise ogre
	_root->createRenderSystem();
}

void 
eqOgre::Window::createOgreWindow( void )
{
	Ogre::NameValuePairList params;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	eq::WGLWindow *os_win = (eq::WGLWindow *)(getOSWindow());
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
		_tracker.reset( new vl::vrpnTracker( _settings->getTrackerAddress() ) );
	}
	else
	{	
		_tracker.reset( new vl::FakeTracker( ) );
	}
		
	_tracker->setOrientation( 0, _settings->getTrackerDefaultOrientation() );
	_tracker->setPosition( 0, _settings->getTrackerDefaultPosition() );
	_tracker->init();
}

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