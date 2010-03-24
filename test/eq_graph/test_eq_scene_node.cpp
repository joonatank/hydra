#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE eq_scene_node
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <turtle/mock.hpp>

#include "eq_graph/eq_scene_node.hpp"
//#include "eq_graph/eq_scene_manager.hpp"
#include "interface/scene_manager.hpp"

#include "base/exceptions.hpp"

#include "mock_scene_manager.hpp"
#include "scenenode_equalizer_fixture.hpp"

using vl::graph::SceneNode;
using vl::graph::SceneManager;
using vl::graph::MovableObject;

BOOST_AUTO_TEST_CASE( constructors_test )
{
	// TODO this should throw as scene manager can't be NULL
	BOOST_CHECK_THROW( vl::cl::SceneNode n1(0), vl::null_pointer );

	mock_scene_manager mock_man;
	SceneManager *man = &mock_man;

	// Check that the creator is recorded correctly
	vl::cl::SceneNode n2(man);
	BOOST_CHECK_EQUAL( n2.getCreator(), man );

	// Check that the scene node is initialized correctly
	BOOST_CHECK_EQUAL( n2.getPosition(), vl::vector::ZERO );
	BOOST_CHECK_EQUAL( n2.getScale(), vl::vector(1.0, 1.0, 1.0) );
	BOOST_CHECK_EQUAL( n2.getOrientation(), vl::quaternion::IDENTITY );

	// Test childs, parents and attached are initialized correctly
	BOOST_CHECK( n2.getParent() == NULL );
	BOOST_CHECK_EQUAL( n2.getChilds().size(), 0);
	BOOST_CHECK_EQUAL( n2.getAttached().size(), 0);

	// TODO this should rather return random name, not empty, I think
	BOOST_CHECK_EQUAL( n2.getName(), std::string() );

	std::string name("Node");
	vl::cl::SceneNode n3(man, name );

	BOOST_CHECK_EQUAL( n3.getName(), name );
}

BOOST_AUTO_TEST_CASE( trans_throws_test )
{
	mock_scene_manager man;
	vl::cl::SceneNode node(&man);

	// Critical transformation checks
	// Scale can not be zero

	BOOST_CHECK_NO_THROW( node.setScale(vl::vector(1.0, 1.0, 1.0)) );
	BOOST_CHECK_THROW( node.setScale(vl::vector::ZERO), vl::zero_scale  );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getScale(), vl::vector(1.0,1.0,1.0)) );

	BOOST_CHECK_THROW( node.scale(0.0), vl::zero_scale );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getScale(), vl::vector(1.0,1.0,1.0)) );

	BOOST_CHECK_THROW( node.setScale(0.0), vl::zero_scale );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getScale(), vl::vector(1.0,1.0,1.0)) );

	// Can not rotate with quaternions including scale factor

	BOOST_CHECK_NO_THROW( node.setOrientation(vl::quaternion::IDENTITY) );

	vl::quaternion q = vl::quaternion::ZERO;
	BOOST_CHECK_THROW( node.rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getOrientation(), vl::quaternion::IDENTITY) );

	q = vl::quaternion(2, 0, 0, 0);
	BOOST_CHECK_THROW( node.rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getOrientation(), vl::quaternion::IDENTITY) );

	q = vl::quaternion(0, 0, 0, 2);
	BOOST_CHECK_THROW( node.rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getOrientation(), vl::quaternion::IDENTITY) );
}

BOOST_AUTO_TEST_CASE( transformations_test )
{
	mock_scene_manager man;
	vl::cl::SceneNode node(&man);
	
	// Check that the node is correctly initialized
	BOOST_CHECK( vl::equal(node.getPosition(), vl::vector::ZERO) );
	BOOST_CHECK( vl::equal(node.getOrientation(), vl::quaternion::IDENTITY) );
	BOOST_CHECK( vl::equal(node.getScale(), vl::vector(1.0, 1.0, 1.0)) );

	// Position test
	vl::vector v = vl::vector(0.0, 0.0, 50.0);
	BOOST_CHECK_NO_THROW( node.translate(v) );
	// Math library limitations, it does not provide epsilon comparison
	// And BOOS_CHECK_CLOSE does not work on ZERO, because it uses procentual
	// difference not absolute...
	BOOST_CHECK( vl::equal( node.getPosition(), v ) );
	
	node.translate(2.0*v);
	BOOST_CHECK( vl::equal( node.getPosition(), 3.0*v ) );

	v = vl::vector(10, 20, -20);
	BOOST_CHECK_NO_THROW( node.setPosition(v) );
	BOOST_CHECK( vl::equal( node.getPosition(), v ) );

	// 90 degree rotation around x
	vl::scalar half_a = (M_PI/2)/2;
	vl::quaternion q( std::sin(half_a), 0, 0, std::cos(half_a) );
	BOOST_REQUIRE( vl::equal(q.abs(), 1.0) );
	// 45 degree rotation around y
	half_a = (M_PI/4)/2;
	vl::quaternion r(0, std::sin(half_a), 0, std::cos(half_a));
	BOOST_REQUIRE( vl::equal(r.abs(), 1.0) );

	BOOST_CHECK_NO_THROW( node.rotate( q ) );
	BOOST_CHECK( vl::equal( node.getOrientation(), q ) );

	BOOST_CHECK_NO_THROW( node.rotate( r ) );
	BOOST_CHECK( vl::equal( node.getOrientation(), q*r ) );

	BOOST_CHECK_NO_THROW( node.setOrientation( r ) );
	BOOST_CHECK( vl::equal( node.getOrientation(), r ) );

	BOOST_CHECK_NO_THROW( node.setOrientation( vl::quaternion::IDENTITY ) );
	BOOST_CHECK( vl::equal( node.getOrientation(), vl::quaternion::IDENTITY ) );

	// scale
	BOOST_CHECK_NO_THROW( node.setScale( 10 ) );
	BOOST_CHECK( vl::equal( node.getScale(), vl::vector(10.0, 10.0, 10.0) ) );

	BOOST_CHECK_NO_THROW( node.scale( 0.5 ) );
	BOOST_CHECK( vl::equal( node.getScale(), vl::vector(5.0, 5.0, 5.0) ) );

	v = vl::vector(1, 2, 0.5 );
	BOOST_CHECK_NO_THROW( node.scale( v ) );
	BOOST_CHECK( vl::equal( node.getScale(), 5.0*v ) );
}

BOOST_AUTO_TEST_CASE( child_test )
{
	mock_scene_manager *man = new mock_scene_manager;
	vl::graph::SceneNode *c1 = new vl::cl::SceneNode(man, "Child1");
	vl::graph::SceneNode *c2 = new vl::cl::SceneNode(man, "Child2");
	vl::graph::SceneNode *parent = new vl::cl::SceneNode(man, "Parent");

	// Start conditions, no childs or parents
	BOOST_CHECK_EQUAL( c1->getParent(), (vl::graph::SceneNode *)0);
	BOOST_CHECK_EQUAL( c2->getParent(), (vl::graph::SceneNode *)0);
	BOOST_CHECK_EQUAL( parent->numChildren(), 0 );

	// add first child, with setParent
	c1->setParent( parent );
	BOOST_CHECK_EQUAL( c1->getParent(), parent);
	BOOST_CHECK_EQUAL( parent->numChildren(), 1 );
	if( parent->numChildren() > 0 )
	{ BOOST_CHECK_EQUAL( parent->getChilds().at(0), c1 ); }

	// add second child, with addChild
	parent->addChild( c2 );
	BOOST_CHECK_EQUAL( c2->getParent(), parent);
	BOOST_CHECK_EQUAL( parent->numChildren(), 2 );

	// Removing by name
	BOOST_CHECK_EQUAL( parent->removeChild( "Child1" ), c1);
	BOOST_REQUIRE_EQUAL( parent->numChildren(), 1 );
	BOOST_CHECK_EQUAL( c1->getParent(), (vl::graph::SceneNode *)0 );
	parent->addChild( c1 );
	BOOST_CHECK_EQUAL( parent->numChildren(), 2 );

	// Getting by name
	BOOST_CHECK_EQUAL( parent->getChild( "Child2" ), c2 );
	BOOST_CHECK_EQUAL( parent->getChild( "Child1" ), c1 );
	BOOST_CHECK_EQUAL( parent->numChildren(), 2 );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
	BOOST_CHECK_EQUAL( c1->getParent(), parent );

	// Disallow adding node multiple times
	BOOST_CHECK_THROW( parent->addChild( c2 ), vl::duplicate );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
	BOOST_CHECK_NO_THROW( c2->addChild( c1 ) );
	BOOST_CHECK_NO_THROW( parent->addChild( c1 ) );
	BOOST_CHECK_EQUAL( c1->getParent(), parent );
	BOOST_CHECK_THROW( c2->setParent( parent ), vl::duplicate );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );

	// Disallow setting node as it's own parent
	BOOST_CHECK_THROW( c2->addChild( c2 ), vl::exception );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
	BOOST_CHECK_THROW( c2->setParent( c2 ), vl::exception );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
	BOOST_CHECK_THROW( c2->setParent( c2 ), vl::exception );

	// Disallow setting null parent
	BOOST_CHECK_THROW( c2->setParent( 0 ), vl::null_pointer );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
	BOOST_CHECK_THROW( c2->addChild( 0 ), vl::null_pointer );

	delete man;
	delete parent;
	delete c1;
	delete c2;
}

BOOST_AUTO_TEST_CASE( node_creation_test )
{
	mock_scene_manager man;

	MOCK_EXPECT( man, getRootNode ).once()
		.returns( new vl::cl::SceneNode(&man, "Root") );
	MOCK_EXPECT( man, createNode ).exactly(2).with("Child")
		.returns( new vl::cl::SceneNode(&man, "Child") );
	MOCK_EXPECT( man, createNode ).once().with("Child2")
		.returns( new vl::cl::SceneNode(&man, "Child2") );
	vl::graph::SceneNode *root = man.getRootNode();
	BOOST_REQUIRE( root );
	BOOST_CHECK_EQUAL( root->getName(), "Root" );

	// Creating childs, that is calling SceneManager
	vl::graph::SceneNode *child = 0;
	vl::graph::SceneNode *child2 = 0;
	BOOST_CHECK_NO_THROW( child = root->createChild("Child") );
	BOOST_CHECK_NO_THROW( child2 = root->createChild("Child2") );
	BOOST_CHECK_THROW( root->createChild("Child"), vl::duplicate );
}

BOOST_AUTO_TEST_CASE( attachment_test )
{
	mock_scene_manager man;
	vl::graph::SceneNode *n= new vl::cl::SceneNode(&man);
	vl::graph::MovableObject obj1, obj2;

	BOOST_CHECK_EQUAL( n->numAttached(), 0 );
	BOOST_CHECK_NO_THROW( n->attachObject(&obj1) );
	BOOST_CHECK_EQUAL( n->numAttached(), 1 );
	if( n->numAttached() > 0 )
	{ BOOST_CHECK_EQUAL( n->getAttached().at(0), &obj1 ); }
	BOOST_CHECK_THROW( n->attachObject(&obj1), vl::duplicate );
	BOOST_CHECK_EQUAL( n->numAttached(), 1 );

	BOOST_CHECK_NO_THROW( n->attachObject(&obj2) );
	BOOST_CHECK_EQUAL( n->numAttached(), 2 );
}

// TODO this should be moved to different directory where we have
// all the synchronization tests.
BOOST_FIXTURE_TEST_CASE( equalizer_test, EqFixture )
{
	BOOST_CHECK( config );

	// Test transmitting translation

	while( sync_fixture->testRemaining() )
	{
		mainloop();
	}
}
