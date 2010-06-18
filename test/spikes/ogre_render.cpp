#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ogre_render

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "eq_ogre/ogre_scene_manager.hpp"
#include "eq_ogre/ogre_scene_node.hpp"
#include "eq_ogre/ogre_entity.hpp"
#include "base/exceptions.hpp"

// Test helpers
#include "../fixtures.hpp"

#include <eq/base/sleep.h>

BOOST_GLOBAL_FIXTURE( InitFixture )

namespace test = boost::unit_test::framework;

struct OgreFixture
{
	OgreFixture( void )
		: ogre_root(), _window(0), _robot(0), _feet(0), _entity(0)
	{}

	void init( fs::path const &conf )
	{
		try {
			// Get settings from file
			vl::SettingsRefPtr settings( new vl::Settings() );
			BOOST_REQUIRE( fs::exists(conf) );
			vl::SettingsSerializer ser(settings);
			ser.readFile( conf.file_string() );

			// Init ogre
			ogre_root.reset( new vl::ogre::Root( settings ) );
			ogre_root->createRenderSystem();
			_window = ogre_root->createWindow( "Win", 800, 600 );
			ogre_root->init();
			BOOST_TEST_MESSAGE( "window created" );

			// Load resources
			ogre_root->setupResources();
			ogre_root->loadResources();

			Ogre::SceneManager *sm = ogre_root->createSceneManager("Manager");
			BOOST_REQUIRE( sm );
			BOOST_TEST_MESSAGE( "manager created" );
			
			Ogre::Camera *cam = sm->createCamera( "Cam" );
			BOOST_REQUIRE( cam );
			BOOST_TEST_MESSAGE( "camera created" );
			
			cam->setNearClipDistance(0.1);

			BOOST_REQUIRE( _window );
			_window->addViewport( cam )->setBackgroundColour(
					Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
			BOOST_TEST_MESSAGE( "viewport created" );

			_entity = sm->createEntity("robot", "robot.mesh");
			BOOST_REQUIRE( _entity );
			BOOST_TEST_MESSAGE( "entity created" );

			_robot = sm->getRootSceneNode()->createChildSceneNode( "RobotNode" );
			BOOST_REQUIRE( _robot );
			_robot->setPosition( Ogre::Vector3(0, 0, -300) );
			BOOST_CHECK_NO_THROW( _robot->attachObject( _entity ) );
			BOOST_TEST_MESSAGE( "scenenode created" );

			_feet = sm->getRootSceneNode()->createChildSceneNode("feet");
			BOOST_REQUIRE( _feet );
			BOOST_CHECK_NO_THROW( _feet->attachObject( cam ) );
		}
		catch (vl::exception const &e)
		{
			std::cerr << "exception : " <<  boost::diagnostic_information<>(e)
				<< std::endl;
		}
	}

	void mainloop( void )
	{
		_window->update(false);
		_window->swapBuffers();
	}

	~OgreFixture( void )
	{
	}

	boost::shared_ptr<vl::ogre::Root> ogre_root;
	Ogre::RenderWindow *_window;
	Ogre::SceneNode *_robot;
	Ogre::SceneNode *_feet;
	Ogre::Entity *_entity;
};

BOOST_FIXTURE_TEST_CASE( render_test, OgreFixture )
{
	// Lets find in which directory the plugins.cfg is
	fs::path cmd( test::master_test_suite().argv[0] );
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";
	BOOST_REQUIRE( fs::exists( conf ) );
	init( conf );

	BOOST_CHECK_EQUAL( _robot->numAttachedObjects(), 1 );
	BOOST_CHECK_EQUAL( _feet->numAttachedObjects(), 1 );
	BOOST_CHECK( _feet->isInSceneGraph() );
	BOOST_CHECK( _robot->isInSceneGraph() );
	BOOST_CHECK( _entity->isInScene() );

	for( size_t i = 0; i < 4000; i++ )
	{ 
		mainloop();
		eq::base::sleep(1);
	}
}