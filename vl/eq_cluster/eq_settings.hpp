/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Global function to create the settings from command-line arguments
 *
 *	Distributed settings structure
 */
#ifndef EQ_OGRE_SETTINGS_HPP
#define EQ_OGRE_SETTINGS_HPP

#include <co/object.h>

// Needed for ProjSettings::Scene
#include "base/projsettings.hpp"
// Needed for EnvSettings::Wall and EnvSettings::Window
#include "base/envsettings.hpp"

// Needed for RefPtrs
#include "base/typedefs.hpp"

// Needed for loading the scene files
#include "resource_manager.hpp"

namespace eqOgre
{

vl::SettingsRefPtr getSettings( int argc, char **argv );

/// The settings for the current project that needs to be distributed
/// Minimal version of vl::Settings that does not contain information on the
/// links between project and scenes.
/// Contains only data needed by the slaves e.g. contains list of scenes
/// but contains no scripts.
/// Also contains runtime parameters that are not in vl::Settings.
class DistributedSettings : public co::Object
{
public :
	DistributedSettings( void );

	virtual ~DistributedSettings( void )
	{}

	uint64_t const getSceneManagerID( void ) const
	{ return _scene_manager_id; }

	void setSceneManagerID( uint64_t const id )
	{ _scene_manager_id = id; }

	eq::base::UUID const &getResourceManagerID( void ) const
	{ return _resource_man_id; }

	void setResourceManagerID( eq::base::UUID const &id )
	{ _resource_man_id = id; }

	eq::base::UUID const &getPlayerID( void ) const
	{ return _player_id; }

	void setPlayerID( eq::base::UUID const &id )
	{ _player_id = id; }

	/// Used by master to copy the necessary data for synchronization
	// TODO Why is the resource man passed here?
	void copySettings( vl::SettingsRefPtr settings, vl::ResourceManager *resource_man );

	/// Logging
	std::string getOgreLogFilePath( void ) const;

	std::string const &getProjectName( void ) const
	{ return _project_name; }

	/// Returns a flags of around which axes the camera rotations are allowed
	/// Fist bit is the x axis, second y axis, third z axis
	uint32_t getCameraRotationAllowed() const
	{ return _camera_rotations_allowed; }

	vl::EnvSettings::Window findWindow( std::string const &name ) const;

	vl::EnvSettings::Window const &getWindow( size_t i ) const
	{ return _windows.at(i); }

	size_t getNWindows( void ) const
	{ return _windows.size(); }

	vl::EnvSettings::Wall findWall( std::string const &channel_name ) const;

	vl::EnvSettings::Wall const &getWall( size_t i ) const
	{ return _walls.at(i); }

	size_t getNWalls( void ) const
	{ return _walls.size(); }

	double getIPD( void ) const
	{ return _ipd; }

	vl::EnvSettings::CFG getStereo( void ) const
	{ return _stereo; }

protected :
	virtual void getInstanceData( co::DataOStream& os );
	virtual void applyInstanceData( co::DataIStream& is );

	std::string _log_dir;

	std::string _project_name;

	uint64_t _scene_manager_id;
	eq::base::UUID _resource_man_id;
	eq::base::UUID _player_id;

	uint32_t _camera_rotations_allowed;

	std::vector<vl::EnvSettings::Wall> _walls;

	std::vector<vl::EnvSettings::Window> _windows;

	vl::EnvSettings::CFG _stereo;

	double _ipd;
};

// Necessary to overload the Window serialization because the Equalizer default
// i.e. memcpy somehow fails and crashes the application on destruction
co::DataOStream &
operator<<(  vl::EnvSettings::Window const &win, co::DataOStream &os );

co::DataIStream &
operator>>( vl::EnvSettings::Window &win, co::DataIStream &is );

co::DataOStream &
operator<<( std::vector<vl::EnvSettings::Window> const &win, co::DataOStream &os );

co::DataIStream &
operator>>( std::vector<vl::EnvSettings::Window> &win, co::DataIStream &is );

// Necessary to overload the Wall serialization because the Equalizer default
// i.e. memcpy somehow fails and crashes the application on destruction
co::DataOStream &
operator<<(  vl::EnvSettings::Wall const &wall, co::DataOStream &os );

co::DataIStream &
operator>>( vl::EnvSettings::Wall &wall, co::DataIStream &is );

co::DataOStream &
operator<<( std::vector<vl::EnvSettings::Wall> const &walls, co::DataOStream &os );

co::DataIStream &
operator>>( std::vector<vl::EnvSettings::Wall> &walls, co::DataIStream &is );
}	// namespace eqOgre

#endif //EQ_OGRE_SETTINGS_HPP
