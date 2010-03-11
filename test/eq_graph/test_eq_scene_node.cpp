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
#include "equalizer_fixture.hpp"

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
	BOOST_CHECK_EQUAL( n2.getTranslation(), vl::vector::ZERO );
	BOOST_CHECK_EQUAL( n2.getScale(), vl::vector(1.0, 1.0, 1.0) );
	BOOST_CHECK_EQUAL( n2.getRotation(), vl::quaternion::IDENTITY );

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
	vl::scalar epsilon = 1e-8;

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

	BOOST_CHECK_NO_THROW( node.setRotation(vl::quaternion::IDENTITY) );

	vl::quaternion q = vl::quaternion::ZERO;
	BOOST_CHECK_THROW( node.rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getRotation(), vl::quaternion::IDENTITY) );

	q = vl::quaternion(2, 0, 0, 0);
	BOOST_CHECK_THROW( node.rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getRotation(), vl::quaternion::IDENTITY) );

	q = vl::quaternion(0, 0, 0, 2);
	BOOST_CHECK_THROW( node.rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node.getRotation(), vl::quaternion::IDENTITY) );
}

BOOST_AUTO_TEST_CASE( transformations_test )
{
	mock_scene_manager man;
	vl::cl::SceneNode node(&man);
	
	// Check that the node is correctly initialized
	BOOST_CHECK( vl::equal(node.getTranslation(), vl::vector::ZERO) );
	BOOST_CHECK( vl::equal(node.getRotation(), vl::quaternion::IDENTITY) );
	BOOST_CHECK( vl::equal(node.getScale(), vl::vector(1.0, 1.0, 1.0)) );

	// Translation test
	vl::vector v = vl::vector(0.0, 0.0, 50.0);
	BOOST_CHECK_NO_THROW( node.translate(v) );
	// Math library limitations, it does not provide epsilon comparison
	// And BOOS_CHECK_CLOSE does not work on ZERO, because it uses procentual
	// difference not absolute...
	BOOST_CHECK( vl::equal( node.getTranslation(), v ) );
	
	node.translate(2.0*v);
	BOOST_CHECK( vl::equal( node.getTranslation(), 3.0*v ) );

	v = vl::vector(10, 20, -20);
	BOOST_CHECK_NO_THROW( node.setTranslation(v) );
	BOOST_CHECK( vl::equal( node.getTranslation(), v ) );

	// 90 degree rotation around x
	vl::scalar half_a = (M_PI/2)/2;
	vl::quaternion q( std::sin(half_a), 0, 0, std::cos(half_a) );
	BOOST_REQUIRE( vl::equal(q.abs(), 1.0) );
	// 45 degree rotation around y
	half_a = (M_PI/4)/2;
	vl::quaternion r(0, std::sin(half_a), 0, std::cos(half_a));
	BOOST_REQUIRE( vl::equal(r.abs(), 1.0) );

	BOOST_CHECK_NO_THROW( node.rotate( q ) );
	BOOST_CHECK( vl::equal( node.getRotation(), q ) );

	BOOST_CHECK_NO_THROW( node.rotate( r ) );
	BOOST_CHECK( vl::equal( node.getRotation(), q*r ) );

	BOOST_CHECK_NO_THROW( node.setRotation( r ) );
	BOOST_CHECK( vl::equal( node.getRotation(), r ) );

	BOOST_CHECK_NO_THROW( node.setRotation( vl::quaternion::IDENTITY ) );
	BOOST_CHECK( vl::equal( node.getRotation(), vl::quaternion::IDENTITY ) );

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
	mock_scene_manager man;
	vl::graph::SceneNode *c1 = new vl::cl::SceneNode(&man, "Child1");
	vl::graph::SceneNode *c2 = new vl::cl::SceneNode(&man, "Child2");
	vl::graph::SceneNode *parent = new vl::cl::SceneNode(&man, "Parent");

	// Start conditions, no childs or parents
	BOOST_CHECK_EQUAL( c1->getParent(), (vl::graph::SceneNode *)0);
	BOOST_CHECK_EQUAL( c2->getParent(), (vl::graph::SceneNode *)0);
	BOOST_CHECK_EQUAL( parent->getChilds().size(), 0 );

	// add first child, with setParent
	c1->setParent( parent );
	BOOST_CHECK_EQUAL( c1->getParent(), parent);
	BOOST_CHECK_EQUAL( parent->getChilds().size(), 1 );
	if( parent->getChilds().size() > 0 )
	{ BOOST_CHECK_EQUAL( parent->getChilds().at(0), c1 ); }

	// add second child, with addChild
	parent->addChild( c2 );
	BOOST_CHECK_EQUAL( c2->getParent(), parent);
	BOOST_CHECK_EQUAL( parent->getChilds().size(), 2 );

	// remove the first child
	parent->removeChild( c1 );
	BOOST_CHECK_EQUAL( c1->getParent(), (vl::graph::SceneNode *)0 );
	BOOST_CHECK_EQUAL( parent->getChilds().size(), 1 );

	// Check throwing by trying to
	// add the second child again
	BOOST_CHECK_THROW( parent->addChild( c2 ), vl::duplicate );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
	BOOST_CHECK_NO_THROW( parent->addChild( c1 ) );
	BOOST_CHECK_EQUAL( c1->getParent(), parent );
	BOOST_CHECK_THROW( c2->setParent( parent ), vl::duplicate );
	BOOST_CHECK_EQUAL( c2->getParent(), parent );
}

BOOST_AUTO_TEST_CASE( attachment_test )
{
	mock_scene_manager man;
	vl::graph::SceneNode *n= new vl::cl::SceneNode(&man);
	vl::graph::MovableObject obj1, obj2;

	BOOST_CHECK_EQUAL( n->getAttached().size(), 0 );
	BOOST_CHECK_NO_THROW( n->attachObject(&obj1) );
	BOOST_CHECK_EQUAL( n->getAttached().size(), 1 );
	if( n->getAttached().size() > 0 )
	{ BOOST_CHECK_EQUAL( n->getAttached().at(0), &obj1 ); }
	BOOST_CHECK_THROW( n->attachObject(&obj1), vl::duplicate );
	BOOST_CHECK_EQUAL( n->getAttached().size(), 1 );

	BOOST_CHECK_NO_THROW( n->attachObject(&obj2) );
	BOOST_CHECK_EQUAL( n->getAttached().size(), 2 );
}

// Used for callback test so we see if the callback is called
// FIXME Callback design is still in half way, should they be called when
// the public interface functions are called or when the data is deserialized?
MOCK_BASE_CLASS( mock_scene_node, vl::cl::SceneNode )
{
	MOCK_METHOD( _setTransform, 2 );
	MOCK_METHOD( _setScale, 1 );
	MOCK_METHOD( _attachObject, 1 );
	MOCK_METHOD( _detachObject, 1 );
	MOCK_METHOD( _addChild, 1 );
	MOCK_METHOD( _removeChild, 1 );
};

BOOST_AUTO_TEST_CASE( callbacks )
{

}

BOOST_FIXTURE_TEST_CASE( equalizer_test, EqFixture )
{
	BOOST_CHECK( config );
	//for( size_t i = 0; i < 1000; ++i )

	// Test transmitting translation
	mainloop();

	node->setTranslation( TRANS_VEC[0] );
	node->commit();
	mainloop();

	// Test transmitting rotation
	node->setRotation( ROT_QUAT[0] );
	node->commit();
	mainloop();

	node->setScale( SCALE_VEC[0] );
	node->setRotation( ROT_QUAT[1] );
	node->setTranslation( TRANS_VEC[1] );
	node->commit();
	mainloop();

	// TODO test attachement

	// TODO test children
}
