#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE eq_scene_manager

#include <boost/test/unit_test.hpp>

#include <turtle/mock.hpp>

#include "eq_graph/eq_scene_manager.hpp"

using vl::cl::SceneManager;

namespace vl
{
namespace cl
{
	// TODO provide stub implementation of vl::cl::SceneNode here
}
}

BOOST_AUTO_TEST_CASE( constructor_test )
{
	BOOST_CHECK_NO_THROW( SceneManager sm("Name") );
}

BOOST_AUTO_TEST_CASE( movable_object_test )
{
	SceneManager sm("Name");

	// Test Entity creation
	vl::graph::MovableObject *ent = sm.createEntity( "ent", "ent.mesh" );
	BOOST_CHECK( ent );
	
	// Test object finding

}

BOOST_AUTO_TEST_CASE( node_test )
{
	SceneManager sm("Name");

	// Test that root node is created
	vl::graph::SceneNode *root;
	BOOST_REQUIRE( root = sm.getRootNode() );

	// Test creating new nodes
	vl::graph::SceneNode *n = sm.createNode("Node");
	BOOST_REQUIRE(n);
	// We should not have parent yet
	BOOST_CHECK( !n->getParent() );

	BOOST_CHECK_NO_THROW( root->addChild(n) );
	BOOST_CHECK_EQUAL( n->getParent(), root );
	BOOST_CHECK_EQUAL( root->getChilds().size(), 1 );
	// Use the SceneNode to call scene manager to create Nodes
	BOOST_CHECK_NO_THROW( root->createChild( "Node2" ) );
	BOOST_CHECK_EQUAL( root->getChilds().size(), 2 );

	// Test finding Nodes, by name because Equalizer needs to be initialized
	// before we can search by IDs
}
