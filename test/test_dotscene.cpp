/*	Joonatan Kuosa
 *	2010-05
 *	Test module for dotscene loader
 *	Creates xml file based on the test cases parses it with rapidxml and
 *	DotsceneLoader, verifies that correct objects are created based on the
 *	dotscene xml definition.
 *
 *	Uses vl::cl::SceneManager, vl::cl::Entity, vl::cl::Light, vl::cl::SceneNode,
 *	vl::cl::Camera
 *	Might want to replace the vl::cl object by mocks.
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE dotscene

#include <boost/test/unit_test.hpp>

#include "dotscene_loader.hpp"
#include "base/exceptions.hpp"

// vl::cl objects
//#include "eq_graph/eq_scene_manager.hpp"
//#include "eq_graph/eq_scene_node.hpp"
//#include "eq_graph/eq_entity.hpp"
//#include "eq_graph/eq_light.hpp"
//#include "eq_graph/eq_camera.hpp"

#include "../eq_graph/mocks.hpp"

#include "base/rapidxml_print.hpp"

struct DotsceneFixture
{
	DotsceneFixture( void )
		: sm( new mock::SceneManager )
	{
		/*
		// Set factories
		sm->setSceneNodeFactory( vl::graph::SceneNodeFactoryPtr(
					new mock::SceneNodeFactory ) );
		sm->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new mock::CameraFactory ) );
		sm->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new mock::EntityFactory ) );
		*/
		scene = doc.allocate_node(rapidxml::node_element, "scene" );
		doc.append_node( scene );
		rapidxml::xml_attribute<> *attr
			= doc.allocate_attribute("formatVersion", "1.0.0" );
		scene->append_attribute( attr );
	}

	~DotsceneFixture( void )
	{
	}

	void loadXML( void )
	{
		xml_data.clear();
		rapidxml::print(std::back_inserter(xml_data), doc, 0);
		// Print to string so we can use SettingsSerializer for the data
		BOOST_TEST_MESSAGE( xml_data );
		loader.parseDotScene( xml_data, sm );
	}
	
	// Generated data for the xml parser
	std::string xml_data;
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> *scene;
	DotSceneLoader loader;
	
	mock::SceneManagerPtr sm;
};

struct ResourceFixture : public DotsceneFixture
{
	ResourceFixture( void )
	{
		resourceLocations = doc.allocate_node(rapidxml::node_element, "resourceLocations" );
		scene->append_node( resourceLocations );
	}

	~ResourceFixture( void )
	{
	}

	rapidxml::xml_node<> *resourceLocations;
};

struct EnvironmentFixture : public DotsceneFixture
{
	EnvironmentFixture( void )
	{
		environment = doc.allocate_node(rapidxml::node_element, "environment" );
		scene->append_node( environment );
	}

	~EnvironmentFixture( void )
	{
	}

	rapidxml::xml_node<> *environment;
};

BOOST_FIXTURE_TEST_SUITE( scene, ResourceFixture )

BOOST_AUTO_TEST_CASE( valid_scene )
{
	mock::SceneNodePtr root( new mock::SceneNode );

	// Invalid scene xml, without formatVersion
	rapidxml::xml_attribute<> *attr
		= scene->first_attribute( "formatVersion" );
	scene->remove_attribute( attr );

	BOOST_CHECK_THROW( loadXML(), vl::invalid_xml );

	// Valid scene xml
	MOCK_EXPECT( sm, getRootNode ).once().returns( root );
	scene->append_attribute( attr );
	BOOST_CHECK_NO_THROW( loadXML() );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( resources, ResourceFixture )

// Resource location test cases
BOOST_AUTO_TEST_CASE( test_resources_locations )
{

}

BOOST_AUTO_TEST_SUITE_END()

// Enviroment test cases
BOOST_FIXTURE_TEST_SUITE( environment, EnvironmentFixture )

// TODO Not supported at the moment
BOOST_AUTO_TEST_CASE( test_viewport )
{
	
}

BOOST_AUTO_TEST_CASE( test_camera )
{
	mock::SceneNodePtr root( new mock::SceneNode );
	mock::SceneNodePtr camNode( new mock::SceneNode );
	mock::CameraPtr cam( new mock::Camera );
	// Loader should retrieve root object
	// where nodes and objects are to be attached
	MOCK_EXPECT( sm, getRootNode ).once().returns( root );

	// Add camera definition to the xml file
	rapidxml::xml_node<> *camera
		= doc.allocate_node(rapidxml::node_element, "camera");
	environment->append_node( camera );
	rapidxml::xml_attribute<> *attrb = doc.allocate_attribute("name", "camera");
	camera->append_attribute(attrb);

	// Add clipping
	rapidxml::xml_node<> *node
		= doc.allocate_node(rapidxml::node_element, "clipping");
	camera->append_node( node );
	attrb = doc.allocate_attribute("near", "0.1");
	node->append_attribute(attrb);
	attrb = doc.allocate_attribute("far", "100");
	node->append_attribute(attrb);

	// Add position
	node = doc.allocate_node(rapidxml::node_element, "position");
	camera->append_node( node );
	attrb = doc.allocate_attribute("x", "1");
	node->append_attribute(attrb);
	attrb = doc.allocate_attribute("y", "2");
	node->append_attribute(attrb);
	attrb = doc.allocate_attribute("z", "-1");
	node->append_attribute(attrb);
	
	// Add rotation
	node = doc.allocate_node(rapidxml::node_element, "rotation");
	camera->append_node( node );
	attrb = doc.allocate_attribute("qw", "0");
	node->append_attribute(attrb);
	attrb = doc.allocate_attribute("qx", "1");
	node->append_attribute(attrb);
	attrb = doc.allocate_attribute("qy", "0");
	node->append_attribute(attrb);
	attrb = doc.allocate_attribute("qz", "0");
	node->append_attribute(attrb);
		
	// Loader should create camera, create cameraNode as a child of roots
	// and it should attach the camera to cameraNode.
	MOCK_EXPECT( sm, createCamera ).once().with("camera").returns( cam );
	MOCK_EXPECT( camNode, attachObject ).once().with( cam );
	MOCK_EXPECT( root, createChild ).once().with( "cameraNode" )
		.returns(camNode);

	// Test clipping
	MOCK_EXPECT( cam, setFarClipDistance ).once().with( 100 );
	MOCK_EXPECT( cam, setNearClipDistance ).once().with( 0.1 );
	
	// Test position
	MOCK_EXPECT( camNode, setPosition ).once()
		.with( vl::vector(1, 2, -1), vl::graph::SceneNode::TS_PARENT );
	
	// Test orientation
	MOCK_EXPECT( camNode, setOrientation ).once()
		.with( vl::quaternion(0, 1, 0, 0), vl::graph::SceneNode::TS_LOCAL );

	BOOST_CHECK_NO_THROW( loadXML() );
}

// TODO Not supported at the moment
BOOST_AUTO_TEST_CASE( test_skybox )
{

}

// TODO Not supported at the moment
BOOST_AUTO_TEST_CASE( test_skydome )
{

}

// TODO Not supported at the moment
BOOST_AUTO_TEST_CASE( test_skyplane )
{

}

// Test environment element containing one fog element
BOOST_AUTO_TEST_CASE( test_fog )
{
	mock::SceneNodePtr root( new mock::SceneNode );
	// Loader should retrieve root object
	// where nodes and objects are to be attached
	MOCK_EXPECT( sm, getRootNode ).exactly(2).returns( root );

	// Add fog element
	rapidxml::xml_node<> *fog
		= doc.allocate_node(rapidxml::node_element, "fog");
	environment->append_node( fog );
	rapidxml::xml_attribute<> *attrb = doc.allocate_attribute("mode", "0");
	fog->append_attribute( attrb );
	attrb = doc.allocate_attribute("start", "0");
	fog->append_attribute( attrb );
	attrb = doc.allocate_attribute("end", "1000");
	fog->append_attribute( attrb );
	attrb = doc.allocate_attribute("density", "0");
	fog->append_attribute( attrb );
	
	// Add colour element
	rapidxml::xml_node<> *colour
		= doc.allocate_node(rapidxml::node_element, "colour");
	fog->append_node( colour );
	attrb = doc.allocate_attribute("r", "0.2");
	colour->append_attribute(attrb);
	attrb = doc.allocate_attribute("b", "0.4");
	colour->append_attribute(attrb);
	attrb = doc.allocate_attribute("g", "0.3");
	colour->append_attribute(attrb);

	MOCK_EXPECT( sm, setFog ).once()
		.with( 0, vl::colour( 0.2, 0.3, 0.4, 1.0 ), 0, 0, 1000 );

	BOOST_CHECK_NO_THROW( loadXML() );

	MOCK_VERIFY( sm, setFog );
	
	attrb = fog->first_attribute("mode");
	attrb->value("exp2");
	attrb = fog->first_attribute("density");
	attrb->value("0.2");
	
	MOCK_EXPECT( sm, setFog ).once()
		.with( vl::FOG_EXP2, vl::colour( 0.2, 0.3, 0.4, 1.0 ), 0.2, 0, 1000 );

	BOOST_CHECK_NO_THROW( loadXML() );
}

// Test environment element containing colourAmbient element
BOOST_AUTO_TEST_CASE( test_ambient )
{
	mock::SceneNodePtr root( new mock::SceneNode );
	// Loader should retrieve root object
	// where nodes and objects are to be attached
	MOCK_EXPECT( sm, getRootNode ).once().returns( root );

	// Add ambient definition
	rapidxml::xml_node<> *ambient
		= doc.allocate_node(rapidxml::node_element, "colourAmbient");
	environment->append_node( ambient );
	rapidxml::xml_attribute<> *attrb = doc.allocate_attribute("r", "0.5");
	ambient->append_attribute(attrb);
	attrb = doc.allocate_attribute("b", "0.5");
	ambient->append_attribute(attrb);
	attrb = doc.allocate_attribute("g", "0.5");
	ambient->append_attribute(attrb);
	
	MOCK_EXPECT( sm, setAmbientLight ).once()
		.with( vl::colour( 0.5, 0.5, 0.5, 1.0 ) );

	BOOST_CHECK_NO_THROW( loadXML() );
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_CASE( test_nodes, DotsceneFixture )
{
	mock::SceneNodePtr root( new mock::SceneNode );

	MOCK_EXPECT( sm, getRootNode ).once().returns( root );

	rapidxml::xml_node<> *nodes
		= doc.allocate_node(rapidxml::node_element, "nodes" );
	scene->append_node( nodes );

	BOOST_CHECK_NO_THROW( loadXML() );
}

struct SceneNodeFixture : public DotsceneFixture
{
	SceneNodeFixture( void )
		: 	root( new mock::SceneNode ), rock( new mock::SceneNode )
	{
		nodes = doc.allocate_node(rapidxml::node_element, "nodes" );
		scene->append_node( nodes );

		// Node element
		node = doc.allocate_node(rapidxml::node_element, "node");
		nodes->append_node( node );
		rapidxml::xml_attribute<> *attrb = doc.allocate_attribute("name", "rock");
		node->append_attribute(attrb);

		// Position element
		pos = vl::vector(-1, 1, 0);
		position = doc.allocate_node(rapidxml::node_element, "position");
		node->append_node(position);
		attrb = doc.allocate_attribute("x", "-1");
		position->append_attribute(attrb);
		attrb = doc.allocate_attribute("y", "1");
		position->append_attribute(attrb);
		attrb = doc.allocate_attribute("z", "0");
		position->append_attribute(attrb);

		// Rotation element
		quat = vl::quaternion(0, 1, 0, 0);
		rotation = doc.allocate_node(rapidxml::node_element, "rotation");
		node->append_node(rotation);
		attrb = doc.allocate_attribute("x", "1");
		rotation->append_attribute(attrb);
		attrb = doc.allocate_attribute("y", "0");
		rotation->append_attribute(attrb);
		attrb = doc.allocate_attribute("z", "0");
		rotation->append_attribute(attrb);
		attrb = doc.allocate_attribute("w", "0");
		rotation->append_attribute(attrb);

		// Scale element
		scl = vl::vector(0.2, 0.1, 0.3);
		scale = doc.allocate_node(rapidxml::node_element, "scale");
		node->append_node(scale);
		attrb = doc.allocate_attribute("x", ".2");
		scale->append_attribute(attrb);
		attrb = doc.allocate_attribute("y", ".1");
		scale->append_attribute(attrb);
		attrb = doc.allocate_attribute("z", ".3");
		scale->append_attribute(attrb);

		// Loader should retrieve root object
		// where nodes and objects are to be attached
		MOCK_EXPECT( sm, getRootNode ).once().returns( root );

		MOCK_EXPECT( root, createChild ).once().with( "rock" ).returns( rock );

		MOCK_EXPECT( rock, setPosition ).once()
			.with( pos, vl::graph::SceneNode::TS_PARENT );
		MOCK_EXPECT( rock, setOrientation ).once()
			.with( quat, vl::graph::SceneNode::TS_LOCAL );
		MOCK_EXPECT( rock, setScale ).once()
			.with( scl );
	}

	~SceneNodeFixture( void )
	{
	}

	rapidxml::xml_node<> *nodes;
	rapidxml::xml_node<> *node;
	rapidxml::xml_node<> *position;
	rapidxml::xml_node<> *rotation;
	rapidxml::xml_node<> *scale;
	vl::vector pos;
	vl::vector scl;
	vl::quaternion quat;
	mock::SceneNodePtr root;
	mock::SceneNodePtr rock;
	
};

// Nodes test cases
BOOST_FIXTURE_TEST_SUITE( nodes, SceneNodeFixture )

BOOST_AUTO_TEST_CASE( test_scene_node )
{
	BOOST_CHECK_NO_THROW( loadXML() );
}

// TODO test material file
BOOST_AUTO_TEST_CASE( test_entity )
{
	mock::EntityPtr rock_ent( new mock::Entity );
	mock::EntityPtr rock_ent2( new mock::Entity );
	
	// First entity
	rapidxml::xml_node<> *entity
		= doc.allocate_node(rapidxml::node_element, "entity");
	node->append_node( entity );
	rapidxml::xml_attribute<> *attrb = doc.allocate_attribute("name", "rock");
	entity->append_attribute(attrb);
	attrb = doc.allocate_attribute("meshFile", "rock.mesh");
	entity->append_attribute(attrb);
	attrb = doc.allocate_attribute("castShadows", "true");
	entity->append_attribute(attrb);

	MOCK_EXPECT( sm, createEntity ).once().with( "rock", "rock.mesh" )
		.returns( rock_ent );
	MOCK_EXPECT( rock, attachObject ).once().with( rock_ent );
	MOCK_EXPECT( rock_ent, setCastShadows ).with(true);

	// Second entity
	entity = doc.allocate_node(rapidxml::node_element, "entity");
	node->append_node( entity );
	attrb = doc.allocate_attribute("name", "rock2");
	entity->append_attribute(attrb);
	attrb = doc.allocate_attribute("meshFile", "rock.mesh");
	entity->append_attribute(attrb);
	attrb = doc.allocate_attribute("castShadows", "false");
	entity->append_attribute(attrb);
	
	MOCK_EXPECT( sm, createEntity ).once().with( "rock2", "rock.mesh" )
		.returns( rock_ent2 );
	MOCK_EXPECT( rock, attachObject ).once().with( rock_ent2 );
	MOCK_EXPECT( rock_ent2, setCastShadows ).with(false);

	BOOST_CHECK_NO_THROW( loadXML() );
}


// Test three nodes which are childs of root node
// Also two nodes which are childs of the first rock node.
BOOST_AUTO_TEST_CASE( test_node_hierarchy )
{
	mock::SceneNodePtr rock2( new mock::SceneNode );
	mock::SceneNodePtr rock3( new mock::SceneNode );
	mock::SceneNodePtr child1( new mock::SceneNode );
	mock::SceneNodePtr child2( new mock::SceneNode );
	
	// Second rock element
	rapidxml::xml_node<> *node2
		= doc.allocate_node(rapidxml::node_element, "node");
	nodes->append_node( node2 );
	rapidxml::xml_attribute<> *attrb = doc.allocate_attribute("name", "rock2");
	node2->append_attribute(attrb);

	MOCK_EXPECT( root, createChild ).once().with( "rock2" ).returns( rock2 );

	// Third rock element
	node2 = doc.allocate_node(rapidxml::node_element, "node");
	nodes->append_node( node2 );
	attrb = doc.allocate_attribute("name", "rock3");
	node2->append_attribute(attrb);
	MOCK_EXPECT( root, createChild ).once().with( "rock3" ).returns( rock3 );

	// First child element
	node2 = doc.allocate_node(rapidxml::node_element, "node");
	node->append_node( node2 );
	attrb = doc.allocate_attribute("name", "child1");
	node2->append_attribute(attrb);
	MOCK_EXPECT( rock, createChild ).once().with( "child1" ).returns( child1 );
	
	// Second child element
	node2 = doc.allocate_node(rapidxml::node_element, "node");
	node->append_node( node2 );
	attrb = doc.allocate_attribute("name", "child2");
	node2->append_attribute(attrb);
	MOCK_EXPECT( rock, createChild ).once().with( "child2" ).returns( child2 );
	
	BOOST_CHECK_NO_THROW( loadXML() );
}

// TODO add two cameras to the rock node and process them
BOOST_AUTO_TEST_CASE( test_camera )
{
	BOOST_CHECK_NO_THROW( loadXML() );
}

BOOST_AUTO_TEST_SUITE_END()

struct LightFixture : public SceneNodeFixture
{
	LightFixture( void )
	{

	}

	~LightFixture( void )
	{
	}

	void createLight( std::string const &name, vl::graph::Light::TYPE type )
	{
		mock::LightPtr light( new mock::Light );
		_lights.push_back( light );
		
		// Create light
		rapidxml::xml_node<> *xml_light
			= doc.allocate_node(rapidxml::node_element, "light");
		node->append_node( xml_light );
		rapidxml::xml_attribute<> *attrb
			= doc.allocate_attribute("name", name.c_str() );
		xml_light->append_attribute(attrb);

		if( type == vl::graph::Light::LT_DIRECTIONAL )
		{ attrb = doc.allocate_attribute("type", "directional" ); }
		else if( type == vl::graph::Light::LT_SPOTLIGHT )
		{ attrb = doc.allocate_attribute("type", "spot" ); }
		else	// ( type == vl::graph::Light::LT_POINT )
		{ attrb = doc.allocate_attribute("type", "point" ); }
		xml_light->append_attribute(attrb);
		
		attrb = doc.allocate_attribute("castShadows", "true");
		xml_light->append_attribute(attrb);

		MOCK_EXPECT( sm, createLight ).once().with( name ).returns( light );
		MOCK_EXPECT(light, setType).once().with( type );
		MOCK_EXPECT(light, setCastShadows).once().with( true );
		MOCK_EXPECT(light, setVisible).once().with( true );
		
		// Create position
		rapidxml::xml_node<> *xml_pos
			= doc.allocate_node(rapidxml::node_element, "position");
		xml_light->append_node( xml_pos );
		attrb = doc.allocate_attribute("x", "-1");
		xml_pos->append_attribute(attrb);
		attrb = doc.allocate_attribute("y", "0");
		xml_pos->append_attribute(attrb);
		attrb = doc.allocate_attribute("z", "1");
		xml_pos->append_attribute(attrb);

		std::string nodeName = name+"Node";
		mock::SceneNodePtr light_node( new mock::SceneNode );
		_light_nodes.push_back(light_node);
		
		MOCK_EXPECT( rock, createChild ).once().with( nodeName )
			.returns( light_node);
		MOCK_EXPECT( light_node, attachObject ).once().with( light );
		MOCK_EXPECT(light_node, setPosition).once()
			.with( vl::vector(-1, 0, 1), vl::graph::SceneNode::TS_PARENT );
			
		// Create direction
		rapidxml::xml_node<> *xml_rot
			= doc.allocate_node(rapidxml::node_element, "directionVector");
		xml_light->append_node( xml_rot );
		attrb = doc.allocate_attribute("x", "0");
		xml_rot->append_attribute(attrb);
		attrb = doc.allocate_attribute("y", "0");
		xml_rot->append_attribute(attrb);
		attrb = doc.allocate_attribute("z", "-1");
		xml_rot->append_attribute(attrb);

		MOCK_EXPECT(light_node, setDirection).once()
			.with( vl::vector(0, 0, -1) );
		
		// Attenuation
		rapidxml::xml_node<> *xml_att
			= doc.allocate_node(rapidxml::node_element, "lightAttenuation");
		xml_light->append_node(xml_att);
		attrb = doc.allocate_attribute("range", "100");
		xml_att->append_attribute(attrb);
		attrb = doc.allocate_attribute("constant", "1");
		xml_att->append_attribute(attrb);
		attrb = doc.allocate_attribute("linear", "0.5");
		xml_att->append_attribute(attrb);
		attrb = doc.allocate_attribute("quadratic", "1");
		xml_att->append_attribute(attrb);

		if( type != vl::graph::Light::LT_DIRECTIONAL )
		{
			MOCK_EXPECT(light, setAttenuation).once().with(100, 1, 0.5, 1);
		}
		
		// Range
		rapidxml::xml_node<> *xml_range
			= doc.allocate_node(rapidxml::node_element, "lightRange");
		xml_light->append_node( xml_range );
		attrb = doc.allocate_attribute("inner", "15");
		xml_range->append_attribute(attrb);
		attrb = doc.allocate_attribute("outer", "30");
		xml_range->append_attribute(attrb);
		attrb = doc.allocate_attribute("falloff", "1");
		xml_range->append_attribute(attrb);

		if( type != vl::graph::Light::LT_DIRECTIONAL )
		{
			MOCK_EXPECT(light, setSpotlightRange).once().with(15, 30, 1);
		}
		// Colour Diffuse
		rapidxml::xml_node<> *xml_diffuse
			= doc.allocate_node(rapidxml::node_element, "colourDiffuse");
		xml_light->append_node( xml_diffuse );
		attrb = doc.allocate_attribute("r", "0.7");
		xml_diffuse->append_attribute(attrb);
		attrb = doc.allocate_attribute("g", "0.7");
		xml_diffuse->append_attribute(attrb);
		attrb = doc.allocate_attribute("b", "0.7");
		xml_diffuse->append_attribute(attrb);

		MOCK_EXPECT(light, setDiffuseColour).once()
			.with( vl::colour(0.7, 0.7, 0.7, 1.0) );
		
		// Colour Specular
		rapidxml::xml_node<> *xml_specular
			= doc.allocate_node(rapidxml::node_element, "colourSpecular");
		xml_light->append_node( xml_specular );
		attrb = doc.allocate_attribute("r", "1");
		xml_specular->append_attribute(attrb);
		attrb = doc.allocate_attribute("g", "1");
		xml_specular->append_attribute(attrb);
		attrb = doc.allocate_attribute("b", "1");
		xml_specular->append_attribute(attrb);

		MOCK_EXPECT(light, setSpecularColour).once()
			.with( vl::colour(1., 1., 1., 1.0) );
	}

	std::vector<mock::LightPtr> _lights;
	std::vector<mock::SceneNodePtr> _light_nodes;
};

BOOST_FIXTURE_TEST_SUITE( lightTests, LightFixture )

BOOST_AUTO_TEST_CASE( test_dir_light )
{
	createLight( "dir_light", vl::graph::Light::LT_DIRECTIONAL );

	BOOST_CHECK_NO_THROW( loadXML() );
}


BOOST_AUTO_TEST_CASE( test_point_light )
{
	createLight( "point_light", vl::graph::Light::LT_POINT );
	
	BOOST_CHECK_NO_THROW( loadXML() );
}


BOOST_AUTO_TEST_CASE( test_spot_light )
{
	createLight( "spot_light", vl::graph::Light::LT_SPOTLIGHT );
	
	BOOST_CHECK_NO_THROW( loadXML() );
}

// Create multiple lights to rock node
// Create one light to scene level i.e. to root node
BOOST_AUTO_TEST_CASE( multiple_lights )
{
	mock::LightPtr light( new mock::Light );
	mock::SceneNodePtr light_node( new mock::SceneNode );

	// Create light
	rapidxml::xml_node<> *xml_light
		= doc.allocate_node(rapidxml::node_element, "light");
	scene->append_node( xml_light );
	rapidxml::xml_attribute<> *attrb
		= doc.allocate_attribute("name", "nodes_ligth" );
	xml_light->append_attribute(attrb);
	attrb = doc.allocate_attribute("type", "point" );
	xml_light->append_attribute(attrb);

	MOCK_EXPECT( sm, createLight ).once().with( "nodes_ligth" ).returns( light );
	MOCK_EXPECT( root, createChild ).once().with( "nodes_ligthNode" )
		.returns( light_node );
	MOCK_EXPECT( light_node, attachObject ).once().with( light );
	MOCK_EXPECT(light, setType).once().with( vl::graph::Light::LT_POINT );
	MOCK_EXPECT(light, setCastShadows).once().with( true );
	MOCK_EXPECT(light, setVisible).once().with( true );

	createLight( "spot_light", vl::graph::Light::LT_SPOTLIGHT );
	createLight( "dir_light", vl::graph::Light::LT_DIRECTIONAL );
	createLight( "point_light", vl::graph::Light::LT_POINT );

	BOOST_CHECK_NO_THROW( loadXML() );
}

BOOST_AUTO_TEST_SUITE_END()
