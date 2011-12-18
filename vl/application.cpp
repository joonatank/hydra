/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011-10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file application.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Interface
#include "application.hpp"

// Necessary for checking architecture
#include "defines.hpp"
// Necessary for creating the Master
#include "config.hpp"

// Necessary for Settings
#include "base/envsettings.hpp"

// Necessary for vl::msleep
#include "base/sleep.hpp"

//#include "game_manager.hpp"
// Necessary for creating Renderer for slave and master
#include "renderer.hpp"
// Necessary for creating the Logger
#include "logger.hpp"

#include "cluster/client.hpp"
// Necessary for spawning external processes
#include "base/system_util.hpp"

#include "base/string_utils.hpp"

#ifdef HYDRA_LINUX
// Necessary for fork
#include <unistd.h>
#endif

// Compile time created header
#include "revision_defines.hpp"

// Necessary for printing Ogre exceptions
#include <OGRE/OgreException.h>

/// -------------------------------- Global ----------------------------------
vl::config::EnvSettingsRefPtr
vl::getMasterSettings( vl::ProgramOptions const &options )
{
	// Should never happen
	if( options.slave() )
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	fs::path env_path = options.environment_file;
	if( !fs::is_regular(env_path) )
	{
		// Valid names for the environment config:
		// hydra_env.xml, hydra.env
		// paths ${Program_dir}, ${Program_dir}/data, ${Program_dir}/../data
		// ${Program_dir}/config, ${Program_dir}/../config
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		paths.push_back( options.program_directory + "/../data" );
		paths.push_back( options.program_directory + "/../config" );
		paths.push_back( options.program_directory + "/config" );
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

	vl::config::EnvSettingsRefPtr env;

	env.reset( new vl::config::EnvSettings );

	// Read the Environment config
	// if there is no such file we return the default configuration
	if( fs::is_regular(env_path) )
	{
		std::string env_data;
		env_data = vl::readFileToString( env_path.string() );
		// TODO check that the files are correct and we have good settings
		vl::config::EnvSerializer env_ser( env );
		env_ser.readString(env_data);
		env->setFile( env_path.string() );
	}

	env->setLogDir( options.log_dir );
	env->setLogLevel( (vl::config::LogLevel)(options.log_level) );

	env->display_n = options.display_n;

	return env;

}

/* This function is useless and depricated
 but it has quite a few nice default paths which should be incorporated into the GameManager

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
		// ${program_dir}/../data ${program_dir}/../data/global
		// @todo these should have steady file name (single filename)
		// and they should have at the most two valid default paths
		// or they could have a master directory "data" or "config" and recursively
		// search it for all matching files. If more than one report to user conflicts.
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		paths.push_back( options.program_directory + "/../data" );
		paths.push_back( options.program_directory + "/data/global" );
		paths.push_back( options.program_directory + "/../data/global" );
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
*/

vl::config::EnvSettingsRefPtr
vl::getSlaveSettings( vl::ProgramOptions const &options )
{
	if( options.master() )
	{
		return vl::config::EnvSettingsRefPtr();
	}
	if( options.slave_name.empty() || options.server_address.empty() )
	{
		return vl::config::EnvSettingsRefPtr();
	}

	size_t pos = options.server_address.find_last_of(":");
	if( pos == std::string::npos )
	{ return vl::config::EnvSettingsRefPtr(); }
	std::string hostname = options.server_address.substr(0, pos);
	uint16_t port = vl::from_string<uint16_t>( options.server_address.substr(pos+1) );

	vl::config::EnvSettingsRefPtr env( new vl::config::EnvSettings );
	env->setSlave();
	vl::config::Server server(port, hostname);
	env->setServer(server);
	env->getMaster().name = options.slave_name;
	env->display_n = options.display_n;

	return env;
}


vl::ExceptionMessage
vl::Hydra_Run(const int argc, char** argv)
{
	std::clog << "Starting Hydra : " << HYDRA_REVISION << std::endl;
	std::string exception_msg;
	vl::ProgramOptions options;
	try
	{
		if(!options.parseOptions(argc, argv))
		{ return ExceptionMessage(); }

		// File doesn't exist (checked earlier)
		if(!options.log_dir.empty() && !fs::exists(options.log_dir))
		{
			fs::create_directory(options.log_dir);
		}

		// Otherwise the file exists and it's a directory
		std::cout << "Using log file: " << options.getOutputFile() << std::endl;

		vl::Application application(options);

		application.run();
	}
	catch(vl::exception const &e)
	{
		exception_msg = "VL Exception : \n" + boost::diagnostic_information<>(e);
	}
	catch(boost::exception const &e)
	{
		exception_msg = "Boost Exception : \n"+ boost::diagnostic_information<>(e);
	}
	catch(Ogre::Exception const &e)
	{
		exception_msg = "Ogre Exception: \n" + std::string(e.what());
	}
	catch( std::exception const &e )
	{
		exception_msg = "STD Exception: \n" + std::string(e.what()); 
	}
	catch( ... )
	{
		exception_msg = std::string("An exception of unknow type occured.");
	}

	if(!exception_msg.empty())
	{
		std::string title;
		if(options.master())
		{ title = "Master"; }
		else
		{ title = "Slave"; }
		/// @todo We need a decent name function to options
		title += " " + options.slave_name + " ERROR";

		return ExceptionMessage(title, exception_msg);
	}
	else
	{ return ExceptionMessage(); }
}

/// -------------------------------- Application -----------------------------
vl::Application::Application(ProgramOptions const &opt)
{
	vl::config::EnvSettingsRefPtr env;
	if( opt.master() )
	{
		env = vl::getMasterSettings(opt);
	}
	else
	{
		env = vl::getSlaveSettings(opt);
	}

	if( !env )
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	_logger.setOutputFile(opt.getOutputFile());
	_init(env, opt);
}

vl::Application::~Application( void )
{
	for(size_t i = 0; i < _spawned_processes.size(); ++i)
	{
		kill_process(_spawned_processes.at(i));
	}
}

bool
vl::Application::progress(void)
{
	// Necessary to catch exceptions here because our Editor can't handle them
	try {
		if(isRunning())
		{
			_mainloop(false);
		}
	}
	catch(vl::exception const &e)
	{
		std::cout << "VL Exception : \n" << boost::diagnostic_information<>(e) << std::endl;
		return false;
	}
	catch(boost::exception const &e)
	{
		std::cout << "Boost Exception : \n" << boost::diagnostic_information<>(e) << std::endl;
		return false;
	}
	catch(Ogre::Exception const &e)
	{
		std::cout << "Ogre Exception: \n" << e.what() << std::endl;
		return false;
	}
	catch( std::exception const &e )
	{
		std::cout << "STD Exception: \n" << e.what() << std::endl;
		return false;
	}
	catch( ... )
	{
		std::cout << "An exception of unknow type occured." << std::endl;
		return false;
	}

	return isRunning();
}

void
vl::Application::run(void)
{		
	while( isRunning() )
	{
		_mainloop(true);	
	}

	_exit();
}

bool
vl::Application::isRunning(void) const
{
	if(_master)
	{ return _master->isRunning(); }
	else
	{ return _slave_client->isRunning(); }
}

vl::RendererInterface *
vl::Application::getRenderer(void)
{
	if(_master)
	{ return _master->getRenderer(); }
	else
	{ return _slave_client->getRenderer(); }
}

vl::GameManagerPtr
vl::Application::getGameManager(void)
{
	if(_master)
	{ return _master->getGameManager(); }
	
	return 0;
}



/// ------------------------------- Private ------------------------------------
void
vl::Application::_mainloop(bool sleep)
{
	// run main loop
	if(_master)
	{
		vl::chrono timer;

		_master->render();

		if(sleep)
		{
			vl::time sleep_time;
			// Try to get the requested frame rate but avoid division by zero
			// also check that the time used for the frame is less than the max fps so we
			// don't overflow the sleep time
			if( _max_fps > 0 && (vl::time(1.0/_max_fps) > timer.elapsed()) )
			{ sleep_time = vl::time(1.0/_max_fps) - timer.elapsed(); }

			// Force context switching by zero sleep
			vl::sleep(sleep_time);
		}
	}
	else
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
}

void
vl::Application::_exit(void)
{
	if(_master)
	{ _master->exit(); }
}

void
vl::Application::_init(vl::config::EnvSettingsRefPtr env, ProgramOptions const &opt)
{
	assert( env );

	if(!opt.show_system_console)
	{ hide_system_console(); }

	// Set the used processors
	if(opt.n_processors != -1)
	{
#ifndef _WIN32
		std::cout << "Setting number of processors is only supported on Windows." << std::endl;
#else
		HANDLE proc = ::GetCurrentProcess();
		DWORD_PTR current_mask, new_mask, system_mask;
		::GetProcessAffinityMask(proc, &current_mask, &system_mask);

		new_mask = 0;
		// @todo add checking that we don't try to use more processors than is available
		for(size_t i = opt.start_processor; i < opt.start_processor + opt.n_processors; ++i)
		{ new_mask |= 1<<i; }

		if(new_mask > current_mask)
		{ 
			std::cerr << "Couldn't change the processor affinity mask because "
				<< "there is not enough processors for the new mask." << std::endl;
		}
		else
		{
			std::clog << "Changing processor affinity mask from " << std::hex << current_mask << " to " << new_mask << std::endl;
			::SetProcessAffinityMask(proc, new_mask);
		}
#endif
	}

	if(env->isMaster())
	{
		_max_fps = env->getFPS();

		// auto_fork is silently ignored for slaves
		if(opt.auto_fork)
		{
			for(size_t i = 0; i < env->getSlaves().size(); ++i)
			{
#ifndef _WIN32
				pid_t pid = ::fork();
				// Reset the environment file for a slave
				if(pid != 0)
				{
					env->setSlave();
					env->getMaster().name = env->getSlaves().at(i).name;
					// Master needs to continue the loop and create the remaining slaves
					break;
				}
#else
				std::clog << "Windows autoforking local slaves." << std::endl;
				/// @todo use a more general parameter for the program name
				std::vector<std::string> params;
				// Add slave param
				params.push_back("--slave");
				params.push_back(env->getSlaves().at(i).name);
				// Add server param
				params.push_back("--server");
				std::stringstream ss;
				ss << env->getServer().hostname << ":" << env->getServer().port;
				params.push_back(ss.str());
				params.push_back("--log_dir");
				params.push_back(env->getLogDir());
				// Create the process
				uint32_t pid = create_process("hydra.exe", params, true);
				_spawned_processes.push_back(pid);
			}
#endif
		}
	}

	/// Correct name has been set
	std::cout << "vl::Application::Application : name = " << env->getName() << std::endl;

	// We should hand over the Renderer to either client or config
	vl::RendererUniquePtr renderer( new Renderer(env->getName()) );

	if( env->isMaster() )
	{
		_master.reset(new vl::Config(env, _logger, renderer));
		_master->init(opt.global_file, opt.project_file, opt.editor);

		std::vector<vl::config::Program> programs = env->getUsedPrograms();
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
