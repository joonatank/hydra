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
