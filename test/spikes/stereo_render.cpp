/*	Joonatan Kuosa
 *	2010-03
 *
 *	Small test to test rendering with Ogre using the equalizer
 *	rendering loop.
 *	Very small equalizer and Ogre initialization.
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE stereo_render

#include <eq/eq.h>

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_render_window.hpp"

#include "eq_test_fixture.hpp"

char *argv[argc] = { "stereo_render\0", "--eq-config\0", "1-window.eqc\0", "\0" };

class Channel : public eq::Channel
{
public :
	Channel( eq::Window *parent )
		: eq::Channel(parent), state(0), ogre_root(0), win(0),
		man(0), feet(0), robot(0)
	{} 

	virtual ~Channel( void )
	{
		delete ogre_root;
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Channel::configInit( initID ) );

		// Initialise ogre
		ogre_root = new vl::ogre::Root();
		ogre_root->createRenderSystem();
		vl::NamedValuePairList params;
		params["currentGLContext"] = std::string("True");
		win = ogre_root->createWindow( "Win", 800, 600, params );
		//ogre_root->init();

		// Create Scene Manager
		man = ogre_root->createSceneManager("SceneManager");
		BOOST_REQUIRE( man );

		// Create camera and viewport
		vl::graph::SceneNode *root = man->getRootNode();
		cam = man->createCamera( "Cam" );
		vl::graph::Viewport *view = win->addViewport( cam );
		view->setBackgroundColour( vl::colour(1.0, 0.0, 0.0, 0.0) );
		feet = root->createChild();
		feet->lookAt( vl::vector(0,0,300) );
		BOOST_CHECK_NO_THROW( feet->attachObject( cam ) );

		// Create robot Entity
		BOOST_REQUIRE( root );
		vl::ogre::Entity *ent = dynamic_cast<vl::ogre::Entity *>(
				man->createEntity( "robot", "robot.mesh" ) );
		ent->load(man);
		robot = root->createChild();
		robot->setPosition( vl::vector(0, 0, 300) );
		BOOST_CHECK_NO_THROW( robot->attachObject( ent ) );

		setNearFar( 100.0, 100.0e3 );

		return true;
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Channel::frameStart( frameID, frameNumber );

		switch( state )
		{
			case 0 :
			{
				++state;
			}
			break;

			case 1 :
			{
				++state;

			}
			break;

			case 2 :
			{
				++state;
			}
			break;
			case 3 :
			{
				++state;
			}
			break;

			case 4 :
			{
				++state;
			}
			break;

			case 5 :
			{ 
				++state;
			}
			break;

			default :
			break;
		}
	}

	virtual void frameDraw( const uint32_t frameID )
	{
		eq::Frustumf frust = getFrustum();
		cam->setProjectionMatrix( frust.compute_matrix() );
		win->update();
		win->swapBuffers();
	}

	vl::graph::Root *ogre_root;
	vl::graph::RenderWindow *win;
	vl::graph::Camera *cam;
	vl::graph::SceneManager *man;
	vl::graph::SceneNode *feet;
	vl::graph::SceneNode *robot;
	int state;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Channel *createChannel( eq::Window *parent )
	{ return new ::Channel( parent ); }
};

eq::NodeFactory *g_nodeFactory = new ::NodeFactory;

BOOST_FIXTURE_TEST_CASE( render_test, EqFixture )
{
	BOOST_REQUIRE( config );

	for( size_t i = 0; i < 1000; i++ )
	{ mainloop(); }
}
