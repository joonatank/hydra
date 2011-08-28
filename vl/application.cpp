/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file application.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

#include "application.hpp"

#include "config.hpp"

// Necessary for vl::Settings
#include "settings.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"

#include "game_manager.hpp"

#include "renderer.hpp"

#include "logger.hpp"

#include "cluster/client.hpp"
// Necessary for spawning external processes
#include "base/system_util.hpp"

#ifndef _WIN32
// Necessary for fork
#include <unistd.h>
#endif

/// -------------------------------- Global ----------------------------------
vl::EnvSettingsRefPtr
vl::getMasterSettings( vl::ProgramOptions const &options )
{
	if( options.slave() )
	{ return vl::EnvSettingsRefPtr(); }

	fs::path env_path = options.environment_file;
	if( !fs::is_regular(env_path) )
	{
		// Valid names for the environment config:
		// hydra_env.xml, hydra.env
		// paths ${Program_dir}, ${Program_dir}/data
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		for( size_t i = 0; i < paths.size(); ++i )
		{
			if( fs::exists(paths.at(i) / "hydra.env") )
			{
				env_path = paths.at(i) / "hydra.env";
				break;
			}
			else if( fs::exists(paths.at(i) / "hydra_env.xml") )
			{
				env_path = paths.at(i) / "hydra_env.xml";
				break;
			}
		}

	}

	vl::EnvSettingsRefPtr env;

	if( !fs::is_regular(env_path) )
	{
		return env;
	}
	else
	{
		/// This point both env_path and project_path are valid
		env.reset( new vl::EnvSettings );

		/// Read the Environment config
		if( fs::is_regular(env_path) )
		{
			std::string env_data;
			env_data = vl::readFileToString( env_path.string() );
			// TODO check that the files are correct and we have good settings
			vl::EnvSettingsSerializer env_ser( env );
			env_ser.readString(env_data);
			env->setFile( env_path.string() );
		}
	}

	env->setLogDir( options.log_dir );
	env->setLogLevel( (vl::LogLevel)(options.log_level) );

	env->display_n = options.display_n;

	return env;

}

vl::Settings
vl::getProjectSettings( vl::ProgramOptions const &options )
{
	if( options.slave() )
	{ return vl::Settings(); }

	vl::Settings settings;

	/// Read the Project Config
	if( fs::is_regular( options.project_file ) )
	{
		vl::ProjSettings proj;

		std::string proj_data;
		proj_data = vl::readFileToString( options.project_file );
		vl::ProjSettingsRefPtr proj_ptr( &proj, vl::null_deleter() );
		vl::ProjSettingsSerializer proj_ser( proj_ptr );
		proj_ser.readString(proj_data);
		proj.setFile( options.project_file );

		settings.setProjectSettings(proj);
	}

	std::string global_file = options.global_file;
	/// If the user did not set the global, we try to find it
	if( !fs::is_regular(global_file) )
	{
		// Try to find the global file from default paths
		// ${program_dir}/hydra.prj, ${program_dir}/hydra.xml,
		// ${program_dir}/data/hydra.xml, ${program_dir}/data/hydra.prj
		// ${program_dir}/data/global/hydra.xml, ${program_dir}/data/global/hydra.prj
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		paths.push_back( options.program_directory + "/data/global" );
		for( size_t i = 0; i < paths.size(); ++i )
		{
			if( fs::exists(paths.at(i) / "hydra.prj") )
			{
				global_file = fs::path(paths.at(i) / "hydra.prj").string();
				break;
			}
			else if( fs::exists(paths.at(i) / "hydra.xml") )
			{
				global_file = fs::path(paths.at(i) / "hydra.xml").string();
				break;
			}
		}
	}


	/// Read the global config
	if( fs::is_regular(global_file) )
	{
		vl::ProjSettings global;

		std::string global_data;
		global_data = vl::readFileToString( global_file );
		vl::ProjSettingsRefPtr glob_ptr( &global, vl::null_deleter() );
		vl::ProjSettingsSerializer glob_ser( glob_ptr );
		glob_ser.readString(global_data);
		global.setFile( global_file );

		settings.addAuxilarySettings(global);
	}

	return settings;
}

vl::EnvSettingsRefPtr
vl::getSlaveSettings( vl::ProgramOptions const &options )
{
	if( options.master() )
	{
		return vl::EnvSettingsRefPtr();
	}
	if( options.slave_name.empty() || options.server_address.empty() )
	{
		return vl::EnvSettingsRefPtr();
	}

	size_t pos = options.server_address.find_last_of(":");
	if( pos == std::string::npos )
	{ return vl::EnvSettingsRefPtr(); }
	std::string hostname = options.server_address.substr(0, pos);
	uint16_t port = vl::from_string<uint16_t>( options.server_address.substr(pos+1) );

	vl::EnvSettingsRefPtr env( new vl::EnvSettings );
	env->setSlave();
	vl::EnvSettings::Server server(port, hostname);
	env->setServer(server);
	env->getMaster().name = options.slave_name;
	env->display_n = options.display_n;

	return env;
}


/// -------------------------------- Application -----------------------------
vl::Application::Application(vl::EnvSettingsRefPtr env, vl::Settings const &settings, vl::Logger &logger, bool auto_fork, bool show_system_console)
	: _master()
	, _slave_client()
{
	assert( env );

	if(!show_system_console)
	{ hide_system_console(); }

	if(env->isMaster())
	{
		_max_fps = env->getFPS();

		// auto_fork is silently ignored for slaves
		if(auto_fork)
		{
			// @todo should be changed to use create_process instead of forking
			// will work on both Windows and Linux, but is slightly slower in Linux
// Windows forking not supported		
#ifndef _WIN32
			for(size_t i = 0; i < env->getSlaves().size(); ++i)
			{
				pid_t pid = ::fork();
				// Reset the environment file for a slave
				if(pid != 0)
				{
					env->setSlave();
					env->getMaster().name = env->getSlaves().at(i).name;
					// Master needs to continue the loop and create the remaining slaves
					break;
				}
			}
#else
			std::cout << "Trying to autofork local slaves, which is not supported on Windows." 
				<< std::endl;
#endif
		}
	}

	/// Correct name has been set
	std::cout << "vl::Application::Application : name = " << env->getName() << std::endl;

	// We should hand over the Renderer to either client or config
	vl::RendererUniquePtr renderer( new Renderer(env->getName()) );

	if( env->isMaster() )
	{
		_master.reset(new vl::Config( settings, env, logger, renderer ));
		_master->init();
		std::vector<EnvSettings::Program> programs = env->getUsedPrograms();
		std::clog << "Should start " << programs.size() << " autolaunched programs."
			<< std::endl;
		for(size_t i = 0; i < programs.size(); ++i)
		{
			std::clog << "Starting : " << programs.at(i).name 
				<< " with command : " << programs.at(i).command << std::endl;
			uint32_t pid = create_process(programs.at(i).command, programs.at(i).params, programs.at(i).new_console);
			_spawned_processes.push_back(pid);
		}
	}
	else
	{
		char const *host = env->getServer().hostname.c_str();
		uint16_t port = env->getServer().port;
		_slave_client.reset(new vl::cluster::Client(host, port, renderer));
	}
}

vl::Application::~Application( void )
{
	for(size_t i = 0; i < _spawned_processes.size(); ++i)
	{
		kill_process(_spawned_processes.at(i));
	}
}


void
vl::Application::run( void )
{
	if(_master)
	{
		// run main loop
		uint32_t frame = 0;
		while( _master->isRunning() )
		{
			_render( ++frame );
		}

		_master->exit();
	}
	else
	{
		assert(_slave_client);
		while( _slave_client->isRunning() )
		{
			_slave_client->mainloop();

			/// @todo test
			/// Windows can have problems with context switching.
			/// At least this is the case for Windows XP.
			/// Might need a workaround for some or all Windows versions
			/// For now use WIN_ZERO_SLEEP define for testing.
			/// Real solution will need a separate busy-wait while rendering 
			/// and context switching while not.
			///
			/// Linux can not handle busy wait,
			/// much faster with context switching in every iteration.
			vl::msleep(0);
		}

		// TODO add clean exit
	}
}

/// ------------------------------- Private ------------------------------------
void
vl::Application::_render( uint32_t const frame )
{
	vl::timer timer;

	_master->render();

	vl::time sleep_time;
	// Try to get the requested frame rate but avoid division by zero
	// also check that the time used for the frame is less than the max fps so we
	// don't overflow the sleep time
	if( _max_fps > 0 && (vl::time(1.0/_max_fps) > timer.elapsed()) )
	{ sleep_time = vl::time(1.0/_max_fps) - timer.elapsed(); }

	// Force context switching by zero sleep
	vl::sleep(sleep_time);
}
