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

vl::Settings::Settings( vl::EnvSettingsRefPtr env, vl::ProjSettingsRefPtr proj,
						vl::ProjSettingsRefPtr global )
	: _env(env),
	  _global(global),
	  _proj(proj)
{}

vl::Settings::~Settings( void )
{}

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
vl::Settings::getEqLogFilePath(void ) const
{
	return getLogFilePath("eq");
}

std::string
vl::Settings::getOgreLogFilePath(void ) const
{
	return getLogFilePath("ogre");
}

std::string
vl::Settings::getLogFilePath(const std::string &identifier,
							 const std::string &prefix) const
{
	uint32_t pid = vl::getPid();
	std::stringstream ss;
	if( !getLogDir().empty() )
	{ ss << getLogDir() << "/"; }

	if( getProjectName().empty() )
	{ ss << "unamed"; }
	else
	{ ss << getProjectName(); }

	if( !identifier.empty() )
	{ ss << '_' << identifier; }

	ss << '_' << pid;

	if( !prefix.empty() )
	{ ss << '_' << prefix; }

	ss << ".log";

	return ss.str();
}

std::vector< vl::ProjSettings::Scene const * >
vl::Settings::getScenes( void ) const
{
	std::vector<ProjSettings::Scene const *> vec;

	if( _global )
	{
		_addScenes( vec, _global->getCasePtr() );
	}

	_addScenes( vec, _proj->getCasePtr() );

	if( !_case.empty() )
	{
		_addScenes( vec, _proj->getCasePtr(_case) );
	}

	return vec;
}

std::vector< std::string >
vl::Settings::getScripts( void ) const
{
	std::vector<std::string> vec;

	if( _global )
	{
		_addScripts( vec, getGlobalDir(), _global->getCasePtr() );
	}

	_addScripts( vec, getProjectDir(), _proj->getCasePtr() );

	if( !_case.empty() )
	{
		_addScripts( vec, getProjectDir(), _proj->getCasePtr(_case) );
	}

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
vl::Settings::_addScenes( std::vector< vl::ProjSettings::Scene const *> &vec,
						 ProjSettings::Case const *cas ) const
{
	for( size_t i = 0; i < cas->getNscenes(); ++i )
	{
		if( cas->getScenePtr(i)->getUse() )
		{ vec.push_back( cas->getScenePtr(i) ); }
	}
}


void
vl::Settings::_updateArgs( void )
{
	// Update args
	_eq_args.clear();
	if( !_exe_path.empty() )
	{ _eq_args.add( _exe_path.c_str() ); }

	// TODO is this necessary?
	if( !_env )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	if( !_env->getEqcFullPath().empty() )
	{
		_eq_args.add( "--eq-config" );
		_eq_args.add( _env->getEqcFullPath().c_str() );
	}
}
