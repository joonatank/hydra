#ifndef VL_TEST_FIXTURES_HPP
#define VL_TEST_FIXTURES_HPP

#include <boost/test/unit_test.hpp>

#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreEntity.h>

#include "base/typedefs.hpp"
#include "base/exceptions.hpp"
#include "settings.hpp"
#include "eq_ogre/ogre_root.hpp"

#include <boost/test/unit_test.hpp>

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

fs::path find_conf_path( void )
{
	fs::path cmd( test::master_test_suite().argv[0] );
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";
	return conf;
}

vl::SettingsRefPtr getSettings( char const *exe_path )
{
	fs::path cmd( exe_path );

	// Lets find in which directory the test_conf.xml is
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";
	if( !fs::exists( conf ) )
	{
		return vl::SettingsRefPtr();
	}

	vl::SettingsRefPtr settings( new vl::Settings );
	vl::SettingsSerializer ser(settings);
	ser.readFile( conf.file_string() );

	settings->setExePath( exe_path );

	return settings;
}

#endif
