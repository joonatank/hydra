/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file program_options.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
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
#include <vector>

#include <cstdint>

// Necessary for storing the ini file path
#include "base/filesystem.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"

namespace vl
{

struct HYDRA_API DebugOptions
{
	DebugOptions(void)
		: overlay(false)
		, overlay_advanced(false)
		, axes(false)
		, display(false)
	{}

	bool overlay;
	bool overlay_advanced;
	bool axes;
	bool display;
};

/// @class ProgramOptions
/// Supports configuration using a single ini file and command line parameters
/// Ini file is always parsed before command line options so command line overrides
/// ini options. 
/// Ini file format is standard Windows ini file format
/// Some options for ini file and command line differ by command line omiting the
/// true/false values.
struct HYDRA_API ProgramOptions
{
	/// @brief Constructor
	/// @param ini_file name of the ini file we try to parse
	/// Ini file does not need to exist.
	/// We search the ini file with the name from multiple default locations
	/// primary location is the application directory (os specific configuration storage)
	/// secondary location is the directory where the program was started.
	/// @todo does this use HydraMain.dll location or hydra.exe location?
	ProgramOptions(std::string const &ini_file = std::string("hydra.ini"));

	~ProgramOptions(void);

	/**	Parse options from both ini file and command line
	 *	@ Post-condition valid or invalid configuration with all the parameters
	 *	parsed
	 */
	bool parseOptions( int argc, char **argv );

	void parseOptions(void);

	/// Is the configuration for a slave,
	/// true for both valid and invalid configurations
	/// If this is true master() is always false
	bool slave( void ) const;

	/// Is the configuration for a master
	/// true for both valid and invalid configurations
	/// If this is true slave is always false
	bool master( void ) const;

	/// @brief Get the log file name for this specific instance
	/// Different names for master and all slaves.
	std::string getLogFile(void) const;

	/// @brief Get the log directory we want to use.
	/// If user specified an absolute directory path this will return it unmodified.
	/// if the user specified a relative path the path will be relative to exe dir.
	std::string getLogDir(void) const;

	/// Global options
	bool verbose;
	int log_level;
	std::string exe_name;
	std::string program_directory;
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

	DebugOptions debug;

	int n_processors;
	int start_processor;

	uint16_t launcher_port;

	/// New variable for supporting multiple projects
	/// that are loadable at runtime, single project can be active at once.
	std::vector<std::string> project_paths;

private :
	/// @brief Parse only ini file
	void _parse_ini(void);

	std::string _ini_file;

	std::string _log_dir_name;

	fs::path _ini_file_path;

};	// class ProgramOptions

}	// namespace vl

#endif	// HYDRA_PROGRAM_OPTIONS_HPP
