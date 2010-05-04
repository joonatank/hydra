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
	fs::path path = settings.getScenePath();
	BOOST_CHECK_EQUAL( path.file_string(), "test.scene" );
	path = settings.getOgreResourcePath();
	BOOST_CHECK_EQUAL( path.file_string(), "resources.cfg" );
	path = settings.getOgrePluginsPath();
	BOOST_CHECK_EQUAL( path.file_string(),  "plugins.cfg" );

	/*
	vl::Args arg = settings.getEqArgs();

	BOOST_REQUIRE_EQUAL( arg.size, 4 );
	BOOST_CHECK_EQUAL( std::string(arg.argv[0]), "test_conf" );
	BOOST_CHECK_EQUAL( std::string(arg.argv[1]), "--eq-config" );
	path = arg.argv[2];
	BOOST_CHECK_EQUAL( path.file_string(), "1-window.eqc" );
	*/
}

BOOST_AUTO_TEST_CASE( missing_read_file )
{
	std::string missing_filename( "conf.xml" );
	std::string invalid_filename( "inv_conf.xml" );
	vl::Settings settings;
	vl::SettingsSerializer ser(&settings);
	BOOST_CHECK_THROW( ser.readFile( missing_filename ), vl::missing_file );
	BOOST_CHECK_THROW( ser.readFile( invalid_filename ), vl::invalid_file );
}

BOOST_AUTO_TEST_CASE( write_to_file )
{

}
