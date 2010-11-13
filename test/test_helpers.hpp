#ifndef VL_TEST_HELPERS_HPP
#define VL_TEST_HELPERS_HPP

#include <boost/test/unit_test.hpp>

#include <iostream>

#include "eq_cluster/eq_settings.hpp"
#include "base/filestringer.hpp"
#include "arguments.hpp"


namespace test = boost::unit_test::framework;

// TODO this should be moved to lib
eqOgre::SettingsRefPtr getSettings( int argc, char **argv )
{
	// Process command line arguments
	vl::Arguments arguments( argc, argv );

	std::cout << "environment path = " << arguments.env_path << std::endl;
	std::cout << "project path = " << arguments.proj_path << std::endl;
	std::cout << "case name = " << arguments.case_name << std::endl;

	// TODO serializer project and environment settings based on the files
	// TODO add case support

	vl::EnvSettingsRefPtr env( new vl::EnvSettings );
	vl::ProjSettingsRefPtr proj( new vl::ProjSettings );

	// Read the config files to strings
	std::string env_data, proj_data;
	if( fs::exists( arguments.env_path ) )
	{ env_data = vl::readFileToString( arguments.env_path ); }
	else
	{
		std::cerr << "No environment file : "
			<< arguments.env_path << std::endl;
		return eqOgre::SettingsRefPtr();
	}
	if( fs::exists( arguments.proj_path ) )
	{ proj_data = vl::readFileToString( arguments.proj_path ); }
	else
	{
		std::cerr << "No project file : " << arguments.proj_path << std::endl;
		return eqOgre::SettingsRefPtr();
	}

	// TODO check that the files are correct and we have good settings
	vl::EnvSettingsSerializer env_ser( env );
	env_ser.readString(env_data);
	env->setFile( arguments.env_path );

	vl::ProjSettingsSerializer proj_ser( proj );
	proj_ser.readString(proj_data);
	proj->setFile( arguments.proj_path );

	eqOgre::SettingsRefPtr settings( new eqOgre::Settings( env, proj ) );

	// Add the command line arguments
	// TODO this should only add Equalizer arguments
	// or we could implement our own switches and supply the equalizer
	// arguments here based on our own switches.
	settings->setExePath( argv[0] );
	for( int i = 1; i < argc; ++i )
	{
		settings->getEqArgs().add(argv[i] );
	}
	
	return settings;
}

#endif