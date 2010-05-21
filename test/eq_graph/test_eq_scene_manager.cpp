#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE eq_scene_manager

#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_scene_manager.hpp"

#include "mocks.hpp"

struct SceneManFixture
{
	SceneManFixture( void )
		: man( new vl::cl::SceneManager( "Name" ) )
	{
	}

	vl::graph::SceneManagerRefPtr man;
};

struct NodeFactoryFixture : public SceneManFixture
{
	NodeFactoryFixture( void )
		: sn_fac( new mock::NodeFactory )
	{
		BOOST_CHECK_NO_THROW( man->setSceneNodeFactory( sn_fac ) );
	}

	mock::NodeFactoryPtr sn_fac;
};


BOOST_AUTO_TEST_CASE( constructor_test )
{
	vl::graph::SceneManagerRefPtr sm;
	BOOST_CHECK_NO_THROW( sm.reset( new vl::cl::SceneManager("Name") ) );
	BOOST_CHECK_EQUAL( sm->getName(), "Name" );

	BOOST_CHECK_THROW( vl::cl::SceneManager(""), vl::empty_param );
}

BOOST_FIXTURE_TEST_SUITE( ObjectTests, SceneManFixture )

BOOST_AUTO_TEST_CASE( entity_test )
{
	// Add Entity factory
	mock::ObjectFactoryPtr obj_fac( new mock::ObjectFactory );
	MOCK_EXPECT( obj_fac, typeName ).at_least(1).returns( "Entity" );
	man->addMovableObjectFactory( obj_fac );

	vl::NamedValuePairList params;
	params["mesh"] = "ent.mesh";
	mock::EntityPtr obj( new mock::Entity );
	MOCK_EXPECT( obj_fac, create ).once().with( "ent", params ).returns( obj );
	MOCK_EXPECT( obj, setManager ).once().with( man );

	// Test Entity creation
	vl::graph::EntityRefPtr ent = man->createEntity( "ent", "ent.mesh" );
	BOOST_CHECK( ent );
	
	// Test object finding
}

BOOST_AUTO_TEST_CASE( camera_test )
{
	// Add Camera factory
	mock::ObjectFactoryPtr obj_fac( new mock::ObjectFactory );
	MOCK_EXPECT( obj_fac, typeName ).at_least(1).returns( "Camera" );
	man->addMovableObjectFactory( obj_fac );

	mock::CameraPtr obj( new mock::Camera );
	MOCK_EXPECT( obj_fac, create ).once().with( "cam", vl::NamedValuePairList() )
		.returns( obj );
	MOCK_EXPECT( obj, setManager ).once().with( man );

	vl::graph::CameraRefPtr cam = man->createCamera( "cam" );
	BOOST_CHECK( cam );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( FactoryTests, NodeFactoryFixture )

BOOST_AUTO_TEST_CASE( factory_test )
{
	mock::ObjectFactoryPtr obj_fact( new mock::ObjectFactory );
	mock::ObjectFactoryPtr obj_fact2( new mock::ObjectFactory );
	mock::ObjectFactoryPtr obj_fact3( new mock::ObjectFactory );
	
	MOCK_EXPECT( obj_fact, typeName ).at_least(1).returns( "Entity" );
	MOCK_EXPECT( obj_fact2, typeName ).at_least(2).returns( "Entity" );
	MOCK_EXPECT( obj_fact3, typeName ).at_least(1).returns( "Light" );

	// add one movable object factory
	BOOST_CHECK_NO_THROW( man->addMovableObjectFactory( obj_fact ) );
	std::vector<std::string> names = man->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 1u );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Entity" ) !=
			names.end() );
	// Try to add non existing movable object factory
	BOOST_CHECK_THROW( man->addMovableObjectFactory(
				vl::graph::MovableObjectFactoryPtr() ), vl::null_pointer );
	BOOST_CHECK_EQUAL( man->movableObjectFactories().size(), 1u );
	// add the factory we already added again
	BOOST_CHECK_THROW( man->addMovableObjectFactory( obj_fact ), vl::exception );
	BOOST_CHECK_EQUAL( man->movableObjectFactories().size(), 1u );
	// add another factory with a different type
	BOOST_CHECK_NO_THROW( man->addMovableObjectFactory( obj_fact3 ) );
	names = man->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 2u );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Light" ) !=
			names.end() );
	// add another factory with the same type without overwrite
	BOOST_CHECK_THROW( man->addMovableObjectFactory( obj_fact2 ), vl::exception );
	BOOST_CHECK_EQUAL( man->movableObjectFactories().size(), 2u );
	// add another factory with the same type with overwrite
	BOOST_CHECK_NO_THROW( man->addMovableObjectFactory( obj_fact2, true ) );
	names = man->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 2u );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Entity" ) !=
			names.end() );

	// test factory removals
	BOOST_CHECK_NO_THROW( man->removeMovableObjectFactory( obj_fact2 ) );
	names = man->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 1u );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Light" ) !=
			names.end() );
	//  trying to remove not existing factory
	BOOST_CHECK_THROW( man->removeMovableObjectFactory( obj_fact ), vl::exception );
	BOOST_CHECK_EQUAL( man->movableObjectFactories().size(), 1u );
	// remove by name
	BOOST_CHECK_NO_THROW( man->removeMovableObjectFactory( "Light" ) );
	BOOST_CHECK_EQUAL( man->movableObjectFactories().size(), 0u );
}

BOOST_AUTO_TEST_CASE( node_test )
{
	MOCK_EXPECT( sn_fac, create ).once().with( man, "Root")
		.returns( vl::graph::SceneNodeRefPtr( new mock::SceneNode ) );

	MOCK_EXPECT( sn_fac, create ).once().with( man, "Node")
		.returns( vl::graph::SceneNodeRefPtr( new mock::SceneNode ) );

	// Set the node factory to mock
	man->setSceneNodeFactory( sn_fac );

	// Test that root node is created
	vl::graph::SceneNodeRefPtr root;
	BOOST_CHECK_NO_THROW( root = man->getRootNode() );
	BOOST_CHECK( root );

	// Test creating new nodes
	vl::graph::SceneNodeRefPtr n;
	BOOST_CHECK_NO_THROW( n = man->createNode("Node") );
	BOOST_CHECK( n );

	// Test finding Nodes, by name

	// Equalizer needs to be initialized before we can search by IDs
}

// Test replacing the scene node factory
BOOST_AUTO_TEST_CASE( replace_SceneNode_factory )
{
	// Initialize
	mock::NodeFactoryPtr sn_fac2( new mock::NodeFactory );

	MOCK_EXPECT( sn_fac, create ).once().with( man, "Root")
		.returns( vl::graph::SceneNodeRefPtr( new mock::SceneNode ) );

	// Test that we can not change SceneNode factory if the scene graph exists
	vl::graph::SceneNodeRefPtr root = man->getRootNode();
	BOOST_CHECK_THROW( man->setSceneNodeFactory( sn_fac2 ), vl::exception );

	MOCK_VERIFY( sn_fac, create );

	MOCK_EXPECT( sn_fac2, create ).once().with( man, "Root")
		.returns( vl::graph::SceneNodeRefPtr( new mock::SceneNode ) );

	// Test that we can destroy the SceneGraph
	man->destroyGraph();
	// Test that we can set new factory if there is no SceneGraph
	BOOST_CHECK_NO_THROW( man->setSceneNodeFactory( sn_fac2 ) );
	man->getRootNode();

	MOCK_VERIFY( sn_fac2, create );
}

BOOST_AUTO_TEST_CASE( find_node )
{

}

BOOST_AUTO_TEST_SUITE_END()

