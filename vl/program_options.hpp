/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *	Interface for program options parsing
 *	Command line or settings file options passed to the program when started
 *	These options are static and can not be changed at run time
 *	They might create or change structures that are changeable at run time
 *	but the ProgramOptions struct is not.
 *	It only controls the start of the program and any values necessary for
 *	later use is copied from it.
 */

#ifndef VL_PROGRAM_OPTIONS_HPP
#define VL_PROGRAM_OPTIONS_HPP

#include <string>

// Used for command line option parsing
#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace vl
{

struct ProgramOptions
{
	ProgramOptions( void );

	/**	Parse command line options and create the structure
	 *	@ Post-condition valid or invalid configuration with all the parameters
	 *	parsed
	 */
	void parseOptions( int argc, char **argv );

	/// Is the configuration for a slave,
	/// true for both valid and invalid configurations
	/// If this is true master() is always false
	bool slave( void ) const;

	/// Is the configuration for a master
	/// true for both valid and invalid configurations
	/// If this is true slave is always false
	bool master( void ) const;

	/// Global options
	bool verbose;
	std::string exe_name;
	std::string program_directory;
	std::string log_dir;

	/// Slave specific options
	bool _slave;
	std::string slave_name;
	std::string server_address;

	/// Master specific options
	std::string environment_file;
	std::string project_file;
	std::string global_file;
	std::string case_name;

private :
	void _parseSlave( po::variables_map const &vm );
	void _parseMaster( po::variables_map const &vm );

};

}	// namespace vl

#endif