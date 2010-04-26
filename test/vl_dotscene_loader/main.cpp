
#include <eq/eq.h>

#include "eq_ogre/ogre_root.hpp"

class RenderWindow : public eq::Window
{
public :
	RenderWindow( eq::Pipe *parent )
		: eq::Window( parent ), root(), win(), cam(), man(), feet(), robot()
	{}

	virtual bool configInit( const uint32_t initID )
	{
		if( !eq::Window::configInit( initID ) )
		{ return false; }

		root.reset( new vl::ogre::Root );
		// Initialise ogre
		root->createRenderSystem();

		vl::NamedValuePairList params;
		params["currentGLContext"] = std::string("True");
		eq::GLXWindow *oswin = dynamic_cast<eq::GLXWindow *>( getOSWindow() );
		EQASSERT( oswin );
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss <<  oswin->getXDrawable();
		params["parentWindowHandle"] = ss.str();
		win = boost::dynamic_pointer_cast<vl::ogre::RenderWindow>(
				root->createWindow( "win", 800, 600, params ) );

		man = root->createSceneManager("SceneManager");
		cam = man->createCamera("Cam");

		feet = man->getRootNode()->createChild("feet");
		feet->attachObject( cam );
		feet->lookAt( vl::vector(0,0,300) );

		vl::graph::EntityRefPtr ent;
		ent = man->createEntity("robot", "robot.mesh");
		ent->load(man);
		robot = man->getRootNode()->createChild("robot");
		robot->setPosition( vl::vector(0, 0, 300) );
		robot->attachObject( ent );
		
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

	virtual Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }
};

int main( const int argc, char** argv )
{
	std::cout << "arguments = ";
	for(int i = 0; i < argc; i++ )
	{
		std::cout << argv[i] << " ";
	}
	std::cout << std::endl;

    // 1. Equalizer initialization
    ::NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }
    
    // 2. get a configuration
    bool        error  = false;
    eq::Config* config = eq::getConfig( argc, argv );
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

