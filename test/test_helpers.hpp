#ifndef VL_TEST_HELPERS_HPP
#define VL_TEST_HELPERS_HPP

#include <boost/test/unit_test.hpp>

#include "eq_cluster/init_data.hpp"
#include "settings.hpp"

namespace test = boost::unit_test::framework;

std::string getConfigPath( char const *exe_path )
{
	fs::path cmd( exe_path );

	// Lets find in which directory the test_conf.xml is
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";

	return conf.file_string();
}

std::string getSettingsXML( char const *exe_path )
{
	std::string conf_file = getConfigPath( exe_path );
	if( !fs::exists( conf_file.c_str() ) )
	{
		return std::string();
	}

	// Read file
	std::ifstream stream( conf_file.c_str(), std::ios::binary );
	
	std::stringstream oss;
	oss << stream.rdbuf();

	return oss.str();
}

eqOgre::InitData getInitData( char const *exe_path, std::string const &project_name )
{
	std::string xml_data = getSettingsXML( exe_path );
	eqOgre::InitData init_data;
	init_data.setXMLdata( xml_data );
	init_data.setName( project_name );
	init_data.getSettings()->setExePath( exe_path );
	init_data.getSettings()->setLogDir( "logs" );

	return init_data;
}

/// Get Settings Structure from exe path and project name
/// Returns NULL if no config found
vl::SettingsRefPtr getSettings( char const *exe_path, std::string const &project_name )
{
	/*
	fs::path cmd( exe_path );

	// Lets find in which directory the test_conf.xml is
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";
	if( !fs::exists( conf ) )
	{
		return vl::SettingsRefPtr();
	}
	*/

	std::string xml_data = getSettingsXML( exe_path );
	// If no config provided return NULL
	if( xml_data.empty() )
	{ return vl::SettingsRefPtr(); }

	vl::SettingsRefPtr settings( new vl::Settings( project_name ) );

	// Read data
	vl::SettingsSerializer ser(settings);
	ser.readData( xml_data );

	// Set additional settings
	settings->setExePath( exe_path );
	settings->setLogDir( "logs" );

	return settings;
}

#endif