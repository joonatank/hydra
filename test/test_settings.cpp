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

	BOOST_CHECK_EQUAL( settings.nRoots(), 2 );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(i);
		BOOST_CHECK_EQUAL( root.path.root_directory(), "/" );
		BOOST_CHECK( fs::exists(root.path) );
		BOOST_TEST_MESSAGE( "root " << i << " path = " << root.path );
	}

	fs::path path = settings.getEqConfigPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(), "1-window.eqc" );

	path = settings.getScene();
	BOOST_CHECK_EQUAL( path.filename(), "test.scene" );
	BOOST_TEST_MESSAGE( "scene = " << path );

	std::vector<fs::path> resource_paths = settings.getOgreResourcePaths();
	BOOST_CHECK_EQUAL( resource_paths.size(), 2 );
	for( size_t i = 0; i < resource_paths.size(); ++i )
	{
		fs::path const &res_path = resource_paths.at(i);
		BOOST_CHECK_EQUAL( res_path.root_directory(), "/" );
		BOOST_CHECK_EQUAL( res_path.filename(), "resources.cfg" );
		BOOST_TEST_MESSAGE( "resource path = " << res_path );
	}

	path = settings.getOgrePluginsPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(),  "plugins.cfg" );
	BOOST_TEST_MESSAGE( "plugin path = "<< path );

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
