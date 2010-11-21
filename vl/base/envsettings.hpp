/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 */

#ifndef ENVSETTINGS_H
#define ENVSETTINGS_H

#include <string>
#include <vector>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "filesystem.hpp"
#include "rapidxml.hpp"

namespace vl
{

class EnvSettings
{

public :
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
	std::string getEqcFullPath( void ) const
	{
		if( getEqc().empty() )
		{ return std::string(); }

		fs::path env_path = getFile();
		fs::path env_dir = env_path.parent_path();
		fs::path path =  env_dir / "eqc" / getEqc();

		return path.file_string();
	}

	std::string getPluginsDirFullPath( void ) const
	{
		fs::path env_path = getFile();
		fs::path env_dir = env_path.parent_path();
		fs::path path =  env_dir / "plugins";

		return path.file_string();
	}

	///// PLUGINS /////////////////////////////////////////////////
	/// get vector containing plugin names and which are active
	std::vector<std::pair<std::string, bool> > const &getPlugins( void ) const
	{ return _plugins; }

	/// add a plugin by name and boolean wether it's in use or not
	// TODO should really check that the same plugin is not added twice
	void addPlugin( std::pair<std::string, bool> const &plugin )
	{ _plugins.push_back(plugin); }

	/// set the plugin named pluginName use to on or off
	/// returns true if plugin found
	/// prints an error message and returns false if no such plugin found
	bool pluginOnOff( std::string const &pluginName, bool newState );

	///// TRACKING /////////////////////////////////////////////////
	/// Returns a vector of tracking files
	std::vector<std::string> const &getTracking( void ) const
	{ return _tracking; }

	/// Adds a tracking file to the tracking file stack
	// TODO this should really check that we don't add the same file twice
	void addTracking( std::string const &track )
	{ _tracking.push_back( track ); }

	/// Remove a tracking file from the stack
	void removeTracking( std::string const &track );

private :

	std::string _file_path;

	std::string _eqc;

	std::vector<std::pair<std::string, bool> > _plugins;
	std::vector<std::string> _tracking;

};	// class EnvSettings

typedef boost::shared_ptr< EnvSettings > EnvSettingsRefPtr;





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


	EnvSettingsRefPtr _envSettings;

	/// file content needed for rapidxml
	char *_xml_data;

};	// class EnvSettingsSerializer

typedef boost::shared_ptr< EnvSettingsSerializer > EnvSettingsSerializerRefPtr;

}	// namespace vl

#endif // ENVSETTINGS_H
