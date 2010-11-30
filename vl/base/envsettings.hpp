/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	2010-11-29 Added camera rotations to env file
 *	moved ref ptr definition to typedefs.hpp
 */

#ifndef ENVSETTINGS_H
#define ENVSETTINGS_H

#include <string>
#include <vector>
#include <iostream>

#include <stdint.h>

#include "filesystem.hpp"
#include "rapidxml.hpp"
#include "typedefs.hpp"

namespace vl
{

class EnvSettings
{

public :

	struct Tracking
	{
		Tracking( std::string const &file_name, bool u = "true" )
			: file(file_name), use(u)
		{}

		std::string file;
		bool use;
	};

	/// Constructor
	EnvSettings( void );

	/// Destructor
	~EnvSettings( void );

	/// Clear the settings structure to default values
	void clear( void );


	/// File path
	std::string const &getFile( void ) const
	{ return _file_path; }

	void setFile( std::string const &file )
	{ _file_path = file; }

	///// EQC /////////////////////////////////////////////////////
	/// get the equalizer config file
	std::string const &getEqc( void ) const
	{ return _eqc; }

	/// set the equalizer config file
	void setEqc( std::string const &e )
	{ _eqc = e; }

	/// Get the absoulute path to eqc file
	std::string getEqcFullPath( void ) const;

	std::string getPluginsDirFullPath( void ) const;

	///// PLUGINS /////////////////////////////////////////////////
	/// get vector containing plugin names and which are active
	std::vector<std::pair<std::string, bool> > const &getPlugins( void ) const
	{ return _plugins; }

	/// Add a plugin by name and boolean wether it's in use or not
	/// Checks that the same plugin is not added twice, NOP if the plugin is
	/// already in the plugins stack.
	void addPlugin( std::pair<std::string, bool> const &plugin );

	/// set the plugin named pluginName use to on or off
	/// returns true if plugin found
	/// prints an error message and returns false if no such plugin found
	bool pluginOnOff( std::string const &pluginName, bool newState );

	///// TRACKING /////////////////////////////////////////////////
	/// Returns a vector of tracking files
	std::vector<Tracking> const &getTracking( void ) const
	{ return _tracking; }

	/// Adds a tracking file to the tracking file stack
	/// Checks that the same file tracking file is not added twice, NOP if the
	/// tracking file is already in the stack.
	void addTracking( Tracking const &track );

	/// Remove a tracking file from the stack
	void removeTracking( std::string const &track );

	void removeTracking( Tracking const &track );

	/// Returns a flags of around which axes the camera rotations are allowed
	/// Fist bit is the x axis, second y axis, third z axis
	uint32_t getCameraRotationAllowed( void ) const
	{ return _camera_rotations_allowed; }

	void setCameraRotationAllowed( uint32_t const flags )
	{ _camera_rotations_allowed = flags; }

private :

	std::string _file_path;

	std::string _eqc;

	std::vector<std::pair<std::string, bool> > _plugins;
	std::vector<Tracking> _tracking;

	uint32_t _camera_rotations_allowed;

};	// class EnvSettings





class EnvSettingsSerializer
{
public :
	/// Will completely over-ride the provided EnvSettings
	/// when XML data is processed.
	/// Any error in processing will result to defaulting EnvSettings.
	EnvSettingsSerializer( EnvSettingsRefPtr );

	~EnvSettingsSerializer( void );

	/// Read data from string buffer. Buffer is not modified.
	bool readString( std::string const &xml_data );


protected :
	bool readXML( void );

	void processConfig( rapidxml::xml_node<>* XMLNode );

	void processPlugins( rapidxml::xml_node<>* XMLNode );

	void processEqc( rapidxml::xml_node<>* XMLNode );

	void processTracking( rapidxml::xml_node<>* XMLNode );

	void processCameraRotations( rapidxml::xml_node<>* XMLNode );

	EnvSettingsRefPtr _envSettings;

	/// file content needed for rapidxml
	char *_xml_data;

};	// class EnvSettingsSerializer

}	// namespace vl

#endif // ENVSETTINGS_H
