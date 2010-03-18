#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ogre_render

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"

struct OgreFixture
{
	OgreFixture( void )
		: ogre_root( 0 )
	{
		ogre_root = new vl::ogre::Root();
		ogre_root->createRenderSystem();
		win = ogre_root->createWindow( "Win", 800, 600 );
		ogre_root->init();

		vl::graph::SceneManager *man = ogre_root->createSceneManager("Manager");
		
		vl::graph::Camera *cam = man->createCamera( "Cam" );
		win->addViewport( cam )->setBackgroundColour(
				vmml::vector<4, double>(1.0, 0.0, 0.0, 0.0) );
		vl::graph::MovableObject *ent = man->createEntity("robot", "robot.mesh");
		((vl::ogre::Entity *)ent)->load(man);
		man->getRootNode()->attachObject(ent);
		vl::graph::SceneNode *feet = man->getRootNode()->createChild();
		feet->setPosition( vl::vector(0, 10, 500) );
		feet->attachObject( cam );
	}

	void mainloop( void )
	{
		win->update();
		win->swapBuffers();
	}

	~OgreFixture( void )
	{
		delete ogre_root;
	}

	vl::ogre::Root *ogre_root;
	vl::graph::RenderWindow *win;
		
};

BOOST_FIXTURE_TEST_CASE( render_test, OgreFixture )
{
	for( size_t i = 0; i < 4000; i++ )
	{ mainloop(); }
}
