/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

// Declaration
#include "eq_settings.hpp"

// Necessary for serializing
#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include "base/filesystem.hpp"
#include "settings.hpp"
#include "base/envsettings.hpp"

#include "distributed.hpp"

// Needed for loading the scene files
#include "resource_manager.hpp"


/// --------------- eqOgre::DistributedSettings -----------------------
eqOgre::DistributedSettings::DistributedSettings( void )
	 : _scene_manager_id( vl::ID_UNDEFINED ),
	   _resource_man_id( vl::ID_UNDEFINED ),
	   _player_id( vl::ID_UNDEFINED ),
	   _camera_rotations_allowed( 1 | 1<<1 | 1<<2 ),
	   _stereo( vl::EnvSettings::ON ),
	   _ipd(0)
{}

void
eqOgre::DistributedSettings::copySettings( vl::SettingsRefPtr settings,
										   vl::ResourceManager *resource_man )
{
	// Copy name
	_project_name = settings->getProjectName();

	// Copy log paths
// 	_log_dir = settings->getLogDir();

// 	vl::EnvSettingsRefPtr env = settings->getEnvironmentSettings();

	// Copy camera rotations flags
// 	_camera_rotations_allowed = env->getCameraRotationAllowed();

	// Copy Wall definitions
// 	_walls = env->getWalls();
// 	assert( _walls.size() > 0 );

	// Copy windows definitions
// 	_windows = env->getWindows();

	// Copy stereo and ipd
// 	_stereo = env->getStereo();
// 	_ipd = env->getIPD();
}

std::string
eqOgre::DistributedSettings::getOgreLogFilePath( void ) const
{
	return vl::createLogFilePath( _project_name, "ogre", "", _log_dir );
}

// vl::EnvSettings::Window
// eqOgre::DistributedSettings::findWindow( std::string const &name ) const
// {
// 	std::vector<vl::EnvSettings::Window>::const_iterator iter;
// 	for( iter = _windows.begin(); iter != _windows.end(); ++iter )
// 	{
// 		if( iter->name == name )
// 		{ return *iter; }
// 	}
//
// 	return vl::EnvSettings::Window();
// }

vl::EnvSettings::Wall
eqOgre::DistributedSettings::findWall( std::string const &channel_name ) const
{
	std::vector<vl::EnvSettings::Wall>::const_iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->channel_name == channel_name )
		{ return *iter; }
	}

	return vl::EnvSettings::Wall();
}



/// -------------------- Protected ---------------------
void
eqOgre::DistributedSettings::getInstanceData( co::DataOStream& os )
{
	os << _project_name << _scene_manager_id << _resource_man_id
		<< _player_id << _log_dir
		<< _camera_rotations_allowed;
	eqOgre::operator<<( _walls, os );
// 	eqOgre::operator<<( _windows, os );
	os << _stereo << _ipd;

	// TODO this should serialize used plugins
}

void
eqOgre::DistributedSettings::applyInstanceData( co::DataIStream& is )
{
	is >> _project_name >> _scene_manager_id >> _resource_man_id
		>> _player_id >> _log_dir
		>> _camera_rotations_allowed;
	eqOgre::operator>>(_walls, is);
// 	eqOgre::operator>>(_windows, is);
	is >> _stereo >> _ipd;
}

co::DataOStream &
eqOgre::operator<<( vl::EnvSettings::Window const &win, co::DataOStream &os )
{
	os << win.name << win.w << win.h << win.x << win.y;

	return os;
}

co::DataIStream &
eqOgre::operator>>( vl::EnvSettings::Window &win, co::DataIStream &is )
{
	is >> win.name >> win.w >> win.h >> win.x >> win.y;

	return is;
}

co::DataOStream &
eqOgre::operator<<( std::vector<vl::EnvSettings::Window> const &wins, co::DataOStream &os )
{
	os << wins.size();
	for( size_t i = 0; i < wins.size(); ++i )
	{ eqOgre::operator<<( wins.at(i), os ); }

	return os;
}

co::DataIStream &
eqOgre::operator>>( std::vector<vl::EnvSettings::Window> &wins, co::DataIStream &is )
{
	size_t size;
	is >> size;
	wins.resize(size);
	for( size_t i = 0; i < wins.size(); ++i )
	{ eqOgre::operator>>( wins.at(i), is ); }

	return is;
}


// Wall serialization
co::DataOStream &
eqOgre::operator<<( vl::EnvSettings::Wall const &wall, co::DataOStream &os )
{
	os << wall.name << wall.channel_name << wall.bottom_left
		<< wall.bottom_right << wall.top_left;

	return os;
}

co::DataIStream &
eqOgre::operator>>( vl::EnvSettings::Wall &wall, co::DataIStream &is )
{
	is >> wall.name >> wall.channel_name >> wall.bottom_left
		>> wall.bottom_right >> wall.top_left;

	return is;
}

co::DataOStream &
eqOgre::operator<<( std::vector<vl::EnvSettings::Wall> const &walls, co::DataOStream &os )
{
	os << walls.size();
	for( size_t i = 0; i < walls.size(); ++i )
	{ eqOgre::operator<<( walls.at(i), os ); }

	return os;
}

co::DataIStream &
eqOgre::operator>>( std::vector<vl::EnvSettings::Wall> &walls, co::DataIStream &is )
{
	size_t size;
	is >> size;
	walls.resize(size);
	for( size_t i = 0; i < walls.size(); ++i )
	{ eqOgre::operator>>( walls.at(i), is ); }

	return is;
}
