
#include <eq/client/channel.h>
#include <stdlib.h>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_render_window.hpp"
#include "base/exceptions.hpp"

#include "../fixtures.hpp"

class InitData : public eq::net::Object
{
public :
	std::string const &getSettingsPath( void ) const
	{
		return settings_path;
	}

	void setSettingsPath( std::string const &path )
	{
		settings_path = path;
	}
		
protected :
	virtual void getInstanceData( eq::net::DataOStream& os )
	{
		os << settings_path;
	}
	
	virtual void applyInstanceData( eq::net::DataIStream& is )
	{
		is >> settings_path;
	}

	std::string settings_path;
};

class Config : public eq::Config
{
public :
	Config( eq::base::RefPtr< eq::Server > parent )
		: eq::Config( parent )
	{}

	virtual bool init( void )
	{
		registerObject( &_initData );
		
		return eq::Config::init( _initData.getID() );
	}
	
	virtual bool exit( void )
	{ 
		const bool ret = eq::Config::exit();
		deregisterObject( &_initData );
		return ret;
	}
	
	void setSettings( vl::SettingsRefPtr set )
	{
		EQINFO << "settings setted" << std::endl;
		_initData.setSettingsPath( set->getFilePath().file_string() );
		_settings = set; 
	}

	vl::SettingsRefPtr getSettings( void ) const
	{
		EQINFO << "settings retrieved" << std::endl;
		return _settings;
	}
	
	// TODO serialize settings
	void mapData( uint32_t initDataID )
	{
		if( _initData.getID() == EQ_ID_INVALID )
		{
			EQCHECK( mapObject( &_initData, initDataID ));
			unmapObject( &_initData ); // data was retrieved, unmap immediately
			_settings = ::getSettings( _initData.getSettingsPath().c_str() );
		}
		else  // appNode, _initData is registered already
		{
			EQASSERT( _initData.getID() == initDataID );
		}
	}

protected :
	vl::SettingsRefPtr _settings;
	::InitData _initData;
};

class RenderWindow : public eq::Window
{
public :
	RenderWindow( eq::Pipe *parent )
		: eq::Window( parent ), _root(), _ogre_win(0), _camera(0), _settings()
	{}

	~RenderWindow( void )
	{
		// TODO delete the ogre root
	}
	
	virtual bool configInit( const uint32_t initID )
	{
		try
		{
			if( !eq::Window::configInit( initID ) )
			{ return false; }

			::Config *config = (::Config *)getConfig();
			// Map config data so we have valid settings
			config->mapData(initID);

			_settings = config->getSettings();
			EQASSERT( _settings );

			_root.reset( new vl::ogre::Root( _settings ) );
			
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

			_ogre_win = _root->createWindow( "Win", 800, 600, params );

			_root->init();

			// Setup resources
			_root->setupResources();
			_root->loadResources();
			
			Ogre::SceneManager *sm = _root->createSceneManager("SceneManager");
			EQASSERT( sm );

			EQASSERT( _camera = sm->createCamera("Cam") );

			Ogre::SceneNode *feet = sm->getRootSceneNode()->createChildSceneNode("feet");
			feet->attachObject( _camera );

			Ogre::Entity *ent = sm->createEntity("robot", "robot.mesh");
			EQASSERT( ent );

			Ogre::SceneNode *robot = sm->getRootSceneNode()->createChildSceneNode("robotNode");
			robot->setPosition( Ogre::Vector3(0, 0, -300) );
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
		if( _ogre_win )
		{ _ogre_win->swapBuffers(); }
	}

	Ogre::RenderWindow *getRenderWindow( void )
	{
		return _ogre_win;
	}

	Ogre::Camera *getCamera( void )
	{
		return _camera;
	}

	boost::shared_ptr<vl::ogre::Root> _root;
	Ogre::RenderWindow *_ogre_win;
	Ogre::Camera *_camera;

	vl::SettingsRefPtr _settings;
};

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent) //_window((::RenderWindow *)parent)
	{}

	virtual bool configInit( const uint32_t initID )
	{
		if( !eq::Channel::configInit( initID ) )
		{ return false; }

		::RenderWindow *window = (::RenderWindow *)getWindow();
		_ogre_win = window->getRenderWindow();
		EQASSERT( _ogre_win );

		_camera = window->getCamera();
		EQASSERT( _camera );

		_viewport = _ogre_win->addViewport( _camera );
		_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );

		setNearFar( 100.0, 100.0e3 );

		return true;
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		if( _camera && _ogre_win )
		{
			eq::Frustumf frust = getFrustum();
			Ogre::Matrix4 proj_mat = vl::math::convert( frust.compute_matrix() );
			_camera->setCustomProjectionMatrix( true, proj_mat );
			_viewport->update();
		}
	}

	Ogre::Camera *_camera;
	Ogre::Viewport *_viewport;
	Ogre::RenderWindow *_ogre_win;
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
	EQINFO << "arguments = ";
	for(int i = 0; i < argc; i++ )
	{
		EQINFO << argv[i] << " ";
	}
	EQINFO << std::endl;

	vl::SettingsRefPtr settings = getSettings(argv[0]);
	EQINFO << "settings file arg = " << argv[0] << std::endl;
	EQINFO << "settings file = " << settings->getFilePath() << std::endl;
	
	if( !settings )
	{
		EQERROR << "No test_conf.xml file found." << std::endl;
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
        if( config->init())
        {
            // 4. run main loop
            uint32_t spin = 0;
            while( config->isRunning() )
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

