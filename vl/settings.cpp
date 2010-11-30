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
{}

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


std::vector< vl::ProjSettings::Scene>
vl::Settings::getScenes( void ) const
{
	std::vector<ProjSettings::Scene> scenes;

	if( _global )
	{ _addScenes( scenes, _global->getCasePtr() ); }

	if( _proj )
	{ _addScenes( scenes, _proj->getCasePtr() ); }

	if( _proj && !_case.empty() )
	{ _addScenes( scenes, _proj->getCasePtr(_case) ); }

	return scenes;
}

std::vector< std::string >
vl::Settings::getScripts( void ) const
{
	std::vector<std::string> vec;

	if( _global )
	{ _addScripts( vec, _global->getCasePtr() ); }

	if( _proj )
	{ _addScripts( vec, _proj->getCasePtr() ); }

	if( _proj && !_case.empty() )
	{ _addScripts( vec, _proj->getCasePtr(_case) ); }

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
						   ProjSettings::Case const *cas ) const
{
	for( size_t i = 0; i < cas->getNscripts(); ++i )
	{
		vl::ProjSettings::Script const *script = cas->getScriptPtr(i);
		if( script->getUse() )
		{
			vec.push_back( script->getFile() );
		}
	}
}


void
vl::Settings::_addScenes( std::vector< vl::ProjSettings::Scene> &vec,
						 ProjSettings::Case const *cas ) const
{
	for( size_t i = 0; i < cas->getNscenes(); ++i )
	{
		ProjSettings::Scene const &scene = *(cas->getScenePtr(i));
		if( scene.getUse() )
		{
			vec.push_back( scene );
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
