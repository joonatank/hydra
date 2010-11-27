/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Global function to create the settings from command-line arguments
 *
 *	Distributed settings structure
 */
#ifndef EQ_OGRE_SETTINGS_HPP
#define EQ_OGRE_SETTINGS_HPP

#include <eq/net/object.h>

// Needed for ProjSettings::Scene
#include "base/projsettings.hpp"

// Needed for RefPtrs
#include "base/typedefs.hpp"

namespace eqOgre
{

vl::SettingsRefPtr getSettings( int argc, char **argv );

/// The settings for the current project that needs to be distributed
/// Minimal version of vl::Settings that does not contain information on the
/// links between project and scenes.
/// Contains only data needed by the slaves e.g. contains list of scenes
/// but contains no scripts.
/// Also contains runtime parameters that are not in vl::Settings.
class DistributedSettings : public eq::net::Object
{
public :
	DistributedSettings( void );

	virtual ~DistributedSettings( void )
	{}

	uint32_t getFrameDataID( void ) const
	{ return _frame_data_id; }

	void setFrameDataID( uint32_t const id )
	{ _frame_data_id = id; }

	/// Used by master to copy the necessary data for synchronization
	void copySettings( vl::SettingsRefPtr settings );

	std::vector<std::string> const &getResources( void ) const
	{ return _resources; }

	std::vector<vl::ProjSettings::Scene> const &getScenes( void ) const
	{ return _scenes; }

	/// Logging
	std::string getOgreLogFilePath( void ) const;

	std::string const &getProjectName( void ) const
	{ return _project_name; }

protected :
	virtual void getInstanceData( eq::net::DataOStream& os );
	virtual void applyInstanceData( eq::net::DataIStream& is );

	std::string _log_dir;

	// Resource paths used
	std::vector<std::string> _resources;

	// Scenes to load
	std::vector<vl::ProjSettings::Scene>  _scenes;

	std::string _project_name;

	uint32_t _frame_data_id;
};

/// Serialize scenes
// TODO this should use a custom SceneStructure that contains the whole xml file
// not the filename. And which doesn't contain useless use and changed attributes.
eq::net::DataOStream &
operator<<(vl::ProjSettings::Scene const &s, eq::net::DataOStream& os);

eq::net::DataIStream &
operator>>(vl::ProjSettings::Scene &s, eq::net::DataIStream& is);

}	// namespace eqOgre

#endif //EQ_OGRE_SETTINGS_HPP
