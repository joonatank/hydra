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
	, auto_fork(false)
	, show_system_console(false)
{}

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
	// Process command line arguments
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce a help message")
		("verbose,v", "print the output to system console")
		("log_level,l", po::value<int>(), "how much detail is logged")
		("log_dir", po::value<std::string>(), "where to write the log files")
		("environment,e", po::value< std::string >(), "environment file")
		("project,p", po::value< std::string >(), "project file")
		("global,g", po::value< std::string >(), "global file")
		("case,c", po::value< std::string >(), "case name")
		("slave", po::value< std::string >(), "start a named rendering slave")
		("server", po::value< std::string >(), "master server where to connect to, hostname:port")
		("auto_fork,f", "Auto fork slave processes. Only usefull for virtual clusters.")
		("display", po::value<int>()->default_value(0), "Display to use for the window. Only on X11.")
		("show_system_console", "Show the system console window on startup.")
	;

	// TODO add control for the log level at least ERROR, INFO, TRACE

	// Parse command line
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// Print help
	if( vm.count("help") )
	{
		std::cout << "Help : " << desc << "\n";
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

	/// Display setting available both on master and slave
	if( vm.count("display") )
	{
		display_n = vm["display"].as<int>();
	}

	if( vm.count("show_system_console") )
	{
		show_system_console = true;
	}

	if( vm.count("log_dir") )
	{
		log_dir = vm["log_dir"].as<std::string>();
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
	if (vm.count("environment"))
	{
		environment_file = vm["environment"].as<std::string>();
	}

	if (vm.count("project"))
	{
		project_file = vm["project"].as<std::string>();
	}

	if (vm.count("global"))
	{
		global_file = vm["global"].as<std::string>();
	}

	if( vm.count("auto_fork") )
	{
		auto_fork = true;
	}

	return true;
}
