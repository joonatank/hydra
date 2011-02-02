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


vl::Settings::Settings( vl::ProjSettingsRefPtr proj,
						vl::ProjSettingsRefPtr global )
	: _global(global),
	  _proj(proj)
// 	  _verbose(false)
{}

vl::Settings::~Settings( void )
{}

std::string
vl::Settings::getProjectName(void ) const
{
	if( _proj )
	{ return _proj->getCasePtr()->getName(); }
	else
	{ return std::string(); }
}

// std::string
// vl::Settings::getEqLogFilePath( PATH_TYPE const type ) const
// {
// 	return getLogFilePath( "eq", "", type );
// }
//
// std::string
// vl::Settings::getOgreLogFilePath( PATH_TYPE const type  ) const
// {
// 	return getLogFilePath( "ogre", "", type );
// }

// std::string
// vl::Settings::getLogFilePath( const std::string &identifier,
// 							  const std::string &prefix,
// 							  PATH_TYPE const type ) const
// {
// // TODO move this to the calling function, because this doesn't have both
// // the log dir and the project name
// // 	return createLogFilePath( getProjectName(), identifier, prefix, getLogDir(type) );
// }


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

/*
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
}*/
