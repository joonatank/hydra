/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Demo executable
 *	Supports loading the settings
 *	Supports listening to a port
 */

// Necessary for vl::exceptions
#include "base/exceptions.hpp"

// Necessary for settings
#include "eq_cluster/eq_settings.hpp"

// Necessary for equalizer client creation
#include "eq_cluster/client.hpp"

#include "program_options.hpp"
#include "base/string_utils.hpp"

#include "settings.hpp"

#include <OGRE/OgreException.h>

vl::EnvSettingsRefPtr getMasterSettings( vl::ProgramOptions options )
{
	if( options.slave() )
	{ return vl::EnvSettingsRefPtr(); }

	if( options.environment_file.empty() )
	{ return vl::EnvSettingsRefPtr(); }

	/// This point both env_path and project_path are valid
	vl::EnvSettingsRefPtr env( new vl::EnvSettings );

	/// Read the Environment config
	if( fs::exists( options.environment_file ) )
	{
		std::string env_data;
		env_data = vl::readFileToString( options.environment_file );
		// TODO check that the files are correct and we have good settings
		vl::EnvSettingsSerializer env_ser( env );
		env_ser.readString(env_data);
		env->setFile( options.environment_file );
	}

	env->setLogDir( options.log_dir );
	env->setExePath( options.exe_path );

	return env;
}

vl::SettingsRefPtr getProjectSettings( vl::ProgramOptions options )
{
	if( options.slave() )
	{
		std::cerr << "Trying to get projects for a slave configuration."
			<< std::endl;
		return vl::SettingsRefPtr();
	}

	vl::SettingsRefPtr settings;

	vl::ProjSettingsRefPtr proj;
	vl::ProjSettingsRefPtr global;

	/// Read the Project Config
	if( fs::is_regular( options.project_file ) )
	{
		std::cout << "Reading project file." << std::endl;
		proj.reset( new vl::ProjSettings );
		std::string proj_data;
		proj_data = vl::readFileToString( options.project_file );
		vl::ProjSettingsSerializer proj_ser( proj );
		proj_ser.readString(proj_data);
		proj->setFile( options.project_file );
	}

	/// Read the global config
	if( fs::is_regular( options.global_file ) )
	{
		std::cout << "Reading global file." << std::endl;
		global.reset( new vl::ProjSettings );
		std::string global_data;
		global_data = vl::readFileToString( options.global_file );
		vl::ProjSettingsSerializer glob_ser( global );
		glob_ser.readString(global_data);
		global->setFile( options.global_file );
	}

	settings.reset( new vl::Settings( proj, global ) );

	return settings;
}

// custom structure is unnecessary for slave configuration because it is small
vl::EnvSettingsRefPtr getSlaveSettings( vl::ProgramOptions options )
{
	if( options.master() )
	{
		std::cout << "Slave configuration with master options?" << std::endl;
		return vl::EnvSettingsRefPtr();
	}
	if( options.slave_name.empty() || options.server_address.empty() )
	{
		std::cout << "Slave configuration without all the parameters." << std::endl;
		return vl::EnvSettingsRefPtr();
	}

	size_t pos = options.server_address.find_last_of(":");
	if( pos == std::string::npos )
	{ return vl::EnvSettingsRefPtr(); }
	std::string hostname = options.server_address.substr(0, pos-1);
	std::cout << "Using server hostname = " << hostname;
	uint16_t port = vl::from_string<uint16_t>( options.server_address.substr(pos) );
	std::cout << " and port = " << port << "." << std::endl;

	vl::EnvSettingsRefPtr env( new vl::EnvSettings );
	env->setSlave();
	vl::EnvSettings::Server server(port, hostname);
	env->setServer(server);
	env->getMaster().name = options.slave_name;

	return env;
}


int main( const int argc, char** argv )
{
	bool error = false;
	try
	{
		vl::ProgramOptions options;
		options.parseOptions(argc, argv);
		vl::EnvSettingsRefPtr env;
		vl::SettingsRefPtr settings;
		if( options.master() )
		{
			std::cout << "Requested master configuration." << std::endl;
			env = getMasterSettings(options);
			settings = getProjectSettings(options);
		}
		else
		{
			std::cout << "Requested slave configuration." << std::endl;
			env = getSlaveSettings(options);
		}

		// 2. initialization of local client node
		if( !env )
		{ return -1; }

		if( env->isMaster() && !settings )
		{ return -1; }

		std::cout << "Creating client" << std::endl;
		eqOgre::Client client( env, settings );
		client.init();

		if( !error )
		{
			error = !client.run();
			if( error )
			{ std::cerr << "Client run returned an error." << std::endl; }
		}
		std::cout << "Client exited" << std::endl;
	}
	catch( vl::exception &e )
	{
		std::cerr << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		error = true;
	}
	catch( Ogre::Exception const &e)
	{
		std::cerr << "Ogre Exception: " << e.what() << std::endl;
		error = true;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
		error = true;
	}
	catch( ... )
	{
		error = true;
	}

	if( error )
	{ std::cerr << "Application exited with an error." << std::endl; }
	else
	{ std::cerr << "Application exited fine." << std::endl; }

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
