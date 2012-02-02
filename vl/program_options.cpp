/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011-10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file program_options.hpp
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
#include "program_options.hpp"

// Used for error printing
#include <iostream>

#include "base/filesystem.hpp"

vl::ProgramOptions::ProgramOptions( void )
	: verbose(false)
	, log_level(0)
	, display_n(0)
	, _slave(false)
	, _cmd_options("Command line options")
	, _config("Configuration")
{
	// Declare a group of options that will be 
	// allowed only on command line
	_cmd_options.add_options()
		("version,v", "print version string")
		("verbose", "print the output to system console")
		("help,h", "produce help message")
		("slave", po::value< std::string >(), "start a named rendering slave")
		("server", po::value< std::string >(), "master server where to connect to, hostname:port")
		("show_system_console", "Show the system console window on startup.")
		("auto_fork,f", "Auto fork slave processes. Only usefull for virtual clusters.")
		("config,c", po::value<std::string>(&_config_file)->default_value("hydra.ini"),
				"name of a file of a configuration.")
	;

	// Declare a group of options that will be 
	// allowed both on command line and in config file
	_config.add_options()
		("log_level,l", po::value<int>(), "how much detail is logged")
		("log_dir", po::value<std::string>(&log_dir), "where to write the log files")
		("environment,e", po::value< std::string >(&environment_file), "environment file")
		("project,p", po::value< std::string >(&project_file), "project file")
		("global,g", po::value< std::string >(&global_file), "global file")
		("display", po::value<int>(&display_n)->default_value(0), 
			"Display to use for the window. Only on X11.")
		("editor", po::value<bool>(&editor)->default_value(false), "Enable editor.")
		("processors", po::value<int>(&n_processors)->default_value(-1), 
			"How many processors or cores the program can use.")
		("start_processor", po::value<int>(&start_processor)->default_value(0), 
			"First processor to use only has effect if processor is defined also.")
	;

	_file_options.add_options()
		("auto_fork", po::value<bool>(&auto_fork)->default_value(false),
			"Auto fork slave processes. Only usefull for virtual clusters.")
		("show_system_console", po::value<bool>(&show_system_console)->default_value(false),
			"Show the system console window on startup.")
	;
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
vl::ProgramOptions::getOutputFile(void) const
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

	fs::path log = fs::path(log_dir) / fs::path(name + ".log");

	return log.string();
}

bool
vl::ProgramOptions::parseOptions( int argc, char **argv )
{
	// TODO add control for the log level at least ERROR, INFO, TRACE

	po::options_description visible("Allowed options");
	visible.add(_cmd_options).add(_config);

	// Parse command line
	// @todo this is problematic because we are overriding command-line options 
	// with ini file options when processed in this order
	// command line should of course take precidence
	// but this does not work if we want to allow changing the config file
	// from command line.
	// So we need to process the config file flag from command line first
	// and store the rest of the options, process ini file and then process
	// the rest of the command line.
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
			options(visible).run(), vm);
	po::notify(vm);

	parseIni(_config_file, vm);

	// Print help
	if( vm.count("help") )
	{
		std::cout << "Help : " << visible << std::endl;
		return false;
	}

	if( vm.count("version") )
	{
		std::cout << "Version flag not supported for the moment." << std::endl;
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
		return _parseSlave( vm );
	}
	else
	{
		_slave = false;
		return _parseMaster( vm );
	}
}

bool
vl::ProgramOptions::parseIni(std::string const &file, po::variables_map vm)
{
    po::options_description visible("Allowed options");
    visible.add(_file_options).add(_config);

	/// Config file
	if(fs::exists(file))
	{
		std::ifstream ifs(file.c_str());
	
		if (!ifs)
		{
			std::cout << "Couldn't not open config file: " << file << std::endl;
			return false;
		}
		else
		{
			std::clog << "Parsing config file : " << file << std::endl;
			po::store(po::parse_config_file(ifs, visible), vm);
			po::notify(vm);

			return true;
		}
	}
	else
	{
		std::clog << "Config file : " << file << " does not exist." << std::endl;
	}

	return false;
}

bool
vl::ProgramOptions::_parseSlave( po::variables_map const &vm )
{
	if( vm.count("slave") )
	{
		slave_name = vm["slave"].as<std::string>();
	}

	if( slave_name.empty() )
	{
		std::cerr << "A slave without a name is not allowed."
			<< std::endl;
		return false;
	}

	if( vm.count("server") )
	{
		server_address = vm["server"].as<std::string>();
	}

	if( server_address.empty() )
	{
		std::cerr << "A slave without a server address to connect to is not allowed."
			<< std::endl;
		return false;
	}

	return true;
}

bool
vl::ProgramOptions::_parseMaster( po::variables_map const &vm )
{
	if( vm.count("auto_fork") )
	{
		auto_fork = true;
	}

	return true;
}
