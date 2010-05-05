#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ogre_render

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"

#ifdef VL_WIN32 && _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif

struct OgreFixture
{
	OgreFixture( void )
		: ogre_root( new vl::ogre::Root ), win(), robot(), ent()
	{
			
#ifdef VL_WIN32 && _DEBUG

	_CrtSetDbgFlag( 0 );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );
#endif

		ogre_root->createRenderSystem();
		win = ogre_root->createWindow( "Win", 800, 600 );
		//ogre_root->init();
		BOOST_TEST_MESSAGE( "window created" );

		vl::graph::SceneManagerRefPtr man = ogre_root->createSceneManager("Manager");
		BOOST_REQUIRE( man );
		BOOST_TEST_MESSAGE( "manager created" );

		man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::ogre::EntityFactory ) );
		BOOST_TEST_MESSAGE( "entity factory added" );
		man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::ogre::CameraFactory ) );
		BOOST_TEST_MESSAGE( "camera factory added" );
		man->setSceneNodeFactory( vl::graph::SceneNodeFactoryPtr(
					new vl::ogre::SceneNodeFactory ) );
		BOOST_TEST_MESSAGE( "scenenode factory added" );

		vl::graph::CameraRefPtr cam = man->createCamera( "Cam" );
		BOOST_REQUIRE( cam );
		BOOST_TEST_MESSAGE( "camera created" );

		boost::shared_ptr<vl::ogre::Camera> og_cam
			= boost::dynamic_pointer_cast<vl::ogre::Camera>(cam);
		BOOST_REQUIRE( og_cam );
		BOOST_TEST_MESSAGE( "camera is correct type" );

		BOOST_REQUIRE( og_cam->getNative() );
 		((Ogre::Camera *)og_cam->getNative())->setNearClipDistance(0.1);
		

		win->addViewport( cam )->setBackgroundColour(
				vl::colour(1.0, 0.0, 0.0, 0.0) );
		BOOST_REQUIRE( win );
		BOOST_TEST_MESSAGE( "viewport created" );

		ent = boost::dynamic_pointer_cast<vl::ogre::Entity>(
				man->createEntity("robot", "robot.mesh") );
		BOOST_REQUIRE( ent );
		BOOST_TEST_MESSAGE( "entity created" );

		BOOST_CHECK_NO_THROW( ent->load() );
		robot = boost::dynamic_pointer_cast<vl::ogre::SceneNode>
			( man->getRootNode()->createChild( "RobotNode" ) );
		BOOST_REQUIRE( robot );
		robot->setPosition( vl::vector(0, 0, 300) );
		BOOST_CHECK_NO_THROW( robot->attachObject(ent) );
		BOOST_TEST_MESSAGE( "scenenode created" );

		feet = boost::dynamic_pointer_cast<vl::ogre::SceneNode>(
				man->getRootNode()->createChild( "feet" ) );
		BOOST_REQUIRE( feet );
		feet->lookAt( vl::vector(0,0,300) );
		BOOST_CHECK_NO_THROW( feet->attachObject( cam ) );
	}

	void mainloop( void )
	{
		win->update();
		win->swapBuffers();
	}

	~OgreFixture( void )
	{
	}

	boost::shared_ptr<vl::ogre::Root> ogre_root;
	vl::graph::RenderWindowRefPtr win;
	boost::shared_ptr<vl::ogre::SceneNode> robot;
	boost::shared_ptr<vl::ogre::SceneNode> feet;
	boost::shared_ptr<vl::ogre::Entity> ent;
		
};

BOOST_FIXTURE_TEST_CASE( render_test, OgreFixture )
{
	BOOST_CHECK_EQUAL( robot->getNative()->numAttachedObjects(), 1 );
	BOOST_CHECK_EQUAL( feet->getNative()->numAttachedObjects(), 1 );
	BOOST_CHECK( feet->getNative()->isInSceneGraph() );
	BOOST_CHECK( robot->getNative()->isInSceneGraph() );
	BOOST_CHECK( ent->getNative()->isInScene() );

	for( size_t i = 0; i < 4000; i++ )
	{ 
		mainloop();
		eq::base::sleep(1);
	}
}