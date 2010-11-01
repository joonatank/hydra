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

#include <OgreVector3.h>
#include <OgreQuaternion.h>

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

			void setPath( std::string const &str )
			{ path = str; }

			std::string getPath( void ) const
			{ return path; }
			
			std::string name;
			std::string path;
		};

		struct Resources
		{
			Resources( std::string const &fil = std::string(), Settings::Root *r = 0 )
				: file(fil), root(r)
			{}

			std::string getPath( void ) const
			{
				if( root && !root->getPath().empty() )
				{ return root->getPath() + "/" + file; }
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

			std::string getPath( void ) const
			{
				if( root && !root->getPath().empty() )
				{ return root->getPath() + "/" + file; }
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

			std::string getPath( void ) const
			{
				if( root && !root->getPath().empty() )
				{ return root->getPath() + "/" + file; }
				else
				{ return file; }
			}
			
			std::string file;
			Root *root;
		};

		struct Tracking
		{
			Tracking( std::string const &fil = std::string(), Settings::Root *r = 0)
				: file(fil), root(r)
			{
			}

			std::string getPath( void ) const
			{
				if( root && !root->getPath().empty() )
				{ return root->getPath() + "/" + file; }
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

		Settings( std::string const &name );

		virtual ~Settings( void );

		void setName( std::string const &name )
		{ _project_name = name; }

		std::string const &getName( void )
		{ return _project_name; }

		void setLogDir( std::string const &dir )
		{ _log_dir = dir; }
		
		std::string const &getLogDir( void )
		{ return _log_dir; }

		void setFilePath( std::string const &path )
		{ _file_path = path; }

		std::string const &getFilePath( void ) const
		{ return _file_path; }

		virtual std::string getEqConfigPath( void ) const
		{ return _eq_config.getPath(); }

		virtual std::string getOgrePluginsPath( void ) const
		{ return _plugins.getPath(); }
		
		virtual std::vector<std::string> getOgreResourcePaths( void ) const;

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

		bool trackerOn( void )
		{ return _tracker_address.size() != 0 ; }

		std::string const &getTrackerAddress( void )
		{ return _tracker_address; }

		void setTrackerAddress( std::string const &add )
		{ _tracker_address = add; }

		Ogre::Vector3 const &getTrackerDefaultPosition( void )
		{
			return _tracker_default_pos;
		}
		
		void setTrackerDefaultPosition( Ogre::Vector3 const &v )
		{
			_tracker_default_pos = v;
		}

		Ogre::Quaternion const &getTrackerDefaultOrientation( void )
		{
			return _tracker_default_orient;
		}
		
		void setTrackerDefaultOrientation( Ogre::Quaternion const &q )
		{
			_tracker_default_orient = q;
		}

	protected :

		void updateArgs( void );

		// Project name
		std::string _project_name;

		// Log directory
		std::string _log_dir;

		// All the paths
		std::vector<Settings::Root> _roots;
		std::string _exe_path;
		std::string _file_path;
		Eqc _eq_config;
		
		std::vector<Settings::Scene> _scenes;
		Settings::Plugins _plugins;
		std::vector<Settings::Resources> _resources;
		std::vector<Settings::Tracking> _tracking;
		vl::Args _eq_args;

		std::string _tracker_address;
		Ogre::Vector3 _tracker_default_pos;
		Ogre::Quaternion _tracker_default_orient;

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

		// Read data from char buffer.
		// xml_data should be valid NULL terminated string.
		void readData( char const *xml_data );
		
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

		Ogre::Vector3 parseVector3( rapidxml::xml_node<>* XMLNode, 
			Ogre::Vector3 const &default_value = Ogre::Vector3::ZERO );

		Ogre::Quaternion parseQuaternion( rapidxml::xml_node<>* XMLNode, 
			Ogre::Quaternion const &default_value = Ogre::Quaternion::IDENTITY );

		// Read data from FileString _xml_data.
		void readData( );

		vl::SettingsRefPtr _settings;

		// file content needed for rapidxml
		char *_xml_data;
};

}	// namespace vl

#endif
