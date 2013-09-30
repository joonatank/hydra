/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file program_options.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

// Interface
#include "program_options.hpp"

// Used for error printing
#include <iostream>

// Used for ini file parsing
// because program_options uses Linux config file syntax
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

// Used for command line option parsing
#include <boost/program_options.hpp>

// Compile time created header
// necessary for the version flag
#include "revision_defines.hpp"

#include "base/string_utils.hpp"

namespace po = boost::program_options;

vl::ProgramOptions::ProgramOptions(std::string const &ini_file)
	: verbose(false)
	, log_level(0)
	, display_n(0)
	, n_processors(-1)
	, start_processor(0)
	, _slave(false)
	, _ini_file(ini_file)
	, launcher_port(9556)
	, oculus_rift(false)
{
	/// Find search directories for the ini file
	/// The executable path
	fs::path exe_dir = vl::get_global_path(vl::GP_EXE);
	exe_dir.remove_leaf();

	/// The application directory
	fs::path app_dir = vl::get_global_path(vl::GP_APP_DATA);
	app_dir.remove_leaf();

	/// App dir overrides the exe path
	if( fs::exists(app_dir / _ini_file) )
	{
		_ini_file_path = app_dir / _ini_file;
	}
	else if( fs::exists(exe_dir / _ini_file) )
	{
		_ini_file_path = exe_dir / _ini_file;
	}
	else
	{
		// Not a deal breaker if we can't find ini file
		// we should really create a default ini file to app dir here
		std::clog << "No ini file found." << std::endl;
	}
}

vl::ProgramOptions::~ProgramOptions(void)
{
}

bool
vl::ProgramOptions::master( void ) const
{
	return !_slave;
}

bool
vl::ProgramOptions::slave( void ) const
{
	return _slave;
}

std::string
vl::ProgramOptions::getLogFile(void) const
{
	std::string name;
	if(master())
	{
		name = "master";
	}
	else
	{
		name = "slave";
		if( !slave_name.empty() )
		{ name += ("_" + slave_name); }
	}

	fs::path log = fs::path(getLogDir()) / fs::path(name + ".log");

	return log.string();
}

std::string
vl::ProgramOptions::getLogDir(void) const
{
	if(fs::path(_log_dir_name).is_absolute())
	{ return _log_dir_name; }
	else
	{
		fs::path exe_dir = vl::get_global_path(vl::GP_EXE);
		exe_dir.remove_leaf();
		fs::path ldir = exe_dir / fs::path(_log_dir_name);

		return ldir.string();
	}
}

void
vl::ProgramOptions::parseOptions(void)
{
	if(fs::exists(_ini_file_path))
	{
		_parse_ini();
	}
}

bool
vl::ProgramOptions::parseOptions( int argc, char **argv )
{
	if(fs::exists(_ini_file_path))
	{
		_parse_ini();
	}

	// Declare a group of options that will be 
	// allowed only on command line
	po::options_description cmd_options("Command line options");
	cmd_options.add_options()
		("version,v", "print version string")
		("verbose", "print the output to system console")
		("help,h", "produce help message")
		("slave", po::value< std::string >(), "start a named rendering slave")
		("server", po::value< std::string >(), "master server where to connect to, hostname:port")
		("show_system_console", "Show the system console window on startup.")
		("auto_fork,f", "Auto fork slave processes. Only usefull for virtual clusters.")
//		For now removed
//		("config,c", po::value<std::string>(&_config_file)->default_value("hydra.ini"),
//				"name of a file of a configuration.")
		("log_level,l", po::value<int>(), "how much detail is logged")
		("log_dir", po::value<std::string>(&_log_dir_name), "where to write the log files")
		("environment,e", po::value< std::string >(&environment_file), "environment file")
		("project,p", po::value< std::string >(&project_file), "project file")
		("global,g", po::value< std::string >(&global_file), "global file")
		("display", po::value<int>(&display_n), 
			"Display to use for the window. Only on X11.")
		("processors", po::value<int>(&n_processors), 
			"How many processors or cores the program can use.")
		("start_processor", po::value<int>(&start_processor), 
			"First processor to use only has effect if processor is defined also.")
		("debug_overlay", po::value<bool>(&debug.overlay), "Enable debug overlay.")
	;

	// Parse command line
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
			options(cmd_options).run(), vm);
	po::notify(vm);

	// Print help
	if( vm.count("help") )
	{
		std::cout << "Help : " << cmd_options << std::endl;
		return false;
	}

	if( vm.count("version") )
	{
		// @todo add version information
		std::cout << "Hydra revision : " << HYDRA_REVISION << std::endl;
		return false;
	}

	// Verbose
	if( vm.count("verbose") )
	{
		verbose = true;
	}

	if( vm.count("log_level") )
	{
		log_level = vm["log_level"].as<int>();
	}

	// Some global settings
	fs::path exe_path = fs::system_complete( argv[0] ).string();
	exe_name = exe_path.filename().string();
	program_directory = fs::path(exe_path).parent_path().string();

	if( vm.count("show_system_console") )
	{
		show_system_console = true;
	}

	// Slave options
	if( vm.count("slave") )
	{
		_slave = true;
		slave_name = vm["slave"].as<std::string>();

		// @todo the validity checking should be last
		// maybe even in a different function
		if( slave_name.empty() )
		{
			std::cerr << "A slave without a name is not allowed."
				<< std::endl;
			return false;
		}

		if( vm.count("server") )
		{
			std::string server_str = vm["server"].as<std::string>();
			server_port = DEFAULT_HYDRA_PORT;
			size_t pos = server_str.find_last_of(":");
			server_hostname = server_str.substr(0, pos);
			if( pos != std::string::npos )
			{
				server_port = vl::from_string<uint16_t>( server_str.substr(pos+1) );
			}
		}
	}

	if( vm.count("auto_fork") )
	{
		auto_fork = true;
	}
}

void
vl::ProgramOptions::_parse_ini(void)
{
    // Create an empty property tree object
    using boost::property_tree::ptree;
    ptree pt;

	// Load the INI file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	read_ini(_ini_file_path.string(), pt);

	// Parse ptree
	environment_file = pt.get("environment", "");
	project_file = pt.get("project", "");
	global_file = pt.get("global", "");
	display_n = pt.get("display", 0);
	log_level = pt.get("log.level", 0);
	_log_dir_name = pt.get("log.dir", "");
	n_processors = pt.get("multicore.processors", -1);
	start_processor = pt.get("multicore.start_processor", 0);
	auto_fork = pt.get("multicore.auto_fork", false);
	debug.overlay = pt.get("debug.overlay", false);
	debug.overlay_advanced = pt.get("debug.overlay_advanced", false);
	show_system_console = pt.get("debug.show_system_console", false);
	debug.axes = pt.get("debug.axes", false);
	debug.display = pt.get("debug.display", false);
	launcher_port = pt.get("launcher.port", 9556);
	cad_importer_enabled = pt.get("cad_importer.enabled", false);
	cad_importer_exe = pt.get("cad_importer.exe", "batch_importer.exe");

	if(pt.count("projects") > 0)
	{
		if(!pt.get_child("projects").empty())
		{
			BOOST_FOREACH(ptree::value_type &v, pt.get_child("projects"))
				project_paths.push_back(v.second.data());
		}
	}
}
