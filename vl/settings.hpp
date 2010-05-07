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

		fs::path const &getFilePath( void ) const
		{ return _file_path; }

		fs::path const &getEqConfigPath( void ) const
		{ return _eq_config; }

		fs::path const &getOgrePluginsPath( void ) const
		{ return _plugin_file; }
		
		std::vector<fs::path> getOgreResourcePaths( void ) const
		{
			std::vector<fs::path> tmp;
			for( size_t i = 0; i < _roots.size(); ++i )
			{
				for( size_t j = 0; j < _roots.at(i).nResources(); ++j )
				{
					tmp.push_back( _roots.at(i).getResource(j) );
				}
			}
			return tmp;
		}

		std::string const &getScene( void ) const
		{ return _scene; }

		vl::Args &getEqArgs( void )
		{ return _eq_args; }

		vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		void setExePath( std::string const &path );

		// Supports both absolute and relative paths (root directory)
		void setEqConfigPath( std::string const &path );

		void setOgrePluginsPath( std::string const &path );

		void setScene( std::string const &path );

		friend class SettingsSerializer;
		friend class SettingsDeserializer;

		// Settings root path
		struct Root
		{
			Root( std::string const &str )
				: path( str )
			{
			}

			Root( void )
			{}

			void setPath( fs::path const &str )
			{
				path = str;
			}

			fs::path getResource( size_t index ) const
			{
				return path / resources.at(index);
			}

			size_t nResources( void ) const
			{ return resources.size(); }

			fs::path path;

			std::vector<fs::path> resources;
		};

		void addRoot( Root const &root )
		{ _roots.push_back(root); }

		Root &getRoot( size_t index )
		{ return _roots.at(index); }

		Root const &getRoot( size_t index ) const
		{ return _roots.at(index); }

		size_t nRoots( void ) const
		{ return _roots.size(); }

	private :
		// All the paths
		std::vector<Settings::Root> _roots;
		fs::path _exe_path;
		fs::path _file_path;
		fs::path _eq_config;
		std::string _scene;
		fs::path _plugin_file;
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

		void processRoot( rapidxml::xml_node<>* XMLNode );

		void processPlugins( rapidxml::xml_node<>* XMLNode,
				vl::Settings::Root const &root = vl::Settings::Root() );

		void processResources( rapidxml::xml_node<>* XMLNode,
				vl::Settings::Root &root );

		void processEqc( rapidxml::xml_node<>* XMLNode,
				vl::Settings::Root const &root = vl::Settings::Root() );

		void processScene( rapidxml::xml_node<>* XMLNode,
				vl::Settings::Root const &root = vl::Settings::Root() );

		std::string getAttrib( rapidxml::xml_node<>* XMLNode,
				std::string const &attrib, std::string const &defaul_value );

		Settings *_settings;

		// file content needed for rapidxml
		vl::FileString xml_data;
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
