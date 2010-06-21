
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include "vrpn_tracker.hpp"
#include "base/filesystem.hpp"
#include "base/exceptions.hpp"
#include "base/rapidxml_print.hpp"

#include <iostream>

BOOST_AUTO_TEST_CASE( constructor )
{
	
}

// TODO this needs an embedded vrpn tracker server to be effective test
BOOST_AUTO_TEST_CASE( retrieve_data )
{
	
}

// Test mapping to Ogre SceneNode
BOOST_AUTO_TEST_CASE( node_mapping )
{
	
}

// TODO test mapping to Equalizer Observer, hard to do without initing equalizer
// Might try to use mock observer and just check head matrix settings
BOOST_AUTO_TEST_CASE( observer_mapping )
{
	
}

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
