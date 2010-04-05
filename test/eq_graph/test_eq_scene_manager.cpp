#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE eq_scene_manager

#include <boost/test/unit_test.hpp>

#include <turtle/mock.hpp>

#include "eq_graph/eq_scene_manager.hpp"

MOCK_BASE_CLASS( mock_node_factory, vl::graph::SceneNodeFactory )
{
	MOCK_METHOD( create, 2 )
};

MOCK_BASE_CLASS( mock_object_factory, vl::graph::MovableObjectFactory )
{
	MOCK_METHOD( create, 2 )
	MOCK_METHOD( typeName, 0 )
};

MOCK_BASE_CLASS( mock_movable_object, vl::graph::MovableObject )
{

};

MOCK_BASE_CLASS( mock_scene_node, vl::graph::SceneNode )
{
	MOCK_METHOD( getName, 0 )
	MOCK_METHOD( translate, 2 )
	MOCK_METHOD( setPosition, 2 )
	MOCK_METHOD( getPosition, 1 )
	MOCK_METHOD( rotate, 2 )
	MOCK_METHOD( setOrientation, 2 )
	MOCK_METHOD( getOrientation, 1 )
	MOCK_METHOD( lookAt, 1 )
	MOCK_METHOD_EXT( scale, 1, void( vl::vector const & ), scaleVector )
	MOCK_METHOD_EXT( scale, 1, void( vl::scalar const ), scaleScalar )
	MOCK_METHOD( setScale, 1 )
	MOCK_METHOD( getScale, 0 )
	MOCK_METHOD( attachObject, 1 )
	MOCK_METHOD( detachObject, 1 )
	MOCK_METHOD( getAttached, 0 )
	MOCK_METHOD( numAttached, 0 )
	MOCK_METHOD( createChild, 1 )
	MOCK_METHOD( setParent, 1 )
	MOCK_METHOD( addChild, 1 )
	MOCK_METHOD_EXT( removeChild, 1, void( vl::graph::SceneNodeRefPtr ), removeChild )
	MOCK_METHOD_EXT( removeChild, 1,
			vl::graph::SceneNodeRefPtr( uint16_t ), removeChildByIndex )
	MOCK_METHOD_EXT( removeChild, 1,
			vl::graph::SceneNodeRefPtr( std::string const & ), removeChildByName )
	MOCK_METHOD_EXT( getChild, 1, vl::graph::SceneNodeRefPtr( uint16_t ), getChildByIndex )
	MOCK_METHOD_EXT( getChild, 1,
			vl::graph::SceneNodeRefPtr( std::string const & ), getChildByName )
	MOCK_METHOD( getChilds, 0 )
	MOCK_METHOD( numChildren, 0 )
	MOCK_METHOD( getParent, 0 )
	MOCK_METHOD( getManager, 0 )
};

BOOST_AUTO_TEST_CASE( constructor_test )
{
	vl::graph::SceneManagerRefPtr sm;
	BOOST_CHECK_NO_THROW( sm.reset( new vl::cl::SceneManager("Name") ) );

	BOOST_CHECK_THROW( vl::cl::SceneManager(""), vl::empty_param );
}

BOOST_AUTO_TEST_CASE( factory_test )
{
	vl::graph::SceneManagerRefPtr sm( new vl::cl::SceneManager("Name") );
	boost::shared_ptr<mock_node_factory> node_fact( new mock_node_factory );
	boost::shared_ptr<mock_object_factory> obj_fact( new mock_object_factory );
	boost::shared_ptr<mock_object_factory> obj_fact2( new mock_object_factory );
	boost::shared_ptr<mock_object_factory> obj_fact3( new mock_object_factory );
	
	MOCK_EXPECT( obj_fact, typeName ).at_least(1).returns( "Entity" );
	MOCK_EXPECT( obj_fact2, typeName ).at_least(2).returns( "Entity" );
	MOCK_EXPECT( obj_fact3, typeName ).at_least(1).returns( "Light" );

	// Test adding sceneNode factory
	// TODO add test case to add scene nodes, trying to add
	// a new node factory and destroying the graph
	BOOST_CHECK_NO_THROW( sm->setSceneNodeFactory( node_fact ) );

	// add one movable object factory
	BOOST_CHECK_NO_THROW( sm->addMovableObjectFactory( obj_fact ) );
	std::vector<std::string> names = sm->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 1 );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Entity" ) !=
			names.end() );
	// Try to add non existing movable object factory
	BOOST_CHECK_THROW( sm->addMovableObjectFactory(
				vl::graph::MovableObjectFactoryPtr() ), vl::null_pointer );
	BOOST_CHECK_EQUAL( sm->movableObjectFactories().size(), 1 );
	// add the factory we already added again
	BOOST_CHECK_THROW( sm->addMovableObjectFactory( obj_fact ), vl::exception );
	BOOST_CHECK_EQUAL( sm->movableObjectFactories().size(), 1 );
	// add another factory with a different type
	BOOST_CHECK_NO_THROW( sm->addMovableObjectFactory( obj_fact3 ) );
	names = sm->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 2 );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Light" ) !=
			names.end() );
	// add another factory with the same type without overwrite
	BOOST_CHECK_THROW( sm->addMovableObjectFactory( obj_fact2 ), vl::exception );
	BOOST_CHECK_EQUAL( sm->movableObjectFactories().size(), 2 );
	// add another factory with the same type with overwrite
	BOOST_CHECK_NO_THROW( sm->addMovableObjectFactory( obj_fact2, true ) );
	names = sm->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 2 );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Entity" ) !=
			names.end() );

//	MOCK_VERIFY( obj_fact, typeName );
//	MOCK_VERIFY( obj_fact2, typeName );
//	MOCK_VERIFY( obj_fact3, typeName );

	// test factory removals
	BOOST_CHECK_NO_THROW( sm->removeMovableObjectFactory( obj_fact2 ) );
	names = sm->movableObjectFactories();
	BOOST_CHECK_EQUAL( names.size(), 1 );
	BOOST_CHECK( std::find( names.begin(), names.end(), "Light" ) !=
			names.end() );
	//  trying to remove not existing factory
	BOOST_CHECK_THROW( sm->removeMovableObjectFactory( obj_fact ), vl::exception );
	BOOST_CHECK_EQUAL( sm->movableObjectFactories().size(), 1 );
	// remove by name
	BOOST_CHECK_NO_THROW( sm->removeMovableObjectFactory( "Light" ) );
	BOOST_CHECK_EQUAL( sm->movableObjectFactories().size(), 0 );
}

BOOST_AUTO_TEST_CASE( movable_object_test )
{
	vl::graph::SceneManagerRefPtr sm( new vl::cl::SceneManager("Name") );
	boost::shared_ptr<mock_object_factory> obj_fac( new mock_object_factory );

	MOCK_EXPECT( obj_fac, typeName ).at_least(1).returns( "Entity" );

	sm->addMovableObjectFactory( obj_fac );

	vl::NamedValuePairList params;
	params["mesh"] = "ent.mesh";
	MOCK_EXPECT( obj_fac, create ).once().with( "ent", params )
		.returns( vl::graph::MovableObjectRefPtr( new mock_movable_object ) );

	// Test Entity creation
	vl::graph::MovableObjectRefPtr ent = sm->createEntity( "ent", "ent.mesh" );
	BOOST_CHECK( ent );
	
	// Test object finding

}

BOOST_AUTO_TEST_CASE( node_test )
{
	vl::graph::SceneManagerRefPtr sm( new vl::cl::SceneManager("Name") );
	boost::shared_ptr<mock_node_factory> sn_fact( new mock_node_factory );

	MOCK_EXPECT( sn_fact, create ).once().with( sm, "Root")
		.returns( vl::graph::SceneNodeRefPtr( new mock_scene_node ) );

	MOCK_EXPECT( sn_fact, create ).once().with( sm, "Node")
		.returns( vl::graph::SceneNodeRefPtr( new mock_scene_node ) );

	// Set the node factory to mock
	sm->setSceneNodeFactory( sn_fact );

	// Test that root node is created
	vl::graph::SceneNodeRefPtr root;
	BOOST_CHECK_NO_THROW( root = sm->getRootNode() );
	BOOST_CHECK( root );

	// Test creating new nodes
	vl::graph::SceneNodeRefPtr n;
	BOOST_CHECK_NO_THROW( n = sm->createNode("Node") );
	BOOST_CHECK( n );

	// Test finding Nodes, by name

	// Equalizer needs to be initialized before we can search by IDs
}

BOOST_AUTO_TEST_CASE( scene_node_factory_test )
{
	// Initialize
	vl::graph::SceneManagerRefPtr sm( new vl::cl::SceneManager("Name") );
	boost::shared_ptr<mock_node_factory> sn_fac( new mock_node_factory );
	boost::shared_ptr<mock_node_factory> sn_fac2( new mock_node_factory );

	MOCK_EXPECT( sn_fac, create ).once().with( sm, "Root")
		.returns( vl::graph::SceneNodeRefPtr( new mock_scene_node ) );

	sm->setSceneNodeFactory( sn_fac );

	// Test that we can not change SceneNode factory if the scene graph exists
	vl::graph::SceneNodeRefPtr root = sm->getRootNode();
	BOOST_CHECK_THROW( sm->setSceneNodeFactory( sn_fac2 ), vl::exception );

	MOCK_VERIFY( sn_fac, create );

	MOCK_EXPECT( sn_fac2, create ).once().with( sm, "Root")
		.returns( vl::graph::SceneNodeRefPtr( new mock_scene_node ) );

	// Test that we can destroy the SceneGraph
	sm->destroyGraph();
	// Test that we can set new factory if there is no SceneGraph
	BOOST_CHECK_NO_THROW( sm->setSceneNodeFactory( sn_fac2 ) );
	sm->getRootNode();

	MOCK_VERIFY( sn_fac2, create );
}
