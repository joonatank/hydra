#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE ogre_root

#include <boost/test/unit_test.hpp>

/*
My first vrpn client - vrpnHelloWorld
If you want to transform a CAMERA, VIEWPOINT or HMD, instead of an displayed object,
you need to invert the transform, since
vrpn returns the transform sensor to tracker/base/emitter transform.

// NOTE: a vrpn tracker must call user callbacks with tracker data (pos and
//       ori info) which represent the transformation xfSourceFromSensor.
//       This means that the pos info is the position of the origin of
//       the sensor coord sys in the source coord sys space, and the
//       quat represents the orientation of the sensor relative to the
//       source space (ie, its value rotates the source's axes so that
//       they coincide with the sensor's)
*/


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

// eqOgre project includes
#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_render_window.hpp"
#include "math/conversion.hpp"
#include "base/args.hpp"

// Test includes
#include "eq_test_fixture.hpp"
#include "../fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )
/*****************************************************************************
 *

   Callback handler

 *

 *****************************************************************************/
vrpn_TRACKERCB g_trackerData;

void    VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
  //this function gets called when the tracker's POSITION xform is updated

  //you can change what this callback function is called for by changing
  //the type of t in the function prototype above.
  //Options are:
  //   vrpn_TRACKERCB              position
  //   vrpn_TRACKERVELCB           velocity
  //   vrpn_TRACKERACCCB           acceleration
  //   vrpn_TRACKERTRACKER2ROOMCB  tracker2room transform 
  //                                 (comes from local or remote
  //                                  vrpn_Tracker.cfg file)
  //   vrpn_TRACKERUNIT2SENSORCB   unit2sensor transform (see above comment)
  //   vrpn_TRACKERWORKSPACECB     workspace bounding box (extent of tracker)

  // userdata is whatever you passed into the register_change_handler function.
  // vrpn sucks it up and spits it back out at you. It's not used by vrpn internally

	g_trackerData = t;

	/*
 	printf("handle_tracker\tSensor %d is now at (%g,%g,%g)\n", 
		t.sensor, t.pos[0], t.pos[1], t.pos[2]);
  	
	std::cout << "Quaternion = ";
  	for( int i = 0; i < 4; i++ )
  	{ std::cout << t.quat[i] << ", "; }
  	std::cout << std::endl;
	*/
}

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), state(0), ogre_root(), win(),
		man(), feet(), robot()
	{} 

	virtual ~Channel( void )
	{}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Channel::configInit( initID ) );

		// Initialise ogre
		ogre_root.reset( new vl::ogre::Root() );
		ogre_root->createRenderSystem();
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
		
		try {
			win = ogre_root->createWindow( "Win", 800, 600, params );
		}
		catch( Ogre::Exception const &e)
		{
			std::cout << "Exception when creating window: " << e.what() 
				<< std::endl;
			throw;
		}
		
		ogre_root->init();

		// Create Scene Manager
		man = ogre_root->createSceneManager("SceneManager");
		BOOST_REQUIRE( man );

		// Set factories
		man->setSceneNodeFactory( vl::graph::SceneNodeFactoryPtr(
					new vl::ogre::SceneNodeFactory ) );
		man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::ogre::EntityFactory ) );
		man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::ogre::CameraFactory ) );
		
		// Create camera and viewport
		vl::graph::SceneNodeRefPtr root;
		BOOST_REQUIRE_NO_THROW( root = man->getRootNode() );
		BOOST_REQUIRE( cam = man->createCamera( "Cam" ) );
		vl::graph::ViewportRefPtr view = win->addViewport( cam );
		view->setBackgroundColour( vl::colour(1.0, 0.0, 0.0, 0.0) );
		feet = root->createChild( "Feet" );
		feet->lookAt( vl::vector(0,0,3) );
		BOOST_CHECK_NO_THROW( feet->attachObject( cam ) );

		// Create robot Entity
		BOOST_REQUIRE( root );
		boost::shared_ptr<vl::ogre::Entity> ent = boost::dynamic_pointer_cast<vl::ogre::Entity>(
				man->createEntity( "robot", "robot.mesh" ) );
		ent->load();
		robot = root->createChild();
		robot->setPosition( vl::vector(0, 0, 3) );
		robot->scale( 1./100 );
		BOOST_CHECK_NO_THROW( robot->attachObject( ent ) );
		setNearFar( 0.1, 100.0 );

		return true;
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Channel::frameStart( frameID, frameNumber );
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		try {
		eq::Channel::frameDraw( frameID );
	  
		Ogre::Quaternion q( g_trackerData.quat[1], g_trackerData.quat[2], g_trackerData.quat[3], g_trackerData.quat[4]);
		q.normalise();
		Ogre::Vector3 v3( g_trackerData.pos[0], g_trackerData.pos[1], g_trackerData.pos[2]);
		//feet->setPosition( vl::math::convert(v3) );

		//vl::quaternion quat = vl::math::convert(q);
		//quat.normalize();
		//feet->setOrientation( quat );
		
		// Note: real applications would use one tracking device per observer
				
		Ogre::Matrix4 oq ( q );
		//= Ogre::Matrix4::IDENTITY; //
		//oq.setTrans( v3 );
		//Ogre::Vector3 v(0., 1, 0.);
		oq.setTrans( v3 );

	    const eq::ObserverVector& observers = getConfig()->getObservers();
	    for( eq::ObserverVector::const_iterator i = observers.begin();
			i != observers.end(); ++i )
	    {   
			std::cerr << "Head Matrix : " << std::endl
				<< (*i)->getHeadMatrix() << std::endl;
			std::cerr << "Tracker matrix : " << std::endl 
				<< vl::math::convert(oq) << std::endl;
		
			(*i)->setHeadMatrix( vl::math::convert(oq) );
		}
		

		eq::Frustumf frust = getFrustum();
		//std::cout << "Frustrum = " << frust.compute_matrix() << std::endl;
		cam->setProjectionMatrix( frust.compute_matrix() );
		win->update();
		}
		catch( vl::exception const &e )
		{
			std::cerr << "Exception : " << e.what << " in " << e.where << std::endl;
		}
	}

	int state;
	vl::graph::RootRefPtr ogre_root;
	vl::graph::RenderWindowRefPtr win;
	vl::graph::CameraRefPtr cam;
	vl::graph::SceneManagerRefPtr man;
	vl::graph::SceneNodeRefPtr feet;
	vl::graph::SceneNodeRefPtr robot;
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
#ifdef VL_WIN32 && _DEBUG

	_CrtSetDbgFlag( 0 );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );
#endif

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

    for ( int i = 0; i < 2000; i++ )
	{
	  	tkr->mainloop();
	  	mainloop(); 

	  	// Sleep
		eq::base::sleep(1);
	}

	delete tkr;
}

