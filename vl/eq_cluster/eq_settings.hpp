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

	eq::base::UUID const &getFrameDataID( void ) const
	{ return _frame_data_id; }

	void setFrameDataID( eq::base::UUID const &id )
	{ _frame_data_id = id; }

	eq::base::UUID const &getResourceManagerID( void ) const
	{ return _resource_man_id; }

	void setResourceManagerID( eq::base::UUID const &id )
	{ _resource_man_id = id; }

	/// Used by master to copy the necessary data for synchronization
	void copySettings( vl::SettingsRefPtr settings, vl::ResourceManager *resource_man );

	std::vector<std::string> const &getResources( void ) const
	{ return _resources; }

	/// Logging
	std::string getOgreLogFilePath( void ) const;

	std::string const &getProjectName( void ) const
	{ return _project_name; }

	/// Returns a flags of around which axes the camera rotations are allowed
	/// Fist bit is the x axis, second y axis, third z axis
	uint32_t getCameraRotationAllowed() const
	{ return _camera_rotations_allowed; }

protected :
	virtual void getInstanceData( co::DataOStream& os );
	virtual void applyInstanceData( co::DataIStream& is );

	std::string _log_dir;

	// Resource paths used
	std::vector<std::string> _resources;

	std::string _project_name;

	eq::base::UUID _frame_data_id;

	eq::base::UUID _resource_man_id;

	uint32_t _camera_rotations_allowed;
};

}	// namespace eqOgre

#endif //EQ_OGRE_SETTINGS_HPP
