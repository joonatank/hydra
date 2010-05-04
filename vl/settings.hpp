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

namespace vl
{

class Settings
{
	public :
		Settings( void );

		~Settings( void );

		std::string const &getOgrePluginsPath( void )
		{
			return _plugin_file;
		}
		
		std::string const &getOgreResourcePath( void )
		{
			return _resource_file;
		}

		std::string const &getScenePath( void )
		{
			return _scene_file;
		}

		vl::Args const &getEqArgs( void )
		{
			return _eq_args;
		}

		friend class SettingsSerializer;
		friend class SettingsDeserializer;
	private :
		std::string _root_path;
		std::string _file_path;
		std::string _eq_config;
		std::string _scene_file;
		std::string _plugin_file;
		std::string _resource_file;
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
