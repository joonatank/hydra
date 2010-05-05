#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include "settings.hpp"
#include "base/filesystem.hpp"
#include "base/exceptions.hpp"

#include <iostream>

BOOST_AUTO_TEST_CASE( read_from_file )
{
	std::string filename( "test_conf.xml" );
	vl::Settings settings;
	vl::SettingsSerializer ser(&settings);
	BOOST_CHECK_NO_THROW( ser.readFile(filename) );

	fs::path path = settings.getRootPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK( fs::exists(path) );

	path = settings.getEqConfigPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(), "1-window.eqc" );

	path = settings.getScenePath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(), "test.scene" );

	path = settings.getOgreResourcePath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(), "resources.cfg" );

	path = settings.getOgrePluginsPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(),  "plugins.cfg" );

	vl::Args const &arg = settings.getEqArgs();
	BOOST_TEST_MESSAGE( "args = "<< arg );
	BOOST_CHECK_EQUAL( arg.size(), 3 );
	settings.setExePath( "test_settings" );

	BOOST_CHECK_EQUAL( arg.size(), 4 );
	BOOST_TEST_MESSAGE( "args = "<< arg );
	if( arg.size() == 4 )
	{
		BOOST_REQUIRE( arg.at(0) );
		BOOST_TEST_MESSAGE( arg.at(0) );
		BOOST_TEST_MESSAGE( arg.at(1) );
		BOOST_CHECK_EQUAL( std::string(arg.at(0)), "test_settings" );
		BOOST_CHECK_EQUAL( std::string(arg.at(1)), "--eq-config" );
		path = arg.at(2);
		BOOST_CHECK_EQUAL( path.filename(), "1-window.eqc" );
	}
}

BOOST_AUTO_TEST_CASE( missing_read_file )
{
	std::string missing_filename( "conf.xml" );
	std::string invalid_filename( "inv_conf.xml" );
	vl::Settings settings;
	vl::SettingsSerializer ser(&settings);
	BOOST_REQUIRE( fs::exists( invalid_filename ) );
	BOOST_REQUIRE( !fs::exists( missing_filename ) );
	BOOST_CHECK_THROW( ser.readFile( missing_filename ), vl::missing_file );
	BOOST_CHECK_THROW( ser.readFile( invalid_filename ), vl::invalid_file );
}

BOOST_AUTO_TEST_CASE( write_to_file )
{

}
