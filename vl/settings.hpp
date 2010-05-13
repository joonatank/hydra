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
		// Settings root path
		struct Root
		{
			Root( std::string const &nam = std::string(),
				  std::string const &p = std::string() )
				: name( nam ), path( p )
			{
			}

			void setPath( fs::path const &str )
			{
				path = str;
			}
/*
			// Implicit casting
			operator fs::path() const
			{
				return path;
			}
*/
			fs::path getPath( void ) const
			{
				return path;
			}
			
			std::string name;
			fs::path path;
		};

		struct Resources
		{
			Resources( std::string const &fil = std::string(), Settings::Root *r = 0 )
				: file(fil), root(r)
			{}
/*
			// Implicit casting
			operator fs::path() const
			{
				return (fs::path)(*root) / file;
			}
*/
			fs::path getPath( void ) const
			{
				if( root )
				{ return root->getPath() / file; }
				else
				{ return file; }
			}
			
			std::string file;
			Root *root;
		};

		struct Plugins
		{
			Plugins( std::string const &fil = std::string(), Settings::Root *r = 0 )
				: file(fil), root(r)
			{}

			fs::path getPath( void ) const
			{
				if( root )
				{ return root->getPath() / file; }
				else
				{ return file; }
			}
			std::string file;
			Root *root;
		};

		struct Eqc
		{
			Eqc( std::string const &fil = std::string(), Root *r = 0 )
				: file(fil), root(r)
			{}

			fs::path getPath( void ) const
			{
				if( root )
				{ return root->getPath() / file; }
				else
				{ return file; }
			}
			
			std::string file;
			Root *root;
		};

		struct Scene
		{
			Scene( std::string const &fil = std::string(),
				   std::string const &n = std::string(),
				   std::string const &attach = std::string(),
				   std::string typ = std::string() )
				: file(fil), name(n), attach_node(attach), type(typ)
			{}

			std::string file;
			std::string name;
			std::string attach_node;
			std::string type;
		};

		Settings( void );

		~Settings( void );

		fs::path const &getFilePath( void ) const
		{ return _file_path; }

		fs::path getEqConfigPath( void ) const
		{ return _eq_config.getPath(); }

		fs::path getOgrePluginsPath( void ) const
		{ return _plugins.getPath(); }
		
		std::vector<fs::path> getOgreResourcePaths( void ) const
		{
			std::vector<fs::path> tmp;
			for( size_t i = 0; i < _resources.size(); ++i )
			{
				Settings::Resources const &resource = _resources.at(i);
				tmp.push_back( resource.getPath() );
			}
			return tmp;
		}

		std::vector<Settings::Scene> const &getScenes( void ) const
		{ return _scenes; }

		vl::Args &getEqArgs( void )
		{ return _eq_args; }

		vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		void setExePath( std::string const &path );

		// Supports both absolute and relative paths (root directory)
		void setEqConfig( Settings::Eqc const &eqc );

		void addPlugins( Settings::Plugins const &plugins );

		void addResources( Settings::Resources const &resource );

		void addScene( Settings::Scene const &scene );
		
		friend class SettingsSerializer;
		friend class SettingsDeserializer;

		Root *findRoot( std::string const &name )
		{
			for( size_t i = 0; i < _roots.size(); ++i )
			{
				if( _roots.at(i).name == name )
				{
					return &(_roots.at(i));
				}
			}

			return 0;
		}
		
		void addRoot( Root const &root )
		{ _roots.push_back(root); }

		Root &getRoot( size_t index )
		{ return _roots.at(index); }

		Root const &getRoot( size_t index ) const
		{ return _roots.at(index); }

		size_t nRoots( void ) const
		{ return _roots.size(); }

		void clear( void )
		{
			_roots.clear();
			_exe_path.clear();
			_file_path.clear();
			_eq_config = Eqc();
			_scenes.clear();
			_plugins = Plugins();
			_resources.clear();
			_eq_args = vl::Args();
		}
	private :

		void updateArgs( void );

		// All the paths
		std::vector<Settings::Root> _roots;
		fs::path _exe_path;
		fs::path _file_path;
		Eqc _eq_config;
		
		std::vector<Settings::Scene> _scenes;
		Settings::Plugins _plugins;
		std::vector<Settings::Resources> _resources;
		vl::Args _eq_args;

};	// class Settings

class SettingsSerializer
{
	public :
		SettingsSerializer( Settings *settings );
 
		~SettingsSerializer( void );

		// Read data from a file path.
		void readFile( std::string const &file_path );

		// Read data from string buffer. Buffer is not modified.
		void readData( std::string const &xml_data );

		// Read data from char buffer. Will modify the buffer.
		// And the xml_data will be freed after it's used.
		// xml_data should be valid NULL terminated string.
		void readData( char *xml_data );
		
	protected :
		void processConfig( rapidxml::xml_node<>* XMLNode );

		void processRoot( rapidxml::xml_node<>* XMLNode );

		void processPlugins( rapidxml::xml_node<>* XMLNode );

		void processResources( rapidxml::xml_node<>* XMLNode );

		void processEqc( rapidxml::xml_node<>* XMLNode );

		void processScene( rapidxml::xml_node<>* XMLNode );

		std::string getAttrib( rapidxml::xml_node<>* XMLNode,
				std::string const &attrib, std::string const &defaul_value );

		Settings::Root *getRootAttrib( rapidxml::xml_node<>* XMLNode )
		{
			rapidxml::xml_attribute<> *attrib = XMLNode->first_attribute( "root" );
			if( attrib )
			{
				std::string root_name = attrib->value();
				if( root_name.empty() )
				{ return 0; }
				
				Settings::Root *root = _settings->findRoot( root_name );

				// Because there is not two layered processing, e.g. we don't
				// first read the xml file for all the elements and then
				// map names to pointers all not found names are errors.
				if( !root )
				{ throw vl::invalid_xml( "vl::SettingsSerailizer::processResources" ); }

				return root;
			}
			return 0;
		}

		// Read data from FileString _xml_data.
		void readData( );

		Settings *_settings;

		// file content needed for rapidxml
		vl::FileString *_xml_data;
};

}	// namespace vl

#endif
