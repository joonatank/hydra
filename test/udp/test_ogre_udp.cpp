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
#include <base/sleep.hpp>

// Test includes
#include "udp_fixtures.hpp"
#include "../debug.hpp"
#include "../ogre_fixtures.hpp"
#include "../test_helpers.hpp"

double const TOLERANCE = 1e-3;

struct OgreUDPFixture : public TestUdpFixture, public OgreFixture
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
	void check_rot( Ogre::SceneNode *node, std::vector<double> vec )
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

	void check_pos( Ogre::SceneNode *node, std::vector<double> vec )
	{
		Ogre::Vector3 const &v = node->getPosition();
		for( size_t i = 0; i < 3; ++i )
		{
			BOOST_CHECK_CLOSE( v[i],  vec.at(i), TOLERANCE );
		}
	}
};

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_FIXTURE_TEST_SUITE( TestOgreUDP, OgreUDPFixture )

// TODO this should be broken to 4 different test cases one for each different
// type of command
// TODO we should add test for too short and too long packets
BOOST_AUTO_TEST_CASE( sending )
{	
	vl::SettingsRefPtr settings = ::getSettings( test::master_test_suite().argv[0], "test_udp_ogre" );
	init( settings );

	// Some initial checking
	BOOST_REQUIRE( _robot->isInSceneGraph() );
	std::vector<double> v(3);
	v.at(0) = 0;
	v.at(1) = 0;
	v.at(2) = -300;
	std::vector<double> q(4);
	q.at(0) = 1;
	q.at(1) = 0;
	q.at(2) = 0;
	q.at(3) = 0;
	check_pos( _robot, v );
	check_quat( _robot, q );
	BOOST_REQUIRE( _feet->isInSceneGraph() );
	check_pos( _feet, std::vector<double>(3) );
	check_quat( _feet, q );

	// Set commands
	boost::shared_ptr<vl::udp::Command> cmd( vl::udp::OgreCommand::create("setPosition", _robot ) );
	server.addCommand( cmd );
	std::cerr << *cmd << std::endl;
	cmd.reset( vl::udp::OgreCommand::create("setQuaternion", _robot ) );
	std::cerr << *cmd << std::endl;
	server.addCommand( cmd );
	cmd.reset( vl::udp::OgreCommand::create("setAngleAxis", _feet ) );
	std::cerr << *cmd << std::endl;
	server.addCommand( cmd );

	// Send the message
	std::vector<double> msg;
	add_vec( msg, msg_pos );
	add_vec( msg, msg_rot_quat );
	add_vec( msg, msg_rot_aa );

	BOOST_CHECK_NO_THROW( sendMsg( msg ) );

	check_pos( _robot, msg_pos );
	check_quat( _robot, msg_rot_quat );
	check_pos( _feet, std::vector<double>(3) );
	check_rot( _feet, msg_rot_aa );
}

BOOST_AUTO_TEST_SUITE_END()