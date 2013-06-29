/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file application.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

// Interface
#include "application.hpp"

// Necessary for checking architecture
#include "defines.hpp"

// Necessary for Settings
#include "base/envsettings.hpp"

// Necessary for creating the Logger
#include "logger.hpp"



#include "base/string_utils.hpp"

#ifdef HYDRA_LINUX
// Necessary for fork
#include <unistd.h>
#endif

// Necessaru for Hydra_RUN
#include "master.hpp"
#include "slave.hpp"
// Necessary for hiding system console
#include "base/system_util.hpp"

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

	env->setLogDir( options.getLogDir() );
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

	uint16_t port = DEFAULT_HYDRA_PORT;
	size_t pos = options.server_address.find_last_of(":");
	std::string hostname = options.server_address.substr(0, pos);
	if( pos != std::string::npos )
	{
		port = vl::from_string<uint16_t>( options.server_address.substr(pos+1) );
	}

	vl::config::EnvSettingsRefPtr env( new vl::config::EnvSettings );
	env->setSlave();
	vl::config::Server server(port, hostname);
	env->setServer(server);
	env->getMaster().name = options.slave_name;
	env->display_n = options.display_n;

	return env;
}


void
vl::Hydra_Run(const int argc, char** argv, vl::ExceptionMessage *msg)
{
	std::clog << "Starting Hydra : " << HYDRA_REVISION << std::endl;
	std::string exception_msg;
	vl::ProgramOptions options;
	try
	{
		// Does not set exception message 
		// because returns false for valid configuration
		// when user requested information and not really running the app.
		// If the options are incorrect this will throw instead.
		if(!options.parseOptions(argc, argv))
		{ return; }

		// File doesn't exist (checked earlier)
		if(!options.getLogDir().empty() && !fs::exists(options.getLogDir()))
		{
			fs::create_directory(options.getLogDir());
		}

		// Otherwise the file exists and it's a directory
		std::cout << "Using log file: " << options.getLogFile() << std::endl;

		vl::ApplicationUniquePtr app = Application::create(options);
		// @todo this doesn't allow an exception to be thrown through it
		// we need to use some other method
		// this is true for exception thrown by our GLRenderer

		app->run();
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

	// Only set exception message if we have one
	if(msg)
	{
		if(!exception_msg.empty())
		{
			if(options.master())
			{ msg->title = "Master"; }
			else
			{ msg->title = "Slave"; }
			/// @todo We need a decent name function to options
			msg->title += " " + options.slave_name + " ERROR";

			msg->message = exception_msg;
		}
		// Clear
		else
		{ msg->clear();}
	}
}

/// -------------------------------- Application -----------------------------
vl::Application::Application(void)
	: _logger(0)
{
}

vl::Application::~Application( void )
{
	delete _logger;
}

vl::ApplicationUniquePtr
vl::Application::create(ProgramOptions const &opt)
{
	ApplicationUniquePtr ptr;
	if(opt.master())
	{ ptr.reset(new vl::Master()); }
	else
	{ ptr.reset(new vl::Slave()); }
	
	ptr->init(opt);

	return ptr;
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

	exit();
}

void
vl::Application::init(ProgramOptions const &opt)
{
	vl::config::EnvSettingsRefPtr env;

	if( opt.master() )
	{ env = vl::getMasterSettings(opt); }
	else
	{ env = vl::getSlaveSettings(opt); }

	if( !env )
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	// Needs to be created here because does not currently support
	// changing the log file at run time.
	_logger = new Logger;
	_logger->setOutputFile(opt.getLogFile());

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

	_do_init(env, opt);
}
