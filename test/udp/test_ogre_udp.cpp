/*	Joonatan Kuosa
 *	2010-08
 *	Test program that contains both udp::Server and udp::Client
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE test_ogre_udp

#include <boost/test/unit_test.hpp>

// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/client.hpp"
#include "udp/ogre_command.hpp"
#include "eq_ogre/ogre_root.hpp"
#include "base/exceptions.hpp"

// Test includes
#include "udp_fixtures.hpp"
#include "../debug.hpp"
#include "../test_helpers.hpp"

double const TOLERANCE = 1e-3;

// Fixture that can create multiple scene nodes without anything else
struct OgreDummyFixture
{
	OgreDummyFixture( void )
		: ogre_root(), _window(0)
	{}

	void init( vl::SettingsRefPtr settings )
	{
		// Init ogre
		ogre_root.reset( new vl::ogre::Root( settings ) );
		ogre_root->createRenderSystem();
		_window = ogre_root->createWindow( "Win", 800, 600 );
		BOOST_REQUIRE( _window );
		BOOST_TEST_MESSAGE( "window created" );
		
		ogre_root->init();

		// Load resources
		ogre_root->setupResources();
		ogre_root->loadResources();

		_sm = ogre_root->createSceneManager("Manager");
		BOOST_REQUIRE( _sm );
		BOOST_TEST_MESSAGE( "manager created" );

		Ogre::Camera *cam = _sm->createCamera( "Cam" );
		BOOST_REQUIRE( cam );
		BOOST_TEST_MESSAGE( "camera created" );

		_window->addViewport( cam );
		BOOST_TEST_MESSAGE( "viewport created" );
	}

	Ogre::SceneNode *createSceneNode( std::string const &name, Ogre::Vector3 const &pos )
	{
		Ogre::SceneNode *node = _sm->getRootSceneNode()->createChildSceneNode( name );
		node->setPosition( pos );
		BOOST_TEST_MESSAGE( "scenenode : " << name << " created." );
		_nodes.push_back(node);
		return node;
	}

	boost::shared_ptr<vl::ogre::Root> ogre_root;
	Ogre::RenderWindow *_window;
	Ogre::SceneManager *_sm;
	std::vector<Ogre::SceneNode *> _nodes;
};

struct OgreUDPFixture : public TestUdpFixture, public OgreDummyFixture
{
	void check_quat( Ogre::SceneNode *node, std::vector<double> vec )
	{
		Ogre::Quaternion const & q = node->getOrientation();
		for( size_t i = 0; i < 4; ++i )
		{
			BOOST_CHECK_CLOSE( q[i],  vec.at(i), TOLERANCE );
		}
	}

	// Check angle-axis rotation
	void check_angle_axis( Ogre::SceneNode *node, std::vector<double> vec )
	{
		Ogre::Radian angle;
		Ogre::Vector3 axis;
		node->getOrientation().ToAngleAxis(angle, axis);
		
		BOOST_CHECK_CLOSE( angle.valueRadians(), vec.at(0), TOLERANCE );
		
		for( size_t i = 0; i < 3; ++i )
		{
			BOOST_CHECK_CLOSE( axis[i],  vec.at(i+1), TOLERANCE );
		}
	}

	void check_angle( Ogre::SceneNode *node, Ogre::Vector3 const &axis, double angle )
	{
		Ogre::Radian og_angle;
		Ogre::Vector3 og_axis;
		node->getOrientation().ToAngleAxis(og_angle, og_axis);

		BOOST_CHECK_CLOSE( og_angle.valueRadians(), angle, TOLERANCE );
		for( size_t i = 0; i < 3; ++i )
		{
			BOOST_CHECK_CLOSE( axis[i], og_axis[i], TOLERANCE );
		}
	}
	
	void check_pos( Ogre::SceneNode *node, std::vector<double> vec )
	{
		Ogre::Vector3 const &v = node->getPosition();
		for( size_t i = 0; i < 3; ++i )
		{
			BOOST_CHECK_CLOSE( v[i],  vec.at(i), TOLERANCE );
		}
	}

	void addCommand( vl::udp::CommandRefPtr cmd )
	{
//		std::cerr << *cmd << std::endl;
		server.addCommand( cmd );
	}
};

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_FIXTURE_TEST_SUITE( TestOgreUDP, OgreUDPFixture )

BOOST_AUTO_TEST_CASE( set_axis )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );

	vl::udp::CommandRefPtr cmd = vl::udp::OgreCommand::create("setAngle", node );
	boost::shared_ptr<vl::udp::SetAngle> angle_cmd = boost::dynamic_pointer_cast<vl::udp::SetAngle>(cmd);
	BOOST_REQUIRE( angle_cmd );

	// Check initial values
	Ogre::Vector3 axis = Ogre::Vector3::ZERO;
	Ogre::Vector3 og_axis = angle_cmd->getAxis();
	Ogre::Radian og_angle = angle_cmd->getAngle();
	BOOST_CHECK_CLOSE( 0, og_angle.valueRadians(), TOLERANCE );
	BOOST_CHECK_CLOSE( axis[0], og_axis[0], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[1], og_axis[1], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[2], og_axis[2], TOLERANCE );

	// Rotate around y without angle
	axis = Ogre::Vector3(0, 1, 0);
	angle_cmd->setAxis( axis );
	og_axis = angle_cmd->getAxis();

	BOOST_CHECK_CLOSE( axis[0], og_axis[0], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[1], og_axis[1], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[2], og_axis[2], TOLERANCE );
	
	angle_cmd->getAngleAxis(og_angle, og_axis);
	BOOST_CHECK_CLOSE( Ogre::Radian(0).valueRadians(), og_angle.valueRadians(), TOLERANCE );
	BOOST_CHECK_CLOSE( axis[0], og_axis[0], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[1], og_axis[1], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[2], og_axis[2], TOLERANCE );

	double angle(90/180*M_PI);
	angle_cmd->setAngle( Ogre::Radian(angle) );
	angle_cmd->getAngleAxis(og_angle, og_axis);
	BOOST_CHECK_CLOSE( Ogre::Radian(angle).valueRadians(), og_angle.valueRadians(), TOLERANCE );
	BOOST_CHECK_CLOSE( axis[0], og_axis[0], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[1], og_axis[1], TOLERANCE );
	BOOST_CHECK_CLOSE( axis[2], og_axis[2], TOLERANCE );
}

BOOST_AUTO_TEST_CASE( send_position )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );
	
	// Add command
	addCommand( vl::udp::OgreCommand::create("setPosition", node ) );

	// Send the message
	std::vector<double> msg( msg_pos );

	BOOST_CHECK_NO_THROW( sendMsg( msg ) );

	check_pos( node, msg_pos );
}

BOOST_AUTO_TEST_CASE( send_quaternion )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );

	addCommand( vl::udp::OgreCommand::create("setQuaternion", node ) );

	// Send the message
	std::vector<double> msg;
	add_vec( msg, msg_rot_quat );
	BOOST_CHECK_NO_THROW( sendMsg( msg ) );
	
	check_quat( node, msg_rot_quat );
}

BOOST_AUTO_TEST_CASE( send_angle_axis )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );

	addCommand( vl::udp::OgreCommand::create("setAngleAxis", node ) );

	// Send the message
	std::vector<double> msg;
	add_vec( msg, msg_rot_aa );
	BOOST_CHECK_NO_THROW( sendMsg( msg ) );
	
	// Check the results
	check_angle_axis( node, msg_rot_aa );
}

BOOST_AUTO_TEST_CASE( send_angle )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );

	vl::udp::CommandRefPtr cmd = vl::udp::OgreCommand::create("setAngle", node );
	boost::shared_ptr<vl::udp::SetAngle> angle_cmd = boost::dynamic_pointer_cast<vl::udp::SetAngle>(cmd);
	BOOST_REQUIRE( angle_cmd );
	Ogre::Vector3 axis(0, 1, 0);
	angle_cmd->setAxis( axis );
	addCommand( cmd );

	// Send the message
	std::vector<double> msg;
	double angle( 90/180*M_PI );
	msg.push_back( angle );
	BOOST_CHECK_NO_THROW( sendMsg( msg ) );

	// Check the results
	check_angle( node, axis, angle );
}

BOOST_AUTO_TEST_CASE( error_short_message )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );

	addCommand( vl::udp::OgreCommand::create("setPosition", node ) );

	// Create too short a message
	std::vector<double> msg( msg_pos );
	msg.resize( msg.size()-1 );

	// Check for throwing
	BOOST_CHECK_THROW( sendMsg( msg ), vl::short_message );

	// Check that we are in the initial state
	check_pos( node, std::vector<double>(3) );
}

BOOST_AUTO_TEST_CASE( send_all_messages )
{
	// Get settings
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Create The SceneNode
	// TODO add multiple scene nodes
	Ogre::SceneNode *node = createSceneNode( "robot", Ogre::Vector3::ZERO );

	// TODO add all the commands
	addCommand( vl::udp::OgreCommand::create("setPosition", node ) );

	std::vector<double> msg;
	add_vec( msg, msg_pos );
}

BOOST_AUTO_TEST_SUITE_END()