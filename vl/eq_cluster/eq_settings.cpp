/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

// Declaration
#include "eq_settings.hpp"

// Necessary for serializing
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include "base/filesystem.hpp"
#include "settings.hpp"
#include "base/envsettings.hpp"

// Used for command line option parsing
#include <boost/program_options.hpp>

namespace po = boost::program_options;

/// Global functions
vl::SettingsRefPtr
eqOgre::getSettings( int argc, char **argv )
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
		("eq-client", "start a listening client")
		("eq-listen", po::value< std::string >(), "whom to listen, hostname:port")
	;
	// TODO add support for setting the log directory
	// TODO add control for the log level at least ERROR, INFO, TRACE
	// TODO add support for the case

	// Parse command line
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	bool verbose = false;
	// Print help
	if( vm.count("help") )
	{
		std::cout << "Help : " << desc << "\n";
		return vl::SettingsRefPtr();
	}
	// Verbose
	if( vm.count("verbose") )
	{
		verbose = true;
	}

	bool eq_client = false;
	std::string eq_listen;

	// Process eq-options
	if( vm.count("eq-client") )
	{
		eq_client = true;
	}
	if( vm.count("eq-listen") )
	{
		eq_listen = vm["eq-listen"].as<std::string>();
	}

	if( eq_client && eq_listen.empty() )
	{
		std::cerr << "Requested to be a client but does not have an address to "
			<< "listen to." << std::endl;
		return vl::SettingsRefPtr();
	}

	// Process eqOgre options
	std::string proj_path;
	std::string global_path;
	std::string env_path;
	std::string case_name;

	if (vm.count("environment"))
	{
		env_path = vm["environment"].as<std::string>();
		std::cout << "Environment path was set to : " << env_path << std::endl;
	} else {
		std::cout << "Environment was not set." << std::endl;
	}
	if (vm.count("project"))
	{
		proj_path = vm["project"].as<std::string>();
		std::cout << "Project path was set to : " << proj_path << std::endl;
	} else {
		std::cout << "Project was not set." << std::endl;
	}
	if (vm.count("global"))
	{
		global_path = vm["global"].as<std::string>();
		std::cout << "Global path was set to : " << global_path << std::endl;
	} else {
		std::cout << "Global was not set." << std::endl;
	}
	if (vm.count("case"))
	{
		case_name = vm["case"].as<std::string>();
		std::cout << "Case was set to : " << case_name << std::endl;
	} else {
		std::cout << "Case was not set." << std::endl;
	}

	// We need to be either listening or have both environment config
	// and project config (this might be changed later)
	bool valid_env = (!env_path.empty() && fs::exists( env_path ) );
	bool valid_proj = (!proj_path.empty() && fs::exists( proj_path ) );
	if( !eq_client && !valid_env && !valid_proj )
	{
		std::cerr << "Either start in listening mode using --eq-client or "
			<< std::endl << " provide valid environment config file and "
			"project config file." << std::endl;
		return vl::SettingsRefPtr();
	}

	/// Get the log dir we use
	// Create the logging directory if it doesn't exist
	// TODO this should create it to the exe path, not current directory
	std::string log_base("logs");
	std::string log_dir(log_base);

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

	vl::SettingsRefPtr settings;


	/// This point both env_path and project_path are either valid or we
	/// are running in listening mode
	// First check the listening mode
	if( eq_client )
	{
		settings.reset( new vl::Settings() );

		// Add the command line arguments
		settings->setExePath( argv[0] );
		if( eq_client )
		{
			settings->getEqArgs().add("--eq-client");
			settings->getEqArgs().add("--eq-listen");
			settings->getEqArgs().add( eq_listen.c_str() );
		}
		settings->setLogDir( log_dir );
	}
	else
	{
		// TODO add case support
		vl::EnvSettingsRefPtr env( new vl::EnvSettings );
		vl::ProjSettingsRefPtr proj( new vl::ProjSettings );
		vl::ProjSettingsRefPtr global;

		/// Read the Environment config
		if( fs::exists( env_path ) )
		{
			std::string env_data;
			env_data = vl::readFileToString( env_path );
			// TODO check that the files are correct and we have good settings
			vl::EnvSettingsSerializer env_ser( env );
			env_ser.readString(env_data);
			env->setFile( env_path );
		}

		/// Read the Project Config
		if( fs::exists( proj_path ) )
		{
			std::string proj_data;
			proj_data = vl::readFileToString( proj_path );
			vl::ProjSettingsSerializer proj_ser( proj );
			proj_ser.readString(proj_data);
			proj->setFile( proj_path );
		}

		/// Read the global config
		if( fs::exists( global_path ) )
		{
			global.reset( new vl::ProjSettings );
			std::string global_data;
			global_data = vl::readFileToString( global_path );
			vl::ProjSettingsSerializer glob_ser( global );
			glob_ser.readString(global_data);
			global->setFile( global_path );
		}

		settings.reset( new vl::Settings( env, proj, global ) );
		settings->setExePath( argv[0] );
		settings->setLogDir( log_dir );
	}

	settings->setVerbose(verbose);
	return settings;
}


eqOgre::SceneData::SceneData( void )
	: name(), file_data(), attachto_scene(), attachto_point()
{}


eqOgre::SceneData::SceneData( vl::ProjSettings::Scene const &scene,
							  std::string const &scene_file_data )
	: name( scene.getName() ),
	  file_data(scene_file_data),
	  attachto_scene( scene.getAttachtoScene() ),
	  attachto_point( scene.getAttachtoPoint() )
{
//	vl::readFileToString( scene.getFile(), file_data );
}

/// eqOgre::DistributedSettings
eqOgre::DistributedSettings::DistributedSettings( void )
	 : _frame_data_id(eq::base::UUID::ZERO),
	   _camera_rotations_allowed( 1 | 1<<1 | 1<<2 )
{}

void
eqOgre::DistributedSettings::copySettings( vl::SettingsRefPtr settings,
										   vl::ResourceManager *resource_man )
{
	// Copy name
	_project_name = settings->getProjectName();

	// Copy log paths
	_log_dir = settings->getLogDir();

	// Copy resource paths
	_resources.clear();
	std::vector<std::string> const &resources = settings->getResourcePaths();
	for( std::vector<std::string>::const_iterator iter = resources.begin();
		 iter != resources.end(); ++iter )
	{
		_resources.push_back( *iter );
	}

	// Copy scenes
	_scenes.clear();
	std::vector<vl::ProjSettings::Scene> const &scenes = settings->getScenes();
	for( std::vector<vl::ProjSettings::Scene>::const_iterator scene_iter = scenes.begin();
		 scene_iter != scenes.end(); ++scene_iter )
	{
		std::string scene_data;
		EQASSERT( resource_man->loadResource( scene_iter->getFile(), scene_data ) );
		_scenes.push_back( SceneData( *scene_iter, scene_data ) );
	}

	// Copy camera rotations flags
	_camera_rotations_allowed = settings->getEnvironmentSettings()
		->getCameraRotationAllowed();
}

std::string
eqOgre::DistributedSettings::getOgreLogFilePath( void ) const
{
	return vl::createLogFilePath( _project_name, "ogre", "", _log_dir );
}


/// -------------------- Protected ---------------------
void
eqOgre::DistributedSettings::getInstanceData(eq::net::DataOStream& os)
{
	os << _project_name << _frame_data_id << _log_dir;

	// Serialize resources
	os << _resources;

	// Serialize scenes
	EQINFO << "Serializing " << _scenes.size() << " scenes." << std::endl;
	os << _scenes.size();
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		operator<<( _scenes.at(i), os );
	}

	// Serialize the camera allowed rotations
	os << _camera_rotations_allowed;
	// TODO this should serialize used plugins
}

void
eqOgre::DistributedSettings::applyInstanceData(eq::net::DataIStream& is)
{
	is >> _project_name >> _frame_data_id >> _log_dir;

	// Serialize resources
	is >> _resources;

	// Serialize scenes
	//is >> _scenes;
	size_t size = 0;
	is >> size;
	_scenes.resize(size);
	EQINFO << "Deserializing " << _scenes.size() << " scenes." << std::endl;
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		operator>>( _scenes.at(i), is );
	}

	is >> _camera_rotations_allowed;
}

eq::net::DataOStream &
eqOgre::operator<<( eqOgre::SceneData const &scene, eq::net::DataOStream& os )
{
	os << scene.name << scene.file_data << scene.attachto_scene
		<< scene.attachto_point;

	return os;
}

eq::net::DataIStream &
eqOgre::operator>>( eqOgre::SceneData &scene, eq::net::DataIStream& is )
{
	is >> scene.name >> scene.file_data >> scene.attachto_scene
		>> scene.attachto_point;

	return is;
}
