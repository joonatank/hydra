#ifndef VL_TEST_FIXTURES_HPP
#define VL_TEST_FIXTURES_HPP

#include "base/rapidxml_print.hpp"
#include "base/typedefs.hpp"
#include "base/exceptions.hpp"
#include "settings.hpp"

#include <OgreStringConverter.h>

std::string const PROJECT( "test_settings" );

struct SettingsFixture
{
	SettingsFixture( void )
		: settings( PROJECT ), ser(0), doc(), config(0)
	{
		vl::SettingsRefPtr settings_ptr( &settings, vl::null_deleter() );
		ser = new vl::SettingsSerializer( settings_ptr );
		
		config = doc.allocate_node(rapidxml::node_element, "config" );
		doc.append_node(config);
	}

	~SettingsFixture( void )
	{
		delete ser;
	}

	void readXML( void )
	{
		data.clear();
		rapidxml::print(std::back_inserter(data), doc, 0);
		// Print to string so we can use SettingsSerializer for the data
		BOOST_TEST_MESSAGE( data );
		ser->readData( data );
	}

	bool checkEmpty( void )
	{
		
		return ( settings.nRoots() == 0
			&& settings.getEqConfigPath().empty()
			&& settings.getFilePath().empty()
			&& settings.getOgrePluginsPath().empty()
			&& settings.getOgreResourcePaths().empty()
			&& settings.getScenes().empty() );
	}

	rapidxml::xml_node<> *createRoot( std::string const &name,
									  std::string const &path )
	{
		char *c_path = doc.allocate_string( path.c_str() );
		char *c_name = doc.allocate_string( name.c_str() );
		
		rapidxml::xml_node<> *root
			= doc.allocate_node(rapidxml::node_element, "root" );
		rapidxml::xml_node<> *node
			= doc.allocate_node(rapidxml::node_element, "path", c_path );
		root->append_node(node);

		rapidxml::xml_attribute<> *attr = doc.allocate_attribute( "name", c_name );
		root->append_attribute( attr );
		
		return root;
	}
	
	rapidxml::xml_node<> *createVector3( Ogre::Vector3 const &v )
	{
		rapidxml::xml_node<> *node
			= doc.allocate_node(rapidxml::node_element, "vector" );

		char *c_x = doc.allocate_string( Ogre::StringConverter::toString( v.x ).c_str() );
		char *c_y = doc.allocate_string( Ogre::StringConverter::toString( v.y ).c_str() );
		char *c_z = doc.allocate_string( Ogre::StringConverter::toString( v.z ).c_str() );

		node->append_attribute( doc.allocate_attribute( "x", c_x ) );
		node->append_attribute( doc.allocate_attribute( "y", c_y ) );
		node->append_attribute( doc.allocate_attribute( "z", c_z ) );

		return node;
	}

	rapidxml::xml_node<> *createQuaternion( Ogre::Quaternion const &q )
	{
		rapidxml::xml_node<> *node
			= doc.allocate_node(rapidxml::node_element, "quaternion" );
		
		char *c_qw = doc.allocate_string( Ogre::StringConverter::toString( q.w ).c_str() );
		char *c_qx = doc.allocate_string( Ogre::StringConverter::toString( q.x ).c_str() );
		char *c_qy = doc.allocate_string( Ogre::StringConverter::toString( q.y ).c_str() );
		char *c_qz = doc.allocate_string( (Ogre::StringConverter::toString( q.z )).c_str() );

		node->append_attribute( doc.allocate_attribute( "w", c_qw ) );
		node->append_attribute( doc.allocate_attribute( "x", c_qx ) );
		node->append_attribute( doc.allocate_attribute( "y", c_qy ) );
		node->append_attribute( doc.allocate_attribute( "z", c_qz ) );

		return node;
	}

	rapidxml::xml_node<> *createTracking( std::string const &tracker_name,
										  bool on = false,
										  Ogre::Vector3 pos = Ogre::Vector3::ZERO,
										  Ogre::Quaternion orient = Ogre::Quaternion::IDENTITY
										  )
	{
		char *c_tracker = doc.allocate_string( tracker_name.c_str() );
//		char *c_pos = doc.allocate_string( Ogre::StringConverter::toString( pos ).c_str() );
//		char *c_orient = doc.allocate_string( Ogre::StringConverter::toString( orient ).c_str() );
		
		char *val_on;
		if( on )
		{ val_on = doc.allocate_string( "true" ); }
		else
		{ val_on = doc.allocate_string( "false" ); }

		rapidxml::xml_node<> *tracking
			= doc.allocate_node(rapidxml::node_element, "tracking" );

		rapidxml::xml_node<> *node
			= doc.allocate_node(rapidxml::node_element, "tracker_name", c_tracker );
		tracking->append_node( node );
		
		rapidxml::xml_attribute<> *attr
			= doc.allocate_attribute( "on", val_on );
		tracking->append_attribute( attr );
		
		// Add vector elem
		node = doc.allocate_node(rapidxml::node_element, "default_position" );
		tracking->append_node( node );
		node->append_node( createVector3( pos ) );

		// Add quaternion elem
		node = doc.allocate_node(rapidxml::node_element, "default_orientation" );
		tracking->append_node( node );
		node->append_node( createQuaternion( orient ) );
		
		return tracking;
	}
	
	rapidxml::xml_node<> *createScene( std::string const &name,
									   std::string const &file,
									   std::string const &type = std::string(),
									   std::string const &attach = std::string() )
	{
		char *c_name = doc.allocate_string( name.c_str() );
		char *c_file = doc.allocate_string( file.c_str() );
		char *c_type = doc.allocate_string( type.c_str() );
		char *c_attach = doc.allocate_string( attach.c_str() );

		rapidxml::xml_node<> *scene
			= doc.allocate_node(rapidxml::node_element, "scene" );
		rapidxml::xml_attribute<> *attr
			= doc.allocate_attribute( "name", c_name );
		scene->append_attribute(attr);
			
		rapidxml::xml_node<> *node
			= doc.allocate_node(rapidxml::node_element, "file", c_file );
		scene->append_node( node );

		node = doc.allocate_node(rapidxml::node_element, "type", c_type );
		scene->append_node( node );

		node = doc.allocate_node(rapidxml::node_element, "attach", c_attach );
		scene->append_node( node );

		return scene;
	}
	
	std::string data;
	vl::Settings settings;
	vl::SettingsSerializer* ser;
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> *config;
};


#endif
