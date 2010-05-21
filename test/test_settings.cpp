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

// TODO test creation of settings
// TODO test copying of settings
BOOST_AUTO_TEST_CASE( constructors )
{

}

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
	vl::SettingsRefPtr settings_ptr( &settings, vl::null_deleter() );
	vl::SettingsSerializer ser( settings_ptr );

	// Create the test data
	std::string data;
	rapidxml::print(std::back_inserter(data), doc, 0);

	// Parse
	BOOST_CHECK_THROW( ser.readData( data ), vl::invalid_settings );
}

struct SettingsFixture
{
	SettingsFixture( void )
		: settings(), ser(0), doc(), config(0)
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

	rapidxml::xml_node<> *createTracking( std::string const &name,
										  std::string const &file,
										  std::string const &root_name = std::string() )
	{
		char *c_name = doc.allocate_string( name.c_str() );
		char *c_file = doc.allocate_string( file.c_str() );
		char *c_root_name = doc.allocate_string( root_name.c_str() );
		
		rapidxml::xml_node<> *tracking
			= doc.allocate_node(rapidxml::node_element, "tracking" );

		rapidxml::xml_node<> *node
			= doc.allocate_node(rapidxml::node_element, "file", c_file );
		tracking->append_node( node );
		
		rapidxml::xml_attribute<> *attr
			= doc.allocate_attribute( "root", c_root_name );
		tracking->append_attribute( attr );
		
		attr = doc.allocate_attribute( "name", c_name );
		
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
	std::string root_name("data");
	std::string root_path("local/work");

	// root node
	rapidxml::xml_node<> *root
		= doc.allocate_node(rapidxml::node_element, "root");
	config->append_node( root );

	// Invalid root, missing path element
	rapidxml::xml_attribute<> *name = doc.allocate_attribute("name", "data");
	root->append_attribute(name);

	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );
	
	// Invalid root, missing name attribute
	root->remove_attribute( name );
	rapidxml::xml_node<> *path = doc.allocate_node(rapidxml::node_element, "path", "local/work" );
	root->append_node(path);

	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );

	// Valid root, name attribute and path element
	root->append_attribute( name );
	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK_EQUAL( settings.nRoots(), 1u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(0);
		BOOST_CHECK_EQUAL( root.name, root_name );
		BOOST_CHECK_EQUAL( root.path.file_string(), root_path );
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
	
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
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
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );
	
	// Test invalid resource xml, with empty file element
	rapidxml::xml_node<> *invalid_file
		= doc.allocate_node(rapidxml::node_element, "file", "" );
	resources->append_node(invalid_file);

	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );
	
	resources->remove_node( invalid_file );

	// Test invalid resource xml, with reference to root that does not exist
	rapidxml::xml_attribute<> *attr = doc.allocate_attribute("root", "data");
	resources->append_attribute( attr );
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
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
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
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
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
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

BOOST_AUTO_TEST_CASE( tracking_elem )
{
	std::string tracking_file( "tracking.xml" );

	rapidxml::xml_node<> *tracking = createTracking( "tracking", tracking_file );
	config->append_node( tracking );

	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK_EQUAL( settings.getTracking().size(), 1u );
	if( settings.getTracking().size() > 0 )
	{
		BOOST_CHECK_EQUAL( settings.getTracking().at(0).getPath(), tracking_file );
	}
}

BOOST_AUTO_TEST_CASE( multiple_trackings )
{
	std::string tracking_file1( "tracking.xml" );
	std::string tracking_file2( "tracking2.xml" );
	
	rapidxml::xml_node<> *tracking
		= createTracking( " track1", tracking_file1 );
	rapidxml::xml_node<> *tracking2
		= createTracking( " track2", tracking_file2 );

	config->append_node( tracking );
	config->append_node( tracking2 );
	
	BOOST_CHECK_NO_THROW( readXML() );

	BOOST_CHECK_EQUAL( settings.getTracking().size(), 2u );
	if( settings.getTracking().size() > 1 )
	{
		BOOST_CHECK_EQUAL( settings.getTracking().at(0).getPath(), tracking_file1 );
		BOOST_CHECK_EQUAL( settings.getTracking().at(1).getPath(), tracking_file2 );
	}
}

BOOST_AUTO_TEST_CASE( tracking_with_root )
{
	std::string track_file1( "tracking.xml" );
	std::string track_file2( "tracking2.xml" );
	std::string track_name1( "track1" );
	std::string track_name2( "track2" );
	std::string root_name( "data" );

	rapidxml::xml_node<> *root = createRoot( root_name, "local/work" );
	config->append_node(root);

	rapidxml::xml_node<> *tracking
		= createTracking( track_name2, track_file1, root_name );
	rapidxml::xml_node<> *tracking2
		= createTracking( track_name2, track_file2, root_name );

	config->append_node( tracking );
	config->append_node( tracking2 );
}

BOOST_AUTO_TEST_CASE( multiple_roots_invalid )
{
	std::string root_name("data");
	
	// First root node
	rapidxml::xml_node<> *root = createRoot( root_name, "local/work" );
	config->append_node( root );

	// Second root node, invalid same name as the first
	root = createRoot( root_name, "local/usr" );
	config->append_node( root );

	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );
}

BOOST_AUTO_TEST_CASE( multiple_roots )
{
	const size_t N_ROOTS = 4;
	std::vector<std::string> names(N_ROOTS);
	names.at(0) = "data";
	names.at(1) = "media";
	names.at(2) = "media2";
	names.at(3) = "something";
	
	std::vector<std::string> paths(N_ROOTS);
	paths.at(0) = "local/work";
	paths.at(1) = "local/usr";
	paths.at(2) = "local/home";
	paths.at(3) = "/something/somewhere/null";

	for( size_t i = 0; i < N_ROOTS; ++i )
	{
		rapidxml::xml_node<> *xml_root = createRoot( names.at(i), paths.at(i) );
		config->append_node( xml_root );
	}

	BOOST_CHECK_NO_THROW( readXML() );

	BOOST_CHECK_EQUAL( settings.nRoots(), N_ROOTS );

	// Find all the root elements and check them
	for( size_t i = 0; i < N_ROOTS; ++i )
	{
		vl::Settings::Root const *root;
		root = settings.findRoot( names.at(i) );
		BOOST_REQUIRE_NE( root, (vl::Settings::Root *)0 );
		BOOST_CHECK_EQUAL( root->path.file_string(), paths.at(i) );
	}
}

BOOST_AUTO_TEST_CASE( multiple_resources )
{
	rapidxml::xml_node<> *xml_resources1
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( xml_resources1 );

	rapidxml::xml_node<> *xml_resources2
		= doc.allocate_node(rapidxml::node_element, "resources" );
	config->append_node( xml_resources2 );

	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "resources.cfg" );
	xml_resources1->append_node(file);
	
	// Test invalid resource xml, one of the resources without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );

	// Valid resource xml, both resources have file element
	file = doc.allocate_node(rapidxml::node_element, "file", "resources2.cfg" );
	xml_resources2->append_node(file);
	
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

BOOST_AUTO_TEST_CASE( multiple_scenes_invalid )
{
	// First two scenes are valid, contains file element
	// First valid scene element
	rapidxml::xml_node<> *scene
		= doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene );
	rapidxml::xml_node<> *file
		= doc.allocate_node(rapidxml::node_element, "file", "T7.scene" );
	scene->append_node( file );

	// Second valid scene element
	scene = doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene );
	file = doc.allocate_node(rapidxml::node_element, "file", "ruukki.scene" );
	scene->append_node( file );

	// Invalid scene element, without file element
	rapidxml::xml_node<> *scene3
		= doc.allocate_node(rapidxml::node_element, "scene" );
	config->append_node( scene3 );

	// Third scene is invalid, without file element
	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );
}

BOOST_AUTO_TEST_CASE( multiple_scenes_valid )
{
	const size_t N_SCENES = 4;
	std::vector<std::string> scene_names(N_SCENES);
	scene_names.at(0) = "T7";
	scene_names.at(1) = "ruukki";
	scene_names.at(3) = "test";
	
	std::vector<std::string> scene_files(N_SCENES);
	scene_files.at(0) = "T7.scene";
	scene_files.at(1) = "ruukki.scene";
	scene_files.at(2) = "test.scene";
	scene_files.at(3) = "test.scene";
	
	std::vector<std::string> scene_attachs(N_SCENES);
	scene_attachs.at(0) = "cave";
	scene_attachs.at(1) = "env";
	
	std::vector<std::string> scene_types(N_SCENES);
	scene_types.at(2) = "release";
	scene_types.at(3) = "release";

	for( size_t i = 0; i < scene_names.size(); ++i )
	{
		std::string const &name = scene_names.at(i);
		std::string const &file = scene_files.at(i);
		std::string const &type = scene_types.at(i);
		std::string const &attach = scene_attachs.at(i);

		rapidxml::xml_node<> *xml_scene
			= createScene( name, file, type, attach );

		config->append_node( xml_scene );
	}
	
	BOOST_CHECK_NO_THROW( readXML() );
	BOOST_CHECK_EQUAL( settings.getScenes().size(), N_SCENES );

	for( size_t i = 0; i < settings.getScenes().size(); ++i )
	{
		BOOST_REQUIRE( i < N_SCENES );
		
		vl::Settings::Scene const &scn = settings.getScenes().at(i);
		BOOST_CHECK_EQUAL( scn.file, scene_files.at(i) );
		BOOST_CHECK_EQUAL( scn.name, scene_names.at(i) );
		BOOST_CHECK_EQUAL( scn.type, scene_types.at(i) );
		BOOST_CHECK_EQUAL( scn.attach_node, scene_attachs.at(i) );
	}
}

BOOST_AUTO_TEST_SUITE_END()

struct CompleteXMLFixture : public SettingsFixture
{
	CompleteXMLFixture(void )
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
			= doc.allocate_node(rapidxml::node_element, "file", "resources.cfg" );
		resource->append_node(file);
		attrib = doc.allocate_attribute("root", "data");
		resource->append_attribute( attrib );

		// Second resources node
		resource
			= doc.allocate_node(rapidxml::node_element, "resources" );
		config->append_node( resource );
		file = doc.allocate_node(rapidxml::node_element, "file", "resources.cfg" );
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
	}

	~CompleteXMLFixture(void )
	{
	}

};

BOOST_FIXTURE_TEST_SUITE( IntegrationTests, CompleteXMLFixture )

// Real world use case. We have two roots, two resource files, three scenes,
// one plugins file, one eqc. Everything except scenes need root reference.
BOOST_AUTO_TEST_CASE( root_references )
{
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

// TODO add creation of the xml file and saving it,
// then reading it and parsing it to settings.
BOOST_AUTO_TEST_CASE( read_from_file )
{
	// Write to out file
	std::string filename( "test_conf.xml" );
	data.clear();
	rapidxml::print(std::back_inserter(data), doc, 0);
	std::ofstream os( filename.c_str() );
	os << data;
	os.close();
	
	BOOST_CHECK_NO_THROW( ser->readFile(filename) );

	BOOST_CHECK_EQUAL( settings.nRoots(), 2u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(i);
		if( i == 0 )
		{
			BOOST_CHECK_EQUAL( root.getPath(), "local/work" );
		}
		else if( i == 1 )
		{
			BOOST_CHECK_EQUAL( root.getPath(), "local/home" );
		}
	}

	fs::path path = settings.getEqConfigPath();
	BOOST_CHECK_EQUAL( path, "local/work/1-window.eqc" );

	std::vector<vl::Settings::Scene> const &scenes = settings.getScenes();
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		if( i == 0 )
		{
			BOOST_CHECK_EQUAL( scenes.at(i).file, "T7.scene" );
			BOOST_CHECK_EQUAL( scenes.at(i).name, "T7" );
		}
		else if( i == 1 )
		{
			BOOST_CHECK_EQUAL( scenes.at(i).file, "ruukki.scene" );
			BOOST_CHECK_EQUAL( scenes.at(i).name, "ruukki" );
		}
		else if( i == 2 )
		{
			BOOST_CHECK_EQUAL( scenes.at(i).file, "ruukki.scene" );
			BOOST_CHECK( scenes.at(i).name.empty() );
		}
	}
	
	std::vector<fs::path> resource_paths = settings.getOgreResourcePaths();
	BOOST_CHECK_EQUAL( resource_paths.size(), 2u );
	for( size_t i = 0; i < resource_paths.size(); ++i )
	{
		fs::path const &res_path = resource_paths.at(i);
		if( i == 0 )
		{
			BOOST_CHECK_EQUAL( res_path, "local/work/resources.cfg" );
		}
		if( i == 1 )
		{
			BOOST_CHECK_EQUAL( res_path, "local/home/resources.cfg" );
		}
	}

	path = settings.getOgrePluginsPath();
	BOOST_CHECK_EQUAL( path,  "local/home/plugins.cfg" );

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
	
	fs::remove( filename );
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_CASE( missing_read_file )
{
	std::string missing_filename( "conf.xml" );
	vl::Settings settings;
	vl::SettingsRefPtr settings_ptr( &settings, vl::null_deleter() );
	vl::SettingsSerializer ser( settings_ptr );
	
	BOOST_REQUIRE( !fs::exists( missing_filename ) );
	BOOST_CHECK_THROW( ser.readFile( missing_filename ), vl::missing_file );
}