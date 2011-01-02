#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include <iostream>

#include <OgreStringConverter.h>

#include "settings.hpp"
#include "base/filesystem.hpp"
#include "base/exceptions.hpp"

#include "settings_fixtures.hpp"

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

	vl::Settings settings( PROJECT );
	vl::SettingsRefPtr settings_ptr( &settings, vl::null_deleter() );
	vl::SettingsSerializer ser( settings_ptr );

	// Create the test data
	std::string data;
	rapidxml::print(std::back_inserter(data), doc, 0);

	// Parse
	BOOST_CHECK_THROW( ser.readData( data ), vl::invalid_settings );
}

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

BOOST_AUTO_TEST_CASE( root_elem_missing_path )
{
	std::string root_name("data");

	// root node
	rapidxml::xml_node<> *root
		= doc.allocate_node(rapidxml::node_element, "root");
	config->append_node( root );

	// Valid root, missing path element
	rapidxml::xml_attribute<> *name = doc.allocate_attribute("name", "data");
	root->append_attribute(name);

	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK_EQUAL( settings.nRoots(), 1u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(i);
		BOOST_CHECK_EQUAL( root.name, root_name );
		BOOST_CHECK_EQUAL( root.path, "" );
	}
}

BOOST_AUTO_TEST_CASE( root_elem_missing_name )
{
	std::string root_name("data");
	std::string root_path("local/work");
	
	rapidxml::xml_node<> *root
		= doc.allocate_node(rapidxml::node_element, "root");

	// Invalid root, missing name attribute
	config->append_node( root );
	rapidxml::xml_node<> *path = doc.allocate_node(rapidxml::node_element, "path", root_path.c_str() );
	root->append_node(path);

	BOOST_CHECK_THROW( readXML(), vl::invalid_settings );
	BOOST_CHECK( checkEmpty() );
}

BOOST_AUTO_TEST_CASE( root_elem )
{
	std::string root_name("data");
	std::string root_path("local/work");

	// Valid root, name attribute and path element
	config->append_node( createRoot( root_name, root_path ) );

	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK_EQUAL( settings.nRoots(), 1u );
	for( size_t i = 0; i < settings.nRoots(); ++i )
	{
		vl::Settings::Root const &root = settings.getRoot(i);
		BOOST_CHECK_EQUAL( root.name, root_name );
		BOOST_CHECK_EQUAL( root.path, root_path );
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
	BOOST_CHECK_EQUAL( settings.getOgrePluginsPath(),
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
		BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().at(0),
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
	settings = vl::Settings( PROJECT );
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
	settings = vl::Settings( PROJECT );
	
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
	settings = vl::Settings( PROJECT );

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

BOOST_AUTO_TEST_CASE( tracker_off )
{
	std::string tracker_name( "glasses@localhost:3883" );

	rapidxml::xml_node<> *tracking = createTracking( tracker_name );
	config->append_node( tracking );

	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK( !settings.trackerOn() );
}

BOOST_AUTO_TEST_CASE( tracker_on_default )
{
	std::string tracker_name( "glasses@localhost:3883" );

	rapidxml::xml_node<> *tracking = createTracking( tracker_name, true );
	config->append_node( tracking );

	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK( settings.trackerOn() );
	BOOST_CHECK_EQUAL( settings.getTrackerAddress(), tracker_name );
	BOOST_CHECK_EQUAL( settings.getTrackerDefaultPosition(), Ogre::Vector3::ZERO );
	BOOST_CHECK_EQUAL( settings.getTrackerDefaultOrientation(), Ogre::Quaternion::IDENTITY );
}

BOOST_AUTO_TEST_CASE( tracker_on_extra )
{
	std::string tracker_name( "glasses@localhost:3883" );
	const double TOLERANCE = 1e-3;

	Ogre::Vector3 pos( 1, 1.5, 10 );
	Ogre::Vector3 u( 1, 1, 1 );
	u.normalise();
	Ogre::Quaternion orient( Ogre::Radian( Ogre::Degree(33) ), u );

	rapidxml::xml_node<> *tracking = createTracking( tracker_name, true, pos, orient );
	config->append_node( tracking );

	BOOST_MESSAGE( doc );
	BOOST_CHECK_NO_THROW( readXML() );
	
	BOOST_CHECK( settings.trackerOn() );
	BOOST_CHECK_EQUAL( settings.getTrackerAddress(), tracker_name );
	for( size_t i = 0; i < 3; ++i )
	{
		BOOST_CHECK_CLOSE( settings.getTrackerDefaultPosition()[i], pos[i], TOLERANCE );
	}

	for( size_t i = 0; i < 4; ++i )
	{
		BOOST_CHECK_CLOSE( settings.getTrackerDefaultOrientation()[i], orient[i], TOLERANCE );
	}
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
		BOOST_CHECK_EQUAL( root->path, paths.at(i) );
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

	std::string data_path( "local/work" );
	std::string media_path( "local/home" );

	BOOST_CHECK_EQUAL( settings.getOgreResourcePaths().size(), 2u );
	if( settings.getOgreResourcePaths().size() == 2 )
	{
		// TODO add checking of the resources
	}

	BOOST_CHECK_EQUAL( settings.getEqConfigPath(),
					   data_path + "/" + std::string("1-window.eqc") );

	BOOST_CHECK_EQUAL( settings.getOgrePluginsPath(),
					   media_path + "/" + std::string("plugins.cfg") );

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

	std::string path = settings.getEqConfigPath();
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
	
	std::vector<std::string> resource_paths = settings.getOgreResourcePaths();
	BOOST_CHECK_EQUAL( resource_paths.size(), 2u );
	for( size_t i = 0; i < resource_paths.size(); ++i )
	{
		std::string const &res_path = resource_paths.at(i);
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
	}
	
	fs::remove( fs::path(filename) );
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_CASE( missing_read_file )
{
	std::string missing_filename( "conf.xml" );
	vl::Settings settings( PROJECT );
	vl::SettingsRefPtr settings_ptr( &settings, vl::null_deleter() );
	vl::SettingsSerializer ser( settings_ptr );
	
	BOOST_REQUIRE( !fs::exists( missing_filename ) );
	BOOST_CHECK_THROW( ser.readFile( missing_filename ), vl::missing_file );
}

