
#include <eq/eq.h>
#include <stdlib.h>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_render_window.hpp"
#include "base/exceptions.hpp"

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
		: eq::Window( parent ), root(), win(), cam(), man(), feet(), robot(),
		  _settings( )
	{}

	virtual bool configInit( const uint32_t initID )
	{
		try
		{
			if( !eq::Window::configInit( initID ) )
			{ return false; }

			::Config *config = (::Config *)getConfig();
			_settings = config->getSettings();
			EQASSERT( _settings );

			root.reset( new vl::ogre::Root( _settings ) );
			
			// Initialise ogre
			root->createRenderSystem();

			vl::NamedValuePairList params;
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

			win = boost::dynamic_pointer_cast<vl::ogre::RenderWindow>(
					root->createWindow( "Win", 800, 600, params ) );

			root->init();

			// Setup resources
			root->setupResources();
			root->loadResources();
			
			EQASSERT( man = root->createSceneManager("SceneManager") );
			// Set factories
			man->setSceneNodeFactory( vl::graph::SceneNodeFactoryPtr(
						new vl::ogre::SceneNodeFactory ) );
			man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
						new vl::ogre::EntityFactory ) );
			man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
						new vl::ogre::CameraFactory ) );
			
			EQASSERT( cam = man->createCamera("Cam") );
			EQASSERT( man->getRootNode() );

			feet = man->getRootNode()->createChild("feet");
			feet->attachObject( cam );
			feet->lookAt( vl::vector(0,0,300) );

			vl::graph::EntityRefPtr ent;
			ent = man->createEntity("robot", "robot.mesh");
			EQASSERT( ent );

			robot = man->getRootNode()->createChild("robot");
			robot->setPosition( vl::vector(0, 0, 300) );
			robot->attachObject( ent );
		}
		catch( Ogre::Exception const &e)
		{
			// TODO this needs a message box
			std::cout << "Exception when creating window: " << e.what() 
				<< std::endl;
			throw;
		}
		catch( vl::exception &e )
		{
			std::cerr << "exception : " <<  boost::diagnostic_information<>(e)
				<< std::endl;
		}
		
		return true;
	}
	
	virtual void swapBuffers( void )
	{
		eq::Window::swapBuffers();
		if( win )
		{ win->swapBuffers(); }
	}

	boost::shared_ptr<vl::graph::RenderWindow> getRenderWindow( void )
	{
		return win;
	}

	vl::graph::CameraRefPtr getCamera( void )
	{
		return cam;
	}

	boost::shared_ptr<vl::ogre::Root> root;
	boost::shared_ptr<vl::ogre::RenderWindow> win;
	vl::graph::CameraRefPtr cam;

	vl::graph::SceneManagerRefPtr man;
	vl::graph::SceneNodeRefPtr feet;
	vl::graph::SceneNodeRefPtr robot;

	vl::SettingsRefPtr _settings;
};

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), window((::RenderWindow *)parent)
	{}

	virtual bool configInit( const uint32_t initID )
	{
		if( !eq::Channel::configInit( initID ) )
		{ return false; }

		win = window->getRenderWindow();
		EQASSERT( win );

		camera = window->getCamera();
		EQASSERT( camera );

		viewport = win->addViewport( camera );
		viewport->setBackgroundColour( vl::colour(1.0, 0.0, 0.0, 0.0) );

		setNearFar( 100.0, 100.0e3 );

		return true;
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		if( camera && win )
		{
			eq::Frustumf frust = getFrustum();
			camera->setProjectionMatrix( frust.compute_matrix() );
			viewport->update();
		}
	}

	boost::shared_ptr<vl::graph::Camera> camera;
	boost::shared_ptr<vl::graph::Viewport> viewport;
	boost::shared_ptr<vl::graph::RenderWindow> win;
	::RenderWindow *window;
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
	std::cout << "arguments = ";
	for(int i = 0; i < argc; i++ )
	{
		std::cout << argv[i] << " ";
	}
	std::cout << std::endl;

	vl::SettingsRefPtr settings = getSettings(argv[0]);
	if( !settings )
	{
		std::cerr << "No test_conf.xml file found." << std::endl;
		return -1;
	}

	// 1. Equalizer initialization
    ::NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }
    
    // 2. get a configuration
    bool error = false;
    ::Config *config = (::Config *)eq::getConfig( argc, argv );
	config->setSettings( settings );

    if( config )
    {
        // 3. init config
        if( config->init( 0 ))
        {
            // 4. run main loop
            uint32_t spin = 0;
            while( config->isRunning( ))
            {
                config->startFrame( ++spin );
                config->finishFrame();
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

