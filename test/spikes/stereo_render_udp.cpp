/*	Joonatan Kuosa
 *	2010-03
 *
 *	Small test to test rendering with Ogre using the equalizer
 *	rendering loop.
 *	Very small equalizer and Ogre initialization.
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE stereo_render

#include <eq/eq.h>

#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreEntity.h>

// eqOgre project includes
#include "eq_ogre/ogre_root.hpp"
#include "base/args.hpp"
#include "base/exceptions.hpp"

// UDP includes
#include "udp/server.hpp"
#include "udp/ogre_command.hpp"

// Test includes
#include "eq_test_fixture.hpp"
#include "../fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )

class Config : public eq::Config
{
public :
	Config( eq::base::RefPtr< eq::Server > parent )
		: eq::Config( parent )
	{}

	void setSettings( vl::SettingsRefPtr set )
	{ _settings = set; }

	vl::SettingsRefPtr getSettings( void )
	{ return _settings; }

protected :
	vl::SettingsRefPtr _settings;
};

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), _root(), _window(0),
		_sm(0)
	{}

	virtual ~Channel( void )
	{}

	virtual bool configInit( const uint32_t initID )
	{
		try
		{
			BOOST_REQUIRE( eq::Channel::configInit( initID ) );

			// Initialise ogre
			::Config *conf = static_cast< ::Config *>( getConfig() );
			vl::SettingsRefPtr settings = conf->getSettings();
			_root.reset( new vl::ogre::Root( settings ) );
			_root->createRenderSystem();
			Ogre::NameValuePairList params;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			eq::WGLWindow *os_win = (eq::WGLWindow *)(getWindow()->getOSWindow());
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

			_window = _root->createWindow( "Win", 800, 600, params );

			_root->init();

			// Setup resources
			_root->setupResources();
			_root->loadResources();

			// Create Scene Manager
			_sm = _root->createSceneManager("SceneManager");
			BOOST_REQUIRE( _sm );

			// Create camera and viewport
			BOOST_REQUIRE( _camera = _sm->createCamera( "Cam" ) );
			Ogre::Viewport *view = _window->addViewport( _camera );
			view->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
			Ogre::SceneNode *feet = _sm->createSceneNode("feet");
			_sm->getRootSceneNode()->addChild(feet);
			//->getRootSceneNode()->createChild( "Feet" );
			//_camera->lookAt( Ogre::Vector3(0, 0, -0.7) );
			BOOST_CHECK_NO_THROW( feet->attachObject( _camera ) );

			// Create robot Entity
			Ogre::Entity *ent = _sm->createEntity( "robot", "robot.mesh" );

			Ogre::SceneNode *robot = _sm->createSceneNode("robotNode");
			_sm->getRootSceneNode()->addChild(robot);
			robot->setPosition( Ogre::Vector3(0, -0.5, -0.7)  );
			robot->scale( 1./100, 1./100, 1./100 );
			BOOST_CHECK_NO_THROW( robot->attachObject( ent ) );

			setNearFar( 0.1, 100.0 );

			return true;
		}
		catch( Ogre::Exception const &e)
		{
			std::cout << "Exception when creating window: " << e.what()
				<< std::endl;
			throw;
		}
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Channel::frameStart( frameID, frameNumber );
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		eq::Channel::frameDraw( frameID );

		eq::Frustumf frust = getFrustum();
		Ogre::Matrix4 m = vl::math::convert( frust.compute_matrix() );
		_camera->setCustomProjectionMatrix( true, m );
		_window->update();
	}

	boost::shared_ptr<vl::ogre::Root> _root;
	Ogre::RenderWindow *_window;
	Ogre::Camera *_camera;
	Ogre::SceneManager *_sm;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }

	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new ::Config( parent ); }
};

eq::NodeFactory *g_nodeFactory = new ::NodeFactory;

struct RenderFixture
{
	// Init code for this test
	RenderFixture( void )
		: error( false ), frameNumber(0), config(0),
		  log_file( "render_test.log" ), settings( new vl::Settings )
	{}

	void init( fs::path const &conf )
	{
		try {
			//std::string filename( "test_conf.xml" );
			BOOST_REQUIRE( fs::exists(conf) );
			vl::SettingsSerializer ser(settings);
			ser.readFile( conf.file_string() );

			settings->setExePath( "stereo_render" );
			vl::Args &args = settings->getEqArgs();

			// Redirect logging
			//eq::base::Log::setOutput( log_file );

			// 1. Equalizer initialization
			BOOST_REQUIRE(  eq::init( args.size(), args.getData(), &nodeFactory ) );

			// 2. get a configuration
			config = (::Config *)( eq::getConfig( args.size(), args.getData() ) );
			BOOST_REQUIRE( config );
			config->setSettings( settings );

			// 3. init config
			BOOST_REQUIRE( config->init(0) );
		}
		catch( vl::exception &e )
		{
			std::cerr << "exception : " <<  boost::diagnostic_information<>(e)
				<< std::endl;
		}
	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );

		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~RenderFixture( void )
	{
		// 5. exit config
		if( config )
		{ BOOST_CHECK( config->exit() ); }

		// 6. release config
		eq::releaseConfig( config );

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	bool error;
	uint32_t frameNumber;
	::Config *config;
	NodeFactory nodeFactory;
	std::ofstream log_file;
	vl::SettingsRefPtr settings;
};

BOOST_FIXTURE_TEST_CASE( render_test, RenderFixture )
{
	fs::path cmd( test::master_test_suite().argv[0] );
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";
	BOOST_REQUIRE( fs::exists( conf ) );
	init( conf );

	BOOST_REQUIRE( config );

	for( size_t i = 0; i < 1000; i++ )
	{ mainloop(); }
}
