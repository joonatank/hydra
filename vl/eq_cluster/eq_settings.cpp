/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

// Declaration
#include "eq_settings.hpp"

// Necessary for serializing
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

// Necessary for vl::Arguments
#include "arguments.hpp"
#include "base/filesystem.hpp"

/// Global functions
eqOgre::SettingsRefPtr
eqOgre::getSettings( int argc, char **argv )
{
	// Process command line arguments
	vl::Arguments arguments( argc, argv );

	std::cout << "environment path = " << arguments.env_path << std::endl;
	std::cout << "project path = " << arguments.proj_path << std::endl;
	std::cout << "global path = " << arguments.global_path << std::endl;
	std::cout << "case name = " << arguments.case_name << std::endl;

	// TODO add case support
	vl::EnvSettingsRefPtr env( new vl::EnvSettings );
	vl::ProjSettingsRefPtr proj( new vl::ProjSettings );
	vl::ProjSettingsRefPtr global;

	/// Read the Environment config
	if( fs::exists( arguments.env_path ) )
	{
		std::string env_data;
		env_data = vl::readFileToString( arguments.env_path );
		// TODO check that the files are correct and we have good settings
		vl::EnvSettingsSerializer env_ser( env );
		env_ser.readString(env_data);
		env->setFile( arguments.env_path );
	}
	else
	{
		std::cerr << "No environment file : " << arguments.env_path << std::endl;
		// Mandatory so we return here
		return eqOgre::SettingsRefPtr();
	}

	/// Read the Project Config
	if( fs::exists( arguments.proj_path ) )
	{
		std::string proj_data;
		proj_data = vl::readFileToString( arguments.proj_path );
		vl::ProjSettingsSerializer proj_ser( proj );
		proj_ser.readString(proj_data);
		proj->setFile( arguments.proj_path );
	}
	else
	{
		std::cerr << "No project file : " << arguments.proj_path << std::endl;
		// Mandatory so we return here
		return eqOgre::SettingsRefPtr();
	}

	/// Read the global config
	if( fs::exists( arguments.global_path ) )
	{
		global.reset( new vl::ProjSettings );
		std::string global_data;
		global_data = vl::readFileToString( arguments.global_path );
		vl::ProjSettingsSerializer glob_ser( global );
		glob_ser.readString(global_data);
		global->setFile( arguments.global_path );
	}
	else
	{
		std::cerr << "No global config file : " << arguments.global_path << std::endl;
		// Optional so we continue
	}


	eqOgre::SettingsRefPtr settings( new eqOgre::Settings( env, proj, global ) );

	// Add the command line arguments
	// TODO this should only add Equalizer arguments
	// or we could implement our own switches and supply the equalizer
	// arguments here based on our own switches.
	settings->setExePath( argv[0] );
	for( int i = 1; i < argc; ++i )
	{
		settings->getEqArgs().add(argv[i] );
	}

	return settings;
}


/// eqOgre::Settings
eqOgre::Settings::Settings( vl::EnvSettingsRefPtr env, vl::ProjSettingsRefPtr proj,
							vl::ProjSettingsRefPtr global )
	: vl::Settings( env, proj, global ),
	  _frame_data_id(EQ_ID_INVALID)
{}

void eqOgre::Settings::getInstanceData(eq::net::DataOStream& os)
{
	os << _frame_data_id << _log_dir;

	// TODO this should serialize used plugins
	// TODO this should serialize ProjectSettings
	// 	which needs eqOgre::ProjectSettings to be created
	/*
	os << _roots.size();
	for( size_t i = 0; i < _roots.size(); ++i )
	{
		os << _roots.at(i).name << _roots.at(i).path;
	}
*/
	// Not necessary as these are only needed when launched
//	os << _exe_path << _file_path << _eq_config.file;
/*
	os << _scenes.size();
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		os << _scenes.at(i).file << _scenes.at(i).name
			<< _scenes.at(i).attach_node << _scenes.at(i).type;
	}

	os << _plugins.getPath();

	os << _resources.size();
	for( size_t i = 0; i < _resources.size(); ++i )
	{
		os << _resources.at(i).getPath();
	}

	os << _tracking.size();
	for( size_t i = 0; i < _tracking.size(); ++i )
	{
		os << _tracking.at(i).getPath();
	}
*/
	// No need to distribute eq args as they are only needed when launched

	// TODO
	// These probably don't need to be distributed also as the AppNode should
	// handle tracking
//	os << _tracker_address << _tracker_default_pos << _tracker_default_orient;
}

void eqOgre::Settings::applyInstanceData(eq::net::DataIStream& is)
{
	is >> _frame_data_id >> _log_dir;

/*
	size_t size;
	is >> size;
	_roots.resize(size);
	for( size_t i = 0; i < _roots.size(); ++i )
	{
		is >> _roots.at(i).name >> _roots.at(i).path;
	}

	// Not necessary as these are only needed when launched
//	is >> _exe_path >> _file_path >> _eq_config.file;

	is >> size;
	_scenes.resize(size);
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		is >> _scenes.at(i).file >> _scenes.at(i).name
			>> _scenes.at(i).attach_node >> _scenes.at(i).type;
	}

	is >> _plugins.file;

	is >> size;
	_resources.resize(size);
	for( size_t i = 0; i < _resources.size(); ++i )
	{
		is >> _resources.at(i).file;
	}

	is >> size;
	_tracking.resize(size);
	for( size_t i = 0; i < _tracking.size(); ++i )
	{
		is >> _tracking.at(i).file;
	}
*/
	// No need to distribute eq args as they are only needed when launched

	// TODO
	// These probably don't need to be distributed also as the AppNode should
	// handle tracking
//	is >> _tracker_address >> _tracker_default_pos >> _tracker_default_orient;
}
