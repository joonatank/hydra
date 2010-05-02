#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ogre_render

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"

struct OgreFixture
{
	OgreFixture( void )
		: ogre_root( new vl::ogre::Root ), win(), robot(), ent()
	{
		ogre_root->createRenderSystem();
		win = ogre_root->createWindow( "Win", 800, 600 );
		//ogre_root->init();

		vl::graph::SceneManagerRefPtr man = ogre_root->createSceneManager("Manager");
		
		man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::ogre::EntityFactory ) );
		man->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::ogre::CameraFactory ) );
		man->setSceneNodeFactory( vl::graph::SceneNodeFactoryPtr(
					new vl::ogre::SceneNodeFactory ) );

		boost::shared_ptr<vl::ogre::Camera> cam
			= boost::dynamic_pointer_cast<vl::ogre::Camera>(
					man->createCamera( "Cam" ) );
 		((Ogre::Camera *)cam->getNative())->setNearClipDistance(0.1);

		win->addViewport( cam )->setBackgroundColour(
				vl::colour(1.0, 0.0, 0.0, 0.0) );

		ent = boost::dynamic_pointer_cast<vl::ogre::Entity>(
				man->createEntity("robot", "robot.mesh") );
		BOOST_REQUIRE( ent );
		
		BOOST_CHECK_NO_THROW( ent->load(man) );
		robot = boost::dynamic_pointer_cast<vl::ogre::SceneNode>
			( man->getRootNode()->createChild( "RobotNode" ) );
		BOOST_REQUIRE( robot );
		robot->setPosition( vl::vector(0, 0, 300) );
		BOOST_CHECK_NO_THROW( robot->attachObject(ent) );

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
	{ mainloop(); }
}
