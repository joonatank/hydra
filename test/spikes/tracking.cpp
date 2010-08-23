#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE tracking

#include <boost/test/unit_test.hpp>

#ifdef VL_WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vrpn_Tracker.h>

#include <time.h>

#include <eq/eq.h>
#include <eq/base/sleep.h>
#include <limits>
#include <iostream>

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>

// eqOgre project includes
#include "eq_ogre/ogre_root.hpp"

#include "math/conversion.hpp"
#include "base/args.hpp"
#include "base/exceptions.hpp"

// Test includes
#include "eq_test_fixture.hpp"
#include "../fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )

/*****************************************************************************
   Callback handler
 *****************************************************************************/
vrpn_TRACKERCB g_trackerData;

void    VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
	g_trackerData = t;
}

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), _root(), _window(0), _sm(0)
	{} 

	virtual ~Channel( void )
	{}

	virtual bool configInit( const uint32_t initID )
	{
		try {
			BOOST_REQUIRE( eq::Channel::configInit( initID ) );

			// Initialise ogre
			// TODO add setting of plugins file path from main
			vl::SettingsRefPtr settings( new vl::Settings() );
			vl::SettingsSerializer ser( settings );
			ser.readFile( "test_conf.xml" );

			_root.reset( new vl::ogre::Root( settings ) );
			_root->createRenderSystem();
			vl::NamedValuePairList params;
			
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
			Ogre::SceneNode *feet = _sm->createSceneNode("Feet");
			_sm->getRootSceneNode()->addChild(feet);
			BOOST_CHECK_NO_THROW( feet->attachObject( _camera ) );

			// Create robot Entity
			std::cerr << "Creating Ogre Entity" << std::endl;
			Ogre::Entity *ent = _sm->createEntity( "robot", "robot.mesh" );
			std::cerr << "Ogre Entity created" << std::endl;
			
			Ogre::SceneNode *robot = _sm->createSceneNode("robotNode" );
			_sm->getRootSceneNode()->addChild(robot);
			robot->setPosition( Ogre::Vector3(0, 0, -0.7) );
			robot->scale( 1./100, 1./100, 1./100 );
			BOOST_CHECK_NO_THROW( robot->attachObject( ent ) );
			std::cerr << "Ogre Entity attached" << std::endl;
			setNearFar( 0.1, 100.0 );
		}
		catch( vl::exception const &e )
		{
			std::cerr << "exception : " <<  boost::diagnostic_information<>(e)
				<< std::endl;
		}
		catch( Ogre::Exception const &e)
		{
			std::cerr << "Exception when creating window: " << e.what() 
				<< std::endl;
		}
		return true;
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Channel::frameStart( frameID, frameNumber );
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		try {
			/*	Old
			eq::Channel::frameDraw( frameID );

			Ogre::Quaternion q = Ogre::Quaternion::IDENTITY;
			Ogre::Vector3 v3 = Ogre::Vector3::ZERO;

			// Quaternion should be about unit length, it's invalid if it's something else
			q = Ogre::Quaternion( g_trackerData.quat[3], g_trackerData.quat[1], 
					g_trackerData.quat[1], g_trackerData.quat[2] );
			//std::cout << "quaternion length = " << q.Norm() << std::endl;
			if( q.Norm() < 0.5 )
			{ q = Ogre::Quaternion::IDENTITY; }
			else
			{ q.normalise(); }
			
			v3 = Ogre::Vector3( -g_trackerData.pos[0], -g_trackerData.pos[1], 
					g_trackerData.pos[2] );

			Ogre::Matrix4 m(q); 
			m.setTrans(v3);

			// Note: real applications would use one tracking device per observer
		    const eq::Observers& observers = getConfig()->getObservers();
		    for( eq::Observers::const_iterator i = observers.begin();
				i != observers.end(); ++i )
		    {
				//std::cerr << "Head Matrix : " << std::endl
				//	<< (*i)->getHeadMatrix() << std::endl;
			
				// When head matrix is set equalizer automatically applies it to the
				// GL Modelview matrix as first transformation
				(*i)->setHeadMatrix( vl::math::convert(m) );
			}

			eq::Frustumf frust = getFrustum();

			cam->setProjectionMatrix( frust.compute_matrix() );
		
			win->update();
			*/

			// New test code
			// Head tracking support
			Ogre::Quaternion q = Ogre::Quaternion::IDENTITY;
			Ogre::Vector3 v3 = Ogre::Vector3::ZERO;

			// Quaternion should be about unit length, it's invalid if it's something else
			// TODO the quaternion is incorrect, test it
			q = Ogre::Quaternion( g_trackerData.quat[3], g_trackerData.quat[0], 
					g_trackerData.quat[1], g_trackerData.quat[2] );

			if( q.Norm() < 0.5 )
			{ q = Ogre::Quaternion::IDENTITY; }
			else
			{ q.normalise(); }
		
			v3 = Ogre::Vector3( g_trackerData.pos[0], g_trackerData.pos[1], 
					-g_trackerData.pos[2] );

			Ogre::Matrix4 m(q); 
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
		    
			if( _camera && _window )
			{
				eq::Frustumf frust = getFrustum();
				_camera->setCustomProjectionMatrix( true, vl::math::convert( frust.compute_matrix() ) );
				Ogre::Matrix4 view = Ogre::Math::makeViewMatrix( v3,  Ogre::Quaternion::IDENTITY );
				_camera->setCustomViewMatrix( true, view );
				_window->update();
			}
		}
		catch( vl::exception const &e )
		{
			std::cerr << "exception : " <<  boost::diagnostic_information<>(e)
				<< std::endl;
		}
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
};

eq::NodeFactory *g_nodeFactory = new ::NodeFactory;

struct RenderFixture
{
	// Init code for this test
	RenderFixture( void )
		: error( false ), frameNumber(0), config(0),
		  log_file( "render_test.log" )
	{
		vl::Args args;
		args.add("stereo_render");
		args.add("--eq-config" );
		args.add("1-window.eqc");

		char **argv = args.getData();

		std::cout << args << std::endl;

		// Redirect logging
		//eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, &nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		// 3. init config
		BOOST_REQUIRE( config->init(0));
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
	eq::Config *config;
	NodeFactory nodeFactory;
	std::ofstream log_file;
};

BOOST_FIXTURE_TEST_CASE( render_test, RenderFixture )
{
	BOOST_REQUIRE( config );

    // Open the tracker
    vrpn_Tracker_Remote *tkr = new vrpn_Tracker_Remote("glasses@130.230.58.16");
	
	// Set up the tracker callback handler
    tkr->register_change_handler(NULL, handle_tracker);

    for ( ; ;)
	{
	  	tkr->mainloop();
	  	mainloop(); 

	  	// Sleep
		eq::base::sleep(1);
	}

	delete tkr;
}

