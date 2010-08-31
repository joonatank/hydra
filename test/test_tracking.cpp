
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE tracking

#include <boost/test/unit_test.hpp>

#include "vrpn_tracker.hpp"
#include "base/filesystem.hpp"
#include "base/exceptions.hpp"
#include "base/rapidxml_print.hpp"
#include "base/sleep.hpp"

//#include <iostream>

// Test includes
#include "debug.hpp"
#include "tracking_fixture.hpp"

double const TOLERANCE = 1e-3;

const char *TRACKER_NAME = "glasses@localhost";

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_FIXTURE_TEST_SUITE( TrackingUnitTests, TrackerServerFixture )

// TODO needs different values for the sensors
// TODO needs multiple sensors
BOOST_AUTO_TEST_CASE( retrieve_data )
{
	vl::vrpnTracker tracker( TRACKER_NAME );
	tracker.init();
	for( size_t i = 0; i < 10; ++i )
	{
		mainloop();
		tracker.mainloop();
		vl::msleep(1);
	}
	BOOST_REQUIRE_EQUAL( tracker.getNSensors(), 1 );

	Ogre::Vector3 const &pos = tracker.getPosition(0);
	Ogre::Vector3 pos_ref( 0, 1.5, 0 );
	for( size_t i = 0; i < 3; ++i )
	{
		BOOST_CHECK_CLOSE( pos_ref[i], pos[i], TOLERANCE );
	}

	Ogre::Quaternion const &quat = tracker.getOrientation(0);
	Ogre::Quaternion quat_ref( 0.707, 0, 0, 0.707 );
	for( size_t i = 0; i < 4; ++i )
	{
		BOOST_CHECK_CLOSE( quat_ref[i], quat[i], TOLERANCE );
	}
}

// Test mapping to Ogre SceneNode
BOOST_AUTO_TEST_CASE( node_mapping )
{
	// TODO
}

BOOST_AUTO_TEST_SUITE_END()


/*
struct TrackingXMLFixture
{
	TrackingXMLFixture( void )
	{
		
	}

	~TrackingXMLFixture( void )
	{

	}

	rapidxml::xml_node<> *createVRPNServer( void )
	{
	}

	rapidxml::xml_node<> *createHost( std::string const &name )
	{
		
	}

	rapidxml::xml_node<> *createPort( unsigned int port )
	{
		
	}
	
	rapidxml::xml_node<> *createMap( std::string const &typeName,
									 std::string const &name )
	{
		
	}
};


BOOST_FIXTURE_TEST_SUITE( xmlTests, TrackingXMLFixture )

BOOST_AUTO_TEST_CASE( read )
{

}

BOOST_AUTO_TEST_SUITE_END()

*/