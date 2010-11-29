/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

// Declaration
#include "settings.hpp"

// Necessary for exceptions
#include "base/exceptions.hpp"

// Necessary for getPid (used for log file names)
#include "base/system_util.hpp"

#include "base/envsettings.hpp"

std::string
vl::createLogFilePath( const std::string &project_name,
					   const std::string &identifier,
					   const std::string &prefix,
					   const std::string &log_dir )
{
	uint32_t pid = vl::getPid();
	std::stringstream ss;

	if( !log_dir.empty() )
	{ ss << log_dir << "/"; }

	if( project_name.empty() )
	{ ss << "unamed"; }
	else
	{ ss << project_name; }

	if( !identifier.empty() )
	{ ss << '_' << identifier; }

	ss << '_' << pid;

	if( !prefix.empty() )
	{ ss << '_' << prefix; }

	ss << ".log";

	return ss.str();
}



vl::Settings::Settings( vl::EnvSettingsRefPtr env, vl::ProjSettingsRefPtr proj,
						vl::ProjSettingsRefPtr global )
	: _env(env),
	  _global(global),
	  _proj(proj),
	  _verbose(false)
{
	_initScenes();
}

vl::Settings::~Settings( void )
{}

std::string
vl::Settings::getLogDir(vl::Settings::PATH_TYPE const type ) const
{
	if( type == PATH_REL )
	{ return _log_dir; }
	else
	{
		fs::path path = fs::complete( _log_dir );
		return path.file_string();
	}
}

void
vl::Settings::setExePath( std::string const &path )
{
	_exe_path = path;

	_updateArgs();
}

std::string
vl::Settings::getProjectName(void ) const
{
	if( _proj )
	{ return _proj->getCasePtr()->getName(); }
	else
	{ return std::string(); }
}

std::string
vl::Settings::getEqLogFilePath( PATH_TYPE const type ) const
{
	return getLogFilePath( "eq", "", type );
}

std::string
vl::Settings::getOgreLogFilePath( PATH_TYPE const type  ) const
{
	return getLogFilePath( "ogre", "", type );
}

std::string
vl::Settings::getLogFilePath( const std::string &identifier,
							  const std::string &prefix,
							  PATH_TYPE const type ) const
{
	return createLogFilePath( getProjectName(), identifier, prefix, getLogDir(type) );
}

std::vector< std::string >
vl::Settings::getTrackingPaths(void ) const
{
	std::string dir( getEnvironementDir() + "/tracking/" );
	std::vector<std::string> vec;
	if( _env )
	{
		for( size_t i = 0; i < _env->getTracking().size(); ++i )
		{
			std::string path = dir + _env->getTracking().at(i);
			if( fs::exists( path ) )
			{ vec.push_back( path ); }
		}
	}

	return vec;
}


std::vector< vl::ProjSettings::Scene> const &
vl::Settings::getScenes( void ) const
{
	return _scenes;
}

std::vector< std::string >
vl::Settings::getScripts( void ) const
{
	std::vector<std::string> vec;

	if( _global )
	{ _addScripts( vec, getGlobalDir(), _global->getCasePtr() ); }

	if( _proj )
	{ _addScripts( vec, getProjectDir(), _proj->getCasePtr() ); }

	if( _proj && !_case.empty() )
	{ _addScripts( vec, getProjectDir(), _proj->getCasePtr(_case) ); }

	return vec;
}

std::vector< std::string > vl::Settings::getResourcePaths(void ) const
{
	std::string dir_name = "/resources";

	std::vector<std::string> vec;
	if( _global )
	{
		std::string dir = getGlobalDir() + dir_name;
		if( fs::exists( dir ) )
		{ vec.push_back( dir ); }
	}
	if( _proj )
	{
		std::string dir = getProjectDir() + dir_name;
		if( fs::exists( dir ) )
		{ vec.push_back( dir ); }
	}

	return vec;
}


std::string vl::Settings::getGlobalDir(void ) const
{
	// This shouldn't be called from slave node
	if( !_global )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	fs::path globFile( _global->getFile() );
	fs::path globDir = globFile.parent_path();
	if( !fs::exists( globDir ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( globDir.file_string() ) ); }

	return globDir.file_string();
}

std::string
vl::Settings::getProjectDir( void ) const
{
	// This shouldn't be called from slave node
	if( !_proj )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	fs::path projFile( _proj->getFile() );
	fs::path projDir = projFile.parent_path();
	if( !fs::exists( projDir ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( projDir.file_string() ) ); }

	return projDir.file_string();
}

std::string
vl::Settings::getEnvironementDir( void ) const
{
	// This shouldn't be called from slave node
	if( !_env )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	fs::path envFile( _env->getFile() );
	fs::path envDir = envFile.parent_path();
	if( !fs::exists( envDir ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( envDir.file_string() ) ); }

	return envDir.file_string();
}


// --------- Settings Protected --------
void
vl::Settings::_addScripts( std::vector< std::string > &vec,
						   std::string const &projDir,
						   ProjSettings::Case const *cas) const
{
	for( size_t i = 0; i < cas->getNscripts(); ++i )
	{
		vl::ProjSettings::Script *script = cas->getScriptPtr(i);
		if( script->getUse() )
		{
			std::string scriptPath = projDir + "/scripts/" + script->getFile();
			vec.push_back( scriptPath );
		}
	}
}


void
vl::Settings::_addScenes( std::vector< vl::ProjSettings::Scene> &vec,
						 ProjSettings::Case const *cas ) const
{
	std::vector<std::string> const &resources = getResourcePaths();

	for( size_t i = 0; i < cas->getNscenes(); ++i )
	{
		if( cas->getScenePtr(i)->getUse() )
		{
			ProjSettings::Scene const &scene = *(cas->getScenePtr(i));

			// find the real path of the scene file
			std::string scene_file_name = scene.getFile();
			std::string full_path;
			bool found = false;
			std::vector<std::string>::const_iterator iter;
			for( iter = resources.begin(); iter != resources.end(); ++iter )
			{
				if( vl::find_file( *iter, scene_file_name, full_path ) )
				{
					found = true;
					break;
				}
			}
			if( found )
			{
				// copy the scene file info
				vec.push_back( scene );
				vec.back().setFile( full_path );
			}
			else
			{
				std::cerr << "Real path of the scene " << scene_file_name
					<< " was not found in any of the resources." << std::endl;
			}
		}
	}
}


void
vl::Settings::_updateArgs( void )
{
	// Update args
	_eq_args.clear();
	if( !_exe_path.empty() )
	{ _eq_args.add( _exe_path.c_str() ); }

	// Only add eqc config in master node,
	// if _env is missing we can assume that this is a slave node
	if( _env && !_env->getEqcFullPath().empty() )
	{
		_eq_args.add( "--eq-config" );
		_eq_args.add( _env->getEqcFullPath().c_str() );
	}
}

void vl::Settings::_initScenes(void )
{
	_scenes.clear();

	/// Initialise the scenes structure with absolute paths
	if( _global )
	{ _addScenes( _scenes, _global->getCasePtr() ); }

	if( _proj )
	{ _addScenes( _scenes, _proj->getCasePtr() ); }

	if( _proj && !_case.empty() )
	{ _addScenes( _scenes, _proj->getCasePtr(_case) ); }
}
