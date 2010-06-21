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
}

BOOST_AUTO_TEST_CASE( read )
{

}

