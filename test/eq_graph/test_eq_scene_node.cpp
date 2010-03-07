#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE eq_scene_node
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <turtle/mock.hpp>

#include "eq_graph/eq_scene_node.hpp"
//#include "eq_graph/eq_scene_manager.hpp"
#include "interface/scene_manager.hpp"

#include "base/exceptions.hpp"

using vl::graph::SceneNode;
using vl::graph::SceneManager;
using vl::graph::MovableObject;

MOCK_BASE_CLASS( mock_scene_manager, SceneManager )
{
	MOCK_METHOD_EXT( destroy, 1, void( MovableObject* ), destroyObj)
	MOCK_METHOD_EXT( destroy, 1, void( SceneNode* ), destroyNode )
	MOCK_METHOD( getRootNode, 0 )
	MOCK_METHOD( createNode, 1 )
	MOCK_METHOD( createEntity, 2 )
	MOCK_METHOD( createCamera, 1 )
	MOCK_METHOD_EXT( getNode, 1, SceneNode*( uint32_t ), getById )
	MOCK_METHOD_EXT( getNode, 1, SceneNode*( std::string const & ), getByName )
	MOCK_METHOD( getObject, 1 )
	MOCK_METHOD( finalize, 0 )
};

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

}

BOOST_AUTO_TEST_CASE( attachment_test )
{

}
