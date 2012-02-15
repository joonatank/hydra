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

/*
 *	Interface for program options parsing
 *	Command line or settings file options passed to the program when started
 *	These options are static and can not be changed at run time
 *	They might create or change structures that are changeable at run time
 *	but the ProgramOptions struct is not.
 *	It only controls the start of the program and any values necessary for
 *	later use is copied from it.
 */

#ifndef HYDRA_PROGRAM_OPTIONS_HPP
#define HYDRA_PROGRAM_OPTIONS_HPP

#include <string>

// Necessary for HYDRA_API
#include "defines.hpp"
// Used for command line option parsing
#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace vl
{

struct HYDRA_API ProgramOptions
{
	ProgramOptions( void );

	~ProgramOptions(void);

	/**	Parse options from both ini file and command line
	 *	@ Post-condition valid or invalid configuration with all the parameters
	 *	parsed
	 */
	bool parseOptions( int argc, char **argv );

	/// @brief Parse only ini file
	bool parseIni(std::string const &file, po::variables_map vm = po::variables_map());

	/// Is the configuration for a slave,
	/// true for both valid and invalid configurations
	/// If this is true master() is always false
	bool slave( void ) const;

	/// Is the configuration for a master
	/// true for both valid and invalid configurations
	/// If this is true slave is always false
	bool master( void ) const;

	std::string getOutputFile(void) const;

	/// Global options
	bool verbose;
	int log_level;
	std::string exe_name;
	std::string program_directory;
	std::string log_dir;
	int display_n;

	/// Slave specific options
	bool _slave;
	std::string slave_name;
	std::string server_address;

	/// Master specific options
	std::string environment_file;
	std::string project_file;
	std::string global_file;
	bool auto_fork;
	bool show_system_console;
	bool editor;
	bool debug_overlay;

	int n_processors;
	int start_processor;

private :
	bool _parseSlave( po::variables_map const &vm );
	bool _parseMaster( po::variables_map const &vm );

private :
	po::options_description _cmd_options;
	po::options_description _config;
	po::options_description _file_options;

	std::string _config_file;

};	// class ProgramOptions

}	// namespace vl

#endif	// HYDRA_PROGRAM_OPTIONS_HPP
