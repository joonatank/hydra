/*	Joonatan Kuosa
 *	2010-05
 *
 *	Settings for the scene.
 *	Has settings for
 *	Ogre3D : resources and plugins
 *	equalizer : config file
 *	sceneloader : scenes to load
 *	tracking : config file
 */
#ifndef VL_SETTINGS_HPP
#define VL_SETTINGS_HPP

#include <string>

#include "base/args.hpp"
#include "base/rapidxml.hpp"
#include "base/filesystem.hpp"
#include "base/typedefs.hpp"

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
			{}

			void setPath( fs::path const &str )
			{ path = str; }

			fs::path getPath( void ) const
			{ return path; }
			
			std::string name;
			fs::path path;
		};

		struct Resources
		{
			Resources( std::string const &fil = std::string(), Settings::Root *r = 0 )
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

		struct Tracking
		{
			Tracking( std::string const &fil, Settings::Root *r )
				: file(fil), root(r)
			{
			}

			fs::path getPath( void ) const
			{
				if( root )
				{ return root->getPath() / file; }
				else
				{ return file; }
			}
			
			std::string file;
			Settings::Root *root;
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

		virtual ~Settings( void );

		virtual void setFilePath( std::string const &path )
		{ _file_path = path; }

		virtual fs::path const &getFilePath( void ) const
		{ return _file_path; }

		virtual fs::path getEqConfigPath( void ) const
		{ return _eq_config.getPath(); }

		virtual fs::path getOgrePluginsPath( void ) const
		{ return _plugins.getPath(); }
		
		virtual std::vector<fs::path> getOgreResourcePaths( void ) const;

		virtual std::vector<Settings::Scene> const &getScenes( void ) const
		{ return _scenes; }

		virtual vl::Args &getEqArgs( void )
		{ return _eq_args; }

		virtual vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		virtual void setExePath( std::string const &path );

		// Supports both absolute and relative paths (root directory)
		virtual void setEqConfig( Settings::Eqc const &eqc );

		virtual void addPlugins( Settings::Plugins const &plugins );

		virtual void addResources( Settings::Resources const &resource );

		virtual void addScene( Settings::Scene const &scene );

		virtual std::vector<Settings::Tracking> const &getTracking( void )
		{ return _tracking; }

		virtual void addTracking( Settings::Tracking const &track )
		{ _tracking.push_back( track ); }

		virtual Root *findRoot( std::string const &name );

		virtual void addRoot( Root const &root )
		{ _roots.push_back(root); }

		virtual Root &getRoot( size_t index )
		{ return _roots.at(index); }

		virtual Root const &getRoot( size_t index ) const
		{ return _roots.at(index); }

		virtual size_t nRoots( void ) const
		{ return _roots.size(); }

		virtual void clear( void );

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
		std::vector<Settings::Tracking> _tracking;
		vl::Args _eq_args;

};	// class Settings

class SettingsSerializer
{
	public :
		SettingsSerializer( SettingsRefPtr settings );
 
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

		void processTracking( rapidxml::xml_node<>* XMLNode );

		std::string getAttrib( rapidxml::xml_node<>* XMLNode,
				std::string const &attrib, std::string const &defaul_value );

		Settings::Root *getRootAttrib( rapidxml::xml_node<>* XMLNode );

		// Read data from FileString _xml_data.
		void readData( );

		vl::SettingsRefPtr _settings;

		// file content needed for rapidxml
		vl::FileString *_xml_data;
};

}	// namespace vl

#endif
