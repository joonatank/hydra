#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include "settings.hpp"
#include "base/filesystem.hpp"
#include "base/exceptions.hpp"
#include "base/rapidxml_print.hpp"

#include <iostream>

// Invalid xml file, without config element
BOOST_AUTO_TEST_CASE( invalid_xml )
{
	rapidxml::xml_document<> doc;

	// First root node
	rapidxml::xml_node<> *root = doc.allocate_node(rapidxml::node_element, "root" );
	doc.append_node( root );
	rapidxml::xml_attribute<> *attr = doc.allocate_attribute("name", "data");
	root->append_attribute(attr);
	rapidxml::xml_node<> *path = doc.allocate_node(rapidxml::node_element, "path", "local/work" );
	root->append_node(path);

	vl::Settings settings;
	vl::SettingsSerializer ser(&settings);

	// Create the test data
	std::string data;
	rapidxml::print(std::back_inserter(data), doc, 0);

	// Parse
	BOOST_CHECK_THROW( ser.readData( data ), vl::invalid_xml );
}

struct SettingsFixture
{
	SettingsFixture( void )
		: settings(), ser(0), doc(), config(0)
	{
		ser = new vl::SettingsSerializer(&settings);
		
		config = doc.allocate_node(rapidxml::node_element, "config" );
		doc.append_node(config);
	}

	~SettingsFixture( void )
	{
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
	
	std::string data;
	vl::Settings settings;
	vl::SettingsSerializer* ser;
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> *config;
};

// TODO files are unreliable we should generate the xml file here
// bit by bit and check that each individual element is processed correctly
// We can then save these files and read them from disk if necessary.
BOOST_FIXTURE_TEST_SUITE( ConfigUnitTests, SettingsFixture )

BOOST_AUTO_TEST_CASE( test_config )
{
	std::string data;
	rapidxml::print(std::back_inserter(data), doc, 0);
	BOOST_TEST_MESSAGE( data );
	BOOST_CHECK_NO_THROW( ser->readData( data ) );
}

BOOST_AUTO_TEST_CASE( test_root_elem )
{	
	// First root node
	rapidxml::xml_node<> *root = doc.allocate_node(rapidxml::node_element, "root" );
	config->append_node( root );

	// Invalid root, missing path element
	rapidxml::xml_attribute<> *name = doc.allocate_attribute("name", "data");
	root->append_attribute(name);
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );
	
	// Invalid root, missing name attribute
	root->remove_attribute( name );
	rapidxml::xml_node<> *path = doc.allocate_node(rapidxml::node_element, "path", "local/work" );
	root->append_node(path);
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	// Valid root, name attribute and path element
	root->append_attribute( name );
	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK_EQUAL( settings.nRoots(), 1u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(0);
		BOOST_CHECK_EQUAL( root.name, "data" );
		BOOST_CHECK_EQUAL( root.path.file_string(), "local/work" );
	}
}

BOOST_AUTO_TEST_CASE( test_plugins_elem )
{
	// First root node
	rapidxml::xml_node<> *plugins
		= doc.allocate_node(rapidxml::node_element, "plugins" );
	config->append_node( plugins );
	rapidxml::xml_node<> *path
		= doc.allocate_node(rapidxml::node_element, "file", "plugins.cfg" );
	plugins->append_node(path);

	// Test invalid plugins xml, containing root reference without root element
	rapidxml::xml_attribute<> *attr = doc.allocate_attribute("root", "data");
	plugins->append_attribute( attr );
	
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	// Test working plugins xml
	plugins->remove_attribute( attr );
	
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.nRoots(), 0u );
	BOOST_CHECK_EQUAL( settings.getOgrePluginsPath().file_string(),
					   "plugins.cfg" );
}

BOOST_AUTO_TEST_CASE( test_resources_elem )
{
	rapidxml::xml_node<> *resources
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( resources );

	// Test invalid resource xml, without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );
	
	// Test invalid resource xml, with empty file element
	rapidxml::xml_node<> *invalid_file
		= doc.allocate_node(rapidxml::node_element, "file", "" );
	resources->append_node(invalid_file);

	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );
	
	resources->remove_node( invalid_file );

	// Test invalid resource xml, with reference to root that does not exist
	rapidxml::xml_attribute<> *attr = doc.allocate_attribute("root", "data");
	resources->append_attribute( attr );
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	resources->remove_attribute( attr );

	// Test valid resource xml
	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "resources.cfg" );
	resources->append_node(file);

	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.nRoots(), 0u );
	BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().size(), 1u );
	if( settings.getOgreResourcePaths().size() > 0 )
	{
		BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().at(0).file_string(),
					   "resources.cfg" );
	}
}

BOOST_AUTO_TEST_CASE( test_scene_elem )
{
	rapidxml::xml_node<> *scene
		= doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene );

	// Test invalid scene, without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	// Test valid scene, with file element
	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "T7.scene" );
	scene->append_node( file );
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 1u );
	if( settings.getScenes().size() > 0 )
	{
		vl::Settings::Scene const &scn = settings.getScenes().at(0);
		BOOST_CHECK_EQUAL( scn.file, "T7.scene" );
		BOOST_CHECK( scn.name.empty() );
		BOOST_CHECK( scn.type.empty() );
		BOOST_CHECK( scn.attach_node.empty() );
	}
	settings = vl::Settings();
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 0u );
	
	// Test valid scene, with file element and name attribute
	rapidxml::xml_attribute<> *attr = doc.allocate_attribute("name", "T7");
	scene->append_attribute( attr );
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 1u );
	if( settings.getScenes().size() > 0 )
	{
		vl::Settings::Scene const &scn = settings.getScenes().at(0);
		BOOST_CHECK_EQUAL( scn.file, "T7.scene" );
		BOOST_CHECK_EQUAL( scn.name, "T7" );
		BOOST_CHECK( scn.type.empty() );
		BOOST_CHECK( scn.attach_node.empty() );
	}
	settings = vl::Settings();
	
	// Test valid scene with file and type elements and name attribute
	rapidxml::xml_node<> *type
		= doc.allocate_node(rapidxml::node_element, "type", "release" );
	scene->append_node( type );
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 1u );
	if( settings.getScenes().size() > 0 )
	{
		vl::Settings::Scene const &scn = settings.getScenes().at(0);
		BOOST_CHECK_EQUAL( scn.file, "T7.scene" );
		BOOST_CHECK_EQUAL( scn.name, "T7" );
		BOOST_CHECK_EQUAL( scn.type, "release" );
		BOOST_CHECK( scn.attach_node.empty() );
	}
	settings = vl::Settings();

	// Test valid scene with file, type and attach elements and name attribute
	rapidxml::xml_node<> *attach
		= doc.allocate_node(rapidxml::node_element, "attach", "cave" );
	scene->append_node( attach );
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 1u );
	if( settings.getScenes().size() > 0 )
	{
		vl::Settings::Scene const &scn = settings.getScenes().at(0);
		BOOST_CHECK_EQUAL( scn.file, "T7.scene" );
		BOOST_CHECK_EQUAL( scn.name, "T7" );
		BOOST_CHECK_EQUAL( scn.type, "release" );
		BOOST_CHECK_EQUAL( scn.attach_node, "cave" );
	}
}

BOOST_AUTO_TEST_CASE( eqc_elem )
{
	std::string eqc_file( "1-window.eqc" );
	rapidxml::xml_node<> *eqc
		= doc.allocate_node(rapidxml::node_element, "eqc" );
	config->append_node( eqc );

	// Test invalid eqc, without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	rapidxml::xml_node<> *file
		= doc.allocate_node( rapidxml::node_element, "file", eqc_file.c_str() );
	eqc->append_node( file );

	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getEqConfigPath(), eqc_file );
	vl::Args const &args = settings.getEqArgs();
	BOOST_TEST_MESSAGE( "args = " << args );
	BOOST_CHECK_EQUAL( args.size(), 3u );
	if( args.size() > 2 )
	{
		BOOST_CHECK_EQUAL( args.at(0), "--eq-config" );
		BOOST_CHECK_EQUAL( args.at(1), eqc_file );
		BOOST_CHECK_EQUAL( args.at(2), "\0" );
	}
}

BOOST_AUTO_TEST_CASE( multiple_roots )
{
	// First root node
	rapidxml::xml_node<> *root1 = doc.allocate_node(rapidxml::node_element, "root" );
	config->append_node( root1 );
	rapidxml::xml_node<> *path = doc.allocate_node(rapidxml::node_element, "path", "local/work" );
	root1->append_node(path);
	
	// Second root node
	rapidxml::xml_node<> *root2 = doc.allocate_node(rapidxml::node_element, "root" );
	config->append_node( root2 );
	path = doc.allocate_node(rapidxml::node_element, "path", "local/home" );
	root2->append_node(path);

	// Invalid name references they should be unique
	rapidxml::xml_attribute<> *name1 = doc.allocate_attribute("name", "data");
	root1->append_attribute( name1 );
	rapidxml::xml_attribute<> *name2 = doc.allocate_attribute("name", "data");
	root2->append_attribute( name2 );

	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	name2->value( "media" );
	BOOST_CHECK_NO_THROW( readXML() );

	BOOST_CHECK_EQUAL( settings.nRoots(), 2u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(i);
		if( i == 0 )
		{
			BOOST_CHECK_EQUAL( root.name, "data" );
			BOOST_CHECK_EQUAL( root.path.file_string(), "local/work" );
		}
		else if( i == 1 )
		{
			BOOST_CHECK_EQUAL( root.name, "media" );
			BOOST_CHECK_EQUAL( root.path.file_string(), "local/home" );
		}

		BOOST_TEST_MESSAGE( "root " << i << " path = " << root.path );
	}
}

BOOST_AUTO_TEST_CASE( multiple_resources )
{
	rapidxml::xml_node<> *resources1
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( resources1 );

	rapidxml::xml_node<> *resources2
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( resources2 );

	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "resources.cfg" );
	resources1->append_node(file);
	
	// Test invalid resource xml, one of the resources without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	// Valid resource xml, both resources have file element
	file = doc.allocate_node(rapidxml::node_element, "file", "resources2.cfg" );
	resources2->append_node(file);
	
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().size(), 2u );
	if( settings.getOgreResourcePaths().size() == 2u )
	{
		BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().at(0),
						   "resources.cfg" );
		BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().at(1),
						   "resources2.cfg" );
	}
}

BOOST_AUTO_TEST_CASE( multiple_scenes )
{
	rapidxml::xml_node<> *scene1
		= doc.allocate_node(rapidxml::node_element, "scene" );
	rapidxml::xml_node<> *scene2
		= doc.allocate_node(rapidxml::node_element, "scene" );
	rapidxml::xml_node<> *scene3
		= doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene1 );
	config->append_node( scene2 );
	config->append_node( scene3 );

	// First two scenes are valid, contains file element
	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "T7.scene" );
	scene1->append_node( file );
	file = doc.allocate_node(rapidxml::node_element, "file", "ruukki.scene" );
	scene2->append_node( file );

	// Third scene is invalid, without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_xml );
	BOOST_CHECK( checkEmpty() );

	// All scenes are valid, containing only file elements
	file = doc.allocate_node(rapidxml::node_element, "file", "test.scene" );
	scene3->append_node( file );
	
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 3u );
	for( size_t i = 0; i < settings.getScenes().size(); ++i )
	{
		vl::Settings::Scene const &scn = settings.getScenes().at(i);
		if( i == 0 )
		{ BOOST_CHECK_EQUAL( scn.file, "T7.scene" ); }
		else if( i == 1 )
		{ BOOST_CHECK_EQUAL( scn.file, "ruukki.scene" ); }
		else if( i == 2 )
		{ BOOST_CHECK_EQUAL( scn.file, "test.scene" ); }
		BOOST_CHECK( scn.name.empty() );
		BOOST_CHECK( scn.type.empty() );
		BOOST_CHECK( scn.attach_node.empty() );
	}
	settings = vl::Settings();
	BOOST_CHECK_EQUAL( settings.getScenes().size(), 0u );
}

// TODO write
// Real world use case. We have two roots, two resource files, three scenes,
// one plugins file, one eqc. Everything except scenes need root reference.
BOOST_AUTO_TEST_CASE( root_references )
{
	// First root node
	rapidxml::xml_node<> *root
		= doc.allocate_node(rapidxml::node_element, "root" );
	config->append_node( root );
	rapidxml::xml_node<> *path
		= doc.allocate_node(rapidxml::node_element, "path", "local/work" );
	root->append_node(path);
	rapidxml::xml_attribute<> *attrib = doc.allocate_attribute("name", "data");
	root->append_attribute( attrib );

	// Second root node
	root = doc.allocate_node(rapidxml::node_element, "root" );
	config->append_node( root );
	path = doc.allocate_node(rapidxml::node_element, "path", "local/home" );
	root->append_node(path);
	attrib = doc.allocate_attribute("name", "media");
	root->append_attribute( attrib );

	// First resources node
	rapidxml::xml_node<> *resource
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( resource );
	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "resource.cfg" );
	resource->append_node(file);
	attrib = doc.allocate_attribute("root", "data");
	resource->append_attribute( attrib );

	// Second resources node
	resource
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( resource );
	file = doc.allocate_node(rapidxml::node_element, "file", "resource.cfg" );
	resource->append_node(file);
	attrib = doc.allocate_attribute("root", "media");
	resource->append_attribute( attrib );

	// eqc
	rapidxml::xml_node<> *eqc
		= doc.allocate_node(rapidxml::node_element, "eqc" );
	config->append_node( eqc );
	file = doc.allocate_node(rapidxml::node_element, "file", "1-window.eqc" );
	eqc->append_node(file);
	attrib = doc.allocate_attribute("root", "data");
	eqc->append_attribute( attrib );

	// plugins
	rapidxml::xml_node<> *plugins
		= doc.allocate_node(rapidxml::node_element, "plugins" );
	config->append_node( plugins );
	file = doc.allocate_node(rapidxml::node_element, "file", "plugins.cfg" );
	plugins->append_node(file);
	attrib = doc.allocate_attribute("root", "media");
	plugins->append_attribute( attrib );

	// First scene
	rapidxml::xml_node<> *scene
		= doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene );
	file = doc.allocate_node(rapidxml::node_element, "file", "T7.scene" );
	scene->append_node(file);
	attrib = doc.allocate_attribute("name", "T7");
	scene->append_attribute( attrib );

	// Second scene
	scene = doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene );
	file = doc.allocate_node(rapidxml::node_element, "file", "ruukki.scene" );
	scene->append_node(file);
	attrib = doc.allocate_attribute("name", "ruukki");
	scene->append_attribute( attrib );

	// Third scene
	scene = doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene );
	file = doc.allocate_node(rapidxml::node_element, "file", "ruukki.scene" );
	scene->append_node(file);

	BOOST_CHECK_NO_THROW( readXML() );
	
	// Check roots
	BOOST_REQUIRE_EQUAL( settings.nRoots(), 2u );
	if( settings.nRoots() == 2 )
	{
		// TODO add checking of the roots
	}
	fs::path data_path( "local/work" );
	fs::path media_path( "local/home" );

	BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().size(), 2u );
	if( settings.getOgreResourcePaths().size() == 2 )
	{
		// TODO add checking of the resources
	}

	BOOST_CHECK_EQUAL( settings.getEqConfigPath(),
					   data_path / fs::path("1-window.eqc") );

	BOOST_CHECK_EQUAL( settings.getOgrePluginsPath(),
					   media_path / fs::path("plugins.cfg") );

	BOOST_CHECK_EQUAL( settings.getScenes().size(), 3u );
	if( settings.getScenes().size() == 3 )
	{
		// TODO add checking of the scenes
	}
}

BOOST_AUTO_TEST_SUITE_END()

// TODO add creation of the xml file and saving it,
// then reading it and parsing it to settings.
BOOST_AUTO_TEST_CASE( read_from_file )
{

	std::string filename( "test_conf.xml" );
	vl::Settings settings;
	vl::SettingsSerializer ser(&settings);
	BOOST_CHECK_NO_THROW( ser.readFile(filename) );

	BOOST_CHECK_EQUAL( settings.nRoots(), 2u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(i);
		BOOST_CHECK_EQUAL( root.path.root_directory(), "/" );
		BOOST_CHECK( fs::exists(root.path) );
		BOOST_TEST_MESSAGE( "root " << i << " path = " << root.path );
	}

	fs::path path = settings.getEqConfigPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(), "1-window.eqc" );

	std::vector<vl::Settings::Scene> const &scenes = settings.getScenes();
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		BOOST_CHECK_EQUAL( scenes.at(i).file, "T7.scene" );
		BOOST_CHECK_EQUAL( scenes.at(i).name, "T7" );
		BOOST_TEST_MESSAGE( "scene = " << path );
	}
	
	std::vector<fs::path> resource_paths = settings.getOgreResourcePaths();
	BOOST_CHECK_EQUAL( resource_paths.size(), 2u );
	for( size_t i = 0; i < resource_paths.size(); ++i )
	{
		fs::path const &res_path = resource_paths.at(i);
		BOOST_CHECK_EQUAL( res_path.root_directory(), "/" );
		BOOST_CHECK_EQUAL( res_path.filename(), "resources.cfg" );
		BOOST_TEST_MESSAGE( "resource path = " << res_path );
	}

	path = settings.getOgrePluginsPath();
	BOOST_CHECK_EQUAL( path.root_directory(), "/" );
	BOOST_CHECK_EQUAL( path.filename(),  "plugins.cfg" );
	BOOST_TEST_MESSAGE( "plugin path = "<< path );

	vl::Args const &arg = settings.getEqArgs();
	BOOST_TEST_MESSAGE( "args = "<< arg );
	BOOST_CHECK_EQUAL( arg.size(), 3u );
	settings.setExePath( "test_settings" );

	BOOST_CHECK_EQUAL( arg.size(), 4u );
	BOOST_TEST_MESSAGE( "args = "<< arg );
	if( arg.size() == 4 )
	{
		BOOST_REQUIRE( arg.at(0) );
		BOOST_TEST_MESSAGE( arg.at(0) );
		BOOST_TEST_MESSAGE( arg.at(1) );
		BOOST_CHECK_EQUAL( std::string(arg.at(0)), "test_settings" );
		BOOST_CHECK_EQUAL( std::string(arg.at(1)), "--eq-config" );
		path = arg.at(2);
		BOOST_CHECK_EQUAL( path.filename(), "1-window.eqc" );
	}
}

BOOST_AUTO_TEST_CASE( missing_read_file )
{
	std::string missing_filename( "conf.xml" );
	std::string invalid_filename( "inv_conf.xml" );
	vl::Settings settings;
	vl::SettingsSerializer ser(&settings);
	BOOST_REQUIRE( fs::exists( invalid_filename ) );
	BOOST_REQUIRE( !fs::exists( missing_filename ) );
	BOOST_CHECK_THROW( ser.readFile( missing_filename ), vl::missing_file );
	BOOST_CHECK_THROW( ser.readFile( invalid_filename ), vl::invalid_xml );
}

// TODO implement using Serializer
// TODO Serializer and Deserializer are named incorrectly.
BOOST_AUTO_TEST_CASE( write_to_file )
{

}
