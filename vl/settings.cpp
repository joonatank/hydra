#include "settings.hpp"

#include "base/filesystem.hpp"
#include "base/exceptions.hpp"
#include "base/string_utils.hpp"

#include <iostream>
#include <fstream>

vl::Settings::Settings( vl::EnvSettingsRefPtr env, vl::ProjSettingsRefPtr proj )
	: _env(env),
	  _proj(proj)
{}

vl::Settings::~Settings( void )
{}

void
vl::Settings::setExePath( std::string const &path )
{
	_exe_path = path;

	updateArgs();
}

void
vl::Settings::clear( void )
{
	_log_dir.clear();
	_exe_path.clear();
	_eq_args = vl::Args();
	_env.reset();
	_proj.reset();
}


// --------- Settings Private --------
void
vl::Settings::updateArgs( void )
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
