#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE init_data

#include <boost/test/unit_test.hpp>

#include "eq_cluster/init_data.hpp"

#include "../debug.hpp"
#include "../settings_fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )

std::string const str_data( "data" );

BOOST_AUTO_TEST_CASE( constructors )
{
	eqOgre::InitData data;

	// Default values
	BOOST_CHECK_EQUAL( data.getXMLdata(), (void *)0 );
	BOOST_CHECK_EQUAL( data.size(), 0u );
	BOOST_CHECK( !data.getSettings() );

	// Destroy old instance and create new with some text data
	data.setXMLdata( str_data );

	BOOST_TEST_MESSAGE( "XML data = " << data.getXMLdata() );

	BOOST_CHECK_EQUAL( data.size(), str_data.length() );
	BOOST_CHECK_EQUAL( ::strcmp( data.getXMLdata(), str_data.c_str() ), 0 );
}

BOOST_AUTO_TEST_CASE( copy_default )
{
	eqOgre::InitData data;
	data = eqOgre::InitData();
	
	// Default values
	BOOST_CHECK_EQUAL( data.getXMLdata(), (void *)0 );
	BOOST_CHECK_EQUAL( data.size(), 0u );
}

BOOST_AUTO_TEST_CASE( copy_constructors )
{
	// Create instance with data
	eqOgre::InitData data = eqOgre::InitData();
	data.setXMLdata( str_data );

	// Create other from copy
	eqOgre::InitData other( data );

	BOOST_CHECK_EQUAL( data.size(), str_data.length() );
	BOOST_CHECK_EQUAL( other.size(), str_data.length() );
	BOOST_CHECK_EQUAL( ::strcmp( data.getXMLdata(), str_data.c_str() ), 0 );
	BOOST_CHECK_EQUAL( ::strcmp( other.getXMLdata(), str_data.c_str() ), 0 );

	// Destroy both
	other = eqOgre::InitData();
	data = eqOgre::InitData();
}

BOOST_FIXTURE_TEST_SUITE( SettingsCreation, SettingsFixture )

BOOST_AUTO_TEST_CASE( settings_creation )
{
	std::string root_name("data");
	std::string root_path("work");

	rapidxml::xml_node<> *xml_root = createRoot( root_name, root_path );
	config->append_node( xml_root );

	std::string xml_data;
	rapidxml::print(std::back_inserter(xml_data), doc, 0);

	eqOgre::InitData data;
	data.setXMLdata( xml_data );

	BOOST_TEST_MESSAGE( data.getXMLdata() );
	vl::SettingsRefPtr settings_ptr;
	BOOST_CHECK_NO_THROW( settings_ptr = data.getSettings() );
	BOOST_REQUIRE( settings_ptr );
	BOOST_REQUIRE_EQUAL( settings_ptr->nRoots(), 1u );
//	BOOST_CHECK_EQUAL( settings_ptr->getRoot(0).name, root_name );
//	BOOST_CHECK_EQUAL( settings_ptr->getRoot(0).path.string(), root_path );
}

BOOST_AUTO_TEST_SUITE_END()