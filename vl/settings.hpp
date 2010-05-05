/*	Joonatan Kuosa
 *	2010-05
 *
 *	Settings for the scene.
 *	Has settings for both ogre, sceneloader and equalizer.
 */
#ifndef VL_SETTINGS_HPP
#define VL_SETTINGS_HPP

#include <string>

#include "base/args.hpp"
#include "base/rapidxml.hpp"
#include "base/filesystem.hpp"

namespace vl
{

class Settings
{
	public :
		Settings( void );

		~Settings( void );

		fs::path const &getRootPath( void ) const
		{ return _root_path; }

		fs::path const &getFilePath( void ) const
		{ return _file_path; }

		fs::path const &getEqConfigPath( void ) const
		{ return _eq_config; }

		fs::path const &getOgrePluginsPath( void ) const
		{ return _plugin_file; }
		
		fs::path const &getOgreResourcePath( void ) const
		{ return _resource_file; }

		fs::path const &getScenePath( void ) const
		{ return _scene_file; }

		vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		void setExePath( std::string const &path );

		// TODO we need both fs::path, std::string path and relative to root path
		// Supports both absolute and relative paths (current directory)
		void setRootPath( std::string const &path )
		{ _root_path = path; }

		// Supports both absolute and relative paths (root directory)
		void setEqConfigPath( std::string const &path );

		void setOgrePluginsPath( std::string const &path );

		void setOgreResourcePath( std::string const &path );

		void setScenePath( std::string const &path );

		friend class SettingsSerializer;
		friend class SettingsDeserializer;

	private :
		fs::path _exe_path;
		fs::path _root_path;
		fs::path _file_path;
		fs::path _eq_config;
		fs::path _scene_file;
		fs::path _plugin_file;
		fs::path _resource_file;
		vl::Args _eq_args;

};	// class Settings

class SettingsSerializer
{
	public :
		SettingsSerializer( Settings *settings );
 
		~SettingsSerializer( void );

		void readFile( std::string const &file_path );

	protected :
		void processConfig( rapidxml::xml_node<>* XMLNode );

		void processPath( rapidxml::xml_node<>* XMLNode );

		void processPlugins( rapidxml::xml_node<>* XMLNode );

		void processResources( rapidxml::xml_node<>* XMLNode );

		void processEqc( rapidxml::xml_node<>* XMLNode );

		void processScene( rapidxml::xml_node<>* XMLNode );

		std::string getAttrib( rapidxml::xml_node<>* XMLNode,
				std::string const &attrib, std::string const &defaul_value );

		Settings *_settings;

		// file content needed for rapidxml
		char *xml_data;
};

class SettingsDeserializer
{
	public :
		SettingsDeserializer( Settings *settings );

		~SettingsDeserializer( void );

		void writeFile( std::string const &file_path );

	protected :

};

}	// namespace vl

#endif
