
#include <eq/eq.h>

#include <eq/base/sleep.h>

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>

#include "eq_ogre/ogre_root.hpp"

#include "settings.hpp"
#include "base/exceptions.hpp"
#include "math/conversion.hpp"
#include "dotscene_loader.hpp"

#include "../fixtures.hpp"

class Config : public eq::Config
{
public :
	Config( eq::base::RefPtr< eq::Server > parent )
		: eq::Config( parent )
	{}

	void setSettings( vl::SettingsRefPtr set )
	{ _settings = set; }

	vl::SettingsRefPtr getSettings( void ) const
	{ return _settings; }

protected :
	vl::SettingsRefPtr _settings;
};

class RenderWindow : public eq::Window
{
public :
	RenderWindow( eq::Pipe *parent )
		: eq::Window( parent ), _root(), _window(0), _camera(0), _sm(0)
	{}

	virtual bool configInit( const uint32_t initID )
	{
		try
		{
			if( !eq::Window::configInit( initID ) )
			{ return false; }

			::Config *config = static_cast< ::Config * >( getConfig() );
			vl::SettingsRefPtr settings = config->getSettings();

			_root.reset( new vl::ogre::Root( settings ) );
			// Initialise ogre
			_root->createRenderSystem();

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
			_window = _root->createWindow( "win", 800, 600, params );

			// Resource registration
			_root->setupResources( );
			_root->loadResources();

			_sm = _root->createSceneManager("SceneManager");

			std::vector<vl::Settings::Scene> const &scenes = settings->getScenes();
			if( scenes.empty() )
			{ return false; }
			
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
		}
		catch( vl::exception const &e )
		{
			std::cerr << "VL exception : " <<  boost::diagnostic_information<>(e)
				<< std::endl;
		}
		catch( Ogre::Exception const &e)
		{
			std::cerr << "Ogre Exception: " << e.what() << std::endl;
		}
		catch( std::exception const &e )
		{
			std::cerr << "STD Exception: " << e.what() << std::endl;
		}

		return true;
	}
	
	virtual void swapBuffers( void )
	{
		eq::Window::swapBuffers();
	}

	Ogre::RenderWindow *getRenderWindow( void )
	{
		return _window;
	}

	Ogre::Camera *getCamera( void )
	{
		return _camera;
	}

	boost::shared_ptr<vl::ogre::Root> _root;
	Ogre::RenderWindow *_window;
	Ogre::Camera *_camera;

	Ogre::SceneManager *_sm;
};

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), _parent((::RenderWindow *)parent)
	{}

	virtual bool configInit( const uint32_t initID )
	{
		if( !eq::Channel::configInit( initID ) )
		{ return false; }

		std::cerr << "Get ogre window from RenderWindow" << std::endl;
		_window = _parent->getRenderWindow();
		EQASSERT( _window );

		std::cerr << "Get camera from RenderWindow" << std::endl;
		_camera = _parent->getCamera();
		EQASSERT( _camera );

		std::cerr << "Creating viewport" << std::endl;
		_viewport = _window->addViewport( _camera );
		_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );

		setNearFar( 0.1, 100.0 );

		return true;
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		// Head tracking support
		Ogre::Vector3 v3 = Ogre::Vector3( 0, 1.5, 0 );

		Ogre::Matrix4 m = Ogre::Matrix4::IDENTITY;
		m.setTrans(v3);

		// Note: real applications would use one tracking device per observer
	    const eq::Observers& observers = getConfig()->getObservers();
	    for( eq::Observers::const_iterator i = observers.begin();
			i != observers.end(); ++i )
	    {
			// When head matrix is set equalizer automatically applies it to the
			// GL Modelview matrix as first transformation
			(*i)->setHeadMatrix( vl::math::convert(m) );
		}

		// From equalizer channel::frameDraw
		EQ_GL_CALL( applyBuffer( ));
	    EQ_GL_CALL( applyViewport( ));
	    
	    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ) );
	    EQ_GL_CALL( glLoadIdentity() );

		EQASSERT( _camera )
		EQASSERT( _window )

		eq::Frustumf frust = getFrustum();
		_camera->setCustomProjectionMatrix( true,
				vl::math::convert( frust.compute_matrix() ) );

		// TODO add support for cameras with parent
		// Ogre::Camera::getPosition returns position relative to the parent
		Ogre::Matrix4 view = Ogre::Math::makeViewMatrix( _camera->getPosition() + v3, _camera->getOrientation() );
		_camera->setCustomViewMatrix( true, view );
		_viewport->update();
	}

	Ogre::Camera *_camera;
	Ogre::Viewport *_viewport;
	Ogre::RenderWindow *_window;
	::RenderWindow *_parent;
};

class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new ::RenderWindow( parent ); }

	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }

	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new ::Config( parent ); }
};

int main( const int argc, char** argv )
{
	InitFixture();
	
	try {
		vl::SettingsRefPtr settings = getSettings(argv[0]);
		if( !settings )
		{
			std::cerr << "No test_conf.xml file found." << std::endl;
			return -1;
		}

		vl::Args &arg = settings->getEqArgs();

		// Redirect logging to file
		std::ofstream log_file( "render_test.log" );
		eq::base::Log::setOutput( log_file );
		
		// 1. Equalizer initialization
		::NodeFactory nodeFactory;
		if( !eq::init( arg.size(), arg.getData(), &nodeFactory ))
		{
			EQERROR << "Equalizer init failed" << std::endl;
			return EXIT_FAILURE;
		}

		// 2. get a configuration
		bool        error  = false;
		::Config* config = static_cast< ::Config * >( eq::getConfig( argc, argv ) );
		if( config )
		{
			config->setSettings( settings );
			// 3. init config
			if( config->init( 0 ))
			{
				// 4. run main loop
				uint32_t spin = 0;
				while( config->isRunning( ))
				{
					config->startFrame( ++spin );
					config->finishFrame();
					eq::base::sleep(1);
				}

				// 5. exit config
				config->exit();
			}
			else
			{
				EQERROR << "Config initialization failed: "
						<< config->getErrorMessage() << std::endl;
				error = true;
			}

			// 6. release config
			eq::releaseConfig( config );
		}
		else
		{
			EQERROR << "Cannot get config" << std::endl;
			error = true;
		}

		// 7. exit
		eq::exit();
		return error ? EXIT_FAILURE : EXIT_SUCCESS;
	}
	catch( vl::exception &e )
	{
		std::cerr << "Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
	}
}

