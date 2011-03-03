/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *	Implementation for program options parsing
 */

// Interface
#include "program_options.hpp"

// Used for error printing
#include <iostream>

#include "base/filesystem.hpp"

vl::ProgramOptions::ProgramOptions( void )
	: verbose(false), _slave(false)
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

void
vl::ProgramOptions::parseOptions( int argc, char **argv )
{
	// Process command line arguments
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce a help message")
		("verbose,v", "print the Equalizer output into std::cerr instead of log file")
		("environment,e", po::value< std::string >(), "environment file")
		("project,p", po::value< std::string >(), "project file")
		("global,g", po::value< std::string >(), "global file")
		("case,c", po::value< std::string >(), "case name")
		("slave", po::value< std::string >(), "start a named rendering slave")
		("server", po::value< std::string >(), "master server where to connect to, hostname:port")
	;
	// TODO add support for setting the log directory
	// TODO add control for the log level at least ERROR, INFO, TRACE
	// TODO add support for the case

	// Parse command line
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// Print help
	if( vm.count("help") )
	{
		std::cout << "Help : " << desc << "\n";
		return;
	}

	// Verbose
	if( vm.count("verbose") )
	{
		verbose = true;
	}

	// Some global settings
	fs::path exe_path = fs::system_complete( argv[0] ).file_string();
	exe_name = exe_path.filename();
	program_directory = fs::path(exe_path).parent_path().file_string();

	// Slave options
	if( vm.count("slave") )
	{
		_slave = true;
		_parseSlave( vm );
	}
	else
	{
		_slave = false;
		_parseMaster( vm );
	}
}

void
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
		return;
	}

	if( vm.count("server") )
	{
		server_address = vm["server"].as<std::string>();
	}

	if( server_address.empty() )
	{
		std::cerr << "A slave without a server address to connect to is not allowed."
			<< std::endl;
		return;
	}
}

void
vl::ProgramOptions::_parseMaster( po::variables_map const &vm )
{
	if (vm.count("environment"))
	{
		environment_file = vm["environment"].as<std::string>();
		std::cout << "Environment path was set to : " << environment_file << std::endl;
	} else
	{
		std::cout << "Environment was not set." << std::endl;
	}
	if (vm.count("project"))
	{
		project_file = vm["project"].as<std::string>();
		std::cout << "Project path was set to : " << project_file << std::endl;
	}
	else
	{ std::cout << "Project was not set." << std::endl; }
	if (vm.count("global"))
	{
		global_file = vm["global"].as<std::string>();
		std::cout << "Global path was set to : " << global_file << std::endl;
	}
	else
	{ std::cout << "Global was not set." << std::endl; }
	if (vm.count("case"))
	{
		case_name = vm["case"].as<std::string>();
		std::cout << "Case was set to : " << case_name << std::endl;
	}
	else
	{ std::cout << "Case was not set." << std::endl; }

	// We need to be either listening or have both environment config
	// and project config (this might be changed later)
	// TODO the validity of the options should not be concerned here
	bool valid_env = (!environment_file.empty() && fs::exists( environment_file ) );
	bool valid_proj = (!project_file.empty() && fs::exists( project_file ) );
	if( !valid_env && !valid_proj )
	{
		std::cerr << "Either start in listening mode using --eq-client or "
			<< std::endl << " provide valid environment config file and "
			"project config file." << std::endl;
		return;
	}

	/// Get the log dir we use
	// Create the logging directory if it doesn't exist
	// TODO this should create it to the exe path, not current directory
	std::string log_base("logs");
	log_dir = std::string(log_base);

	// File already exists but it's not a directory
	size_t index = 0;
	while( fs::exists(log_dir) && !fs::is_directory( log_dir ) )
	{
		std::cerr << "File : " << log_dir << " already exists and it's not "
			<< "a directory. Trying another dir." << std::endl;
		++index;
		std::stringstream ss;
		// TODO should add zeros so we have three numbers
		ss << log_base << index;
		log_dir = ss.str();
	}

	// File doesn't exist (checked earlier)
	if( !fs::exists( log_dir ) )
	{
		fs::create_directory( log_dir );
	}

	// Otherwise the file exists and it's a directory
	std::cout << "Using log dir: " << log_dir << std::endl;
}
