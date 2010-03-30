#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE eq_scene_manager

#include <boost/test/unit_test.hpp>

#include <turtle/mock.hpp>

#include "eq_graph/eq_scene_manager.hpp"

using vl::cl::SceneManager;


BOOST_AUTO_TEST_CASE( constructor_test )
{
	vl::graph::SceneManager *sm;
	BOOST_CHECK_NO_THROW( sm = new SceneManager("Name") );

	BOOST_CHECK_THROW( SceneManager(""), vl::empty_param );

	delete sm;
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
	BOOST_CHECK( dynamic_cast<vl::cl::SceneNode *>(n) );

	// Test finding Nodes, by name
	// FIXME
	// Does not work if we use stub objects

	// Equalizer needs to be initialized before we can search by IDs
}
