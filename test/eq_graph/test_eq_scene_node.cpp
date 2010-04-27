#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE eq_scene_node
#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_scene_node.hpp"

#include "interface/scene_manager.hpp"

#include "base/exceptions.hpp"
#include "base/string_utils.hpp"

#include "mock_scene_manager.hpp"

namespace vl
{
	using namespace graph;
}

struct SceneNodeFixture
{
	SceneNodeFixture( void )
		: man( new mock_scene_manager ),
		  node( new vl::cl::SceneNode( man, "Node" ) )
	{}

	~SceneNodeFixture( void ) { }

//	vl::SceneManagerRefPtr man;
	boost::shared_ptr<mock_scene_manager> man;
	vl::SceneNodeRefPtr node;
};

struct ChildNodeFixture : public SceneNodeFixture
{
	ChildNodeFixture( void )
		: child1( new vl::cl::SceneNode( man, "Child1" ) ),
		  child2( new vl::cl::SceneNode( man, "Child2" ) ),
		  obj1( new vl::MovableObject ),
		  obj2( new vl::MovableObject )
	{}

	~ChildNodeFixture( void ) { }

	vl::SceneNodeRefPtr child1;
	vl::SceneNodeRefPtr child2;
	vl::MovableObjectRefPtr obj1;
	vl::MovableObjectRefPtr obj2;
};

struct FindNodeFixture : public SceneNodeFixture
{
	FindNodeFixture( void )
	{
		// Generate nodes to the scene graph
		for( int i = 0; i < 10; i++ )
		{
			// Top level
			std::string name = "Node-" + vl::string_convert(i);
			vl::SceneNodeRefPtr parent( new vl::cl::SceneNode( man, name ) );
			node->addChild( parent );
			node_cont.push_back( parent );
			for( int j = 0; j < 10; j++ )
			{
				std::string name = "Node-" + vl::string_convert(i)
					+ "-" + vl::string_convert(j);
				vl::SceneNodeRefPtr ptr( new vl::cl::SceneNode( man, name ) );
				parent->addChild( ptr );
				node_cont.push_back( ptr );
			}
		}

		// Generate some objects, TODO needs mock object with typeName function
	}

	~FindNodeFixture( void ) {}

	std::vector<vl::SceneNodeRefPtr> node_cont;
	std::vector<vl::MovableObjectRefPtr> obj_cont;
};

BOOST_FIXTURE_TEST_SUITE( BasicSceneNode, SceneNodeFixture )

BOOST_AUTO_TEST_CASE( constructors_test )
{
	// should throw as scene manager can't be NULL
	vl::SceneManagerRefPtr mgr;
	BOOST_CHECK( !mgr );
	BOOST_CHECK_THROW( vl::cl::SceneNode n1( mgr ), vl::null_pointer );

	// Check that the creator is recorded correctly
	BOOST_CHECK_EQUAL( (void *)( node->getManager().get() ), (void *)(man.get()) );

	// Check that the scene node is initialized correctly
	BOOST_CHECK_EQUAL( node->getPosition(), vl::vector::ZERO );
	BOOST_CHECK_EQUAL( node->getScale(), vl::vector(1.0, 1.0, 1.0) );
	BOOST_CHECK_EQUAL( node->getOrientation(), vl::quaternion::IDENTITY );

	// Test childs, parents and attached are initialized correctly
	BOOST_CHECK( !node->getParent() );
	BOOST_CHECK_EQUAL( node->getChilds().size(), 0);
	BOOST_CHECK_EQUAL( node->getAttached().size(), 0);

	BOOST_CHECK_EQUAL( node->getName(), "Node" );

	node.reset( new vl::cl::SceneNode( man ) );
	// TODO this should rather return random name, not empty, I think
	BOOST_CHECK_EQUAL( node->getName(), std::string() );
}

BOOST_AUTO_TEST_CASE( trans_throws_test )
{
	// Critical transformation checks
	// Scale can not be zero
	BOOST_CHECK_NO_THROW( node->setScale(vl::vector(1.0, 1.0, 1.0)) );
	BOOST_CHECK_THROW( node->setScale(vl::vector::ZERO), vl::zero_scale  );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node->getScale(), vl::vector(1.0,1.0,1.0)) );

	BOOST_CHECK_THROW( node->scale(0.0), vl::zero_scale );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node->getScale(), vl::vector(1.0,1.0,1.0)) );

	BOOST_CHECK_THROW( node->setScale(0.0), vl::zero_scale );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node->getScale(), vl::vector(1.0,1.0,1.0)) );

	// Can not rotate with quaternions including scale factor

	BOOST_CHECK_NO_THROW( node->setOrientation(vl::quaternion::IDENTITY) );

	vl::quaternion q = vl::quaternion::ZERO;
	BOOST_CHECK_THROW( node->rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node->getOrientation(), vl::quaternion::IDENTITY) );

	q = vl::quaternion(2, 0, 0, 0);
	BOOST_CHECK_THROW( node->rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node->getOrientation(), vl::quaternion::IDENTITY) );

	q = vl::quaternion(0, 0, 0, 2);
	BOOST_CHECK_THROW( node->rotate(q), vl::scale_quaternion );
	// We should be in consistent state after throw
	BOOST_CHECK( vl::equal(node->getOrientation(), vl::quaternion::IDENTITY) );
}

BOOST_AUTO_TEST_CASE( transformations_test )
{
	// Check that the node is correctly initialized
	BOOST_CHECK( vl::equal(node->getPosition(), vl::vector::ZERO) );
	BOOST_CHECK( vl::equal(node->getOrientation(), vl::quaternion::IDENTITY) );
	BOOST_CHECK( vl::equal(node->getScale(), vl::vector(1.0, 1.0, 1.0)) );

	// Position test
	vl::vector v = vl::vector(0.0, 0.0, 50.0);
	BOOST_CHECK_NO_THROW( node->translate(v) );
	// Math library limitations, it does not provide epsilon comparison
	// And BOOS_CHECK_CLOSE does not work on ZERO, because it uses procentual
	// difference not absolute...
	BOOST_CHECK( vl::equal( node->getPosition(), v ) );
	
	node->translate(2.0*v);
	BOOST_CHECK( vl::equal( node->getPosition(), 3.0*v ) );

	v = vl::vector(10, 20, -20);
	BOOST_CHECK_NO_THROW( node->setPosition(v) );
	BOOST_CHECK( vl::equal( node->getPosition(), v ) );

	// 90 degree rotation around x
	vl::scalar half_a = (M_PI/2)/2;
	vl::quaternion q( std::sin(half_a), 0, 0, std::cos(half_a) );
	BOOST_REQUIRE( vl::equal(q.abs(), 1.0) );
	// 45 degree rotation around y
	half_a = (M_PI/4)/2;
	vl::quaternion r(0, std::sin(half_a), 0, std::cos(half_a));
	BOOST_REQUIRE( vl::equal(r.abs(), 1.0) );

	BOOST_CHECK_NO_THROW( node->rotate( q ) );
	BOOST_CHECK( vl::equal( node->getOrientation(), q ) );

	BOOST_CHECK_NO_THROW( node->rotate( r ) );
	BOOST_CHECK( vl::equal( node->getOrientation(), q*r ) );

	BOOST_CHECK_NO_THROW( node->setOrientation( r ) );
	BOOST_CHECK( vl::equal( node->getOrientation(), r ) );

	BOOST_CHECK_NO_THROW( node->setOrientation( vl::quaternion::IDENTITY ) );
	BOOST_CHECK( vl::equal( node->getOrientation(), vl::quaternion::IDENTITY ) );

	// scale
	BOOST_CHECK_NO_THROW( node->setScale( 10 ) );
	BOOST_CHECK( vl::equal( node->getScale(), vl::vector(10.0, 10.0, 10.0) ) );

	BOOST_CHECK_NO_THROW( node->scale( 0.5 ) );
	BOOST_CHECK( vl::equal( node->getScale(), vl::vector(5.0, 5.0, 5.0) ) );

	v = vl::vector(1, 2, 0.5 );
	BOOST_CHECK_NO_THROW( node->scale( v ) );
	BOOST_CHECK( vl::equal( node->getScale(), 5.0*v ) );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( ChildSceneNode, ChildNodeFixture )

BOOST_AUTO_TEST_CASE( child_test )
{
	BOOST_REQUIRE( child1 && child2 && node );
	// Start conditions, no childs or parents
	BOOST_CHECK( !child1->getParent() );
	BOOST_CHECK( !child2->getParent() );
	BOOST_CHECK_EQUAL( node->numChildren(), 0 );

	// add first child, with setParent
	child1->setParent( node );
	BOOST_CHECK_EQUAL( child1->getParent(), node );
	BOOST_CHECK_EQUAL( node->numChildren(), 1 );
	if( node->numChildren() > 0 )
	{ BOOST_CHECK_EQUAL( node->getChilds().at(0), child1 ); }

	// add second child, with addChild
	node->addChild( child2 );
	BOOST_CHECK_EQUAL( child2->getParent(), node );
	BOOST_CHECK_EQUAL( node->numChildren(), 2 );

	// remove
	BOOST_CHECK_NO_THROW( node->removeChild( child1 ) );
	BOOST_REQUIRE_EQUAL( node->numChildren(), 1 );
	BOOST_CHECK( !child1->getParent() );
	node->addChild( child1 );
	BOOST_CHECK_EQUAL( node->numChildren(), 2 );

	// Removing by name
	BOOST_CHECK_EQUAL( node->removeChild( "Child1" ), child1 );
	BOOST_REQUIRE_EQUAL( node->numChildren(), 1 );
	BOOST_CHECK( !child1->getParent() );
	node->addChild( child1 );
	BOOST_CHECK_EQUAL( node->numChildren(), 2 );

	// Getting by name
	BOOST_CHECK_EQUAL( node->getChild( "Child2" ), child2 );
	BOOST_CHECK_EQUAL( node->getChild( "Child1" ), child1 );
	BOOST_CHECK_EQUAL( node->numChildren(), 2 );
	BOOST_CHECK_EQUAL( child2->getParent(), node );
	BOOST_CHECK_EQUAL( child1->getParent(), node );

	// Disallow adding node multiple times
	BOOST_CHECK_EQUAL( child2->getParent(), node );
	BOOST_CHECK_THROW( node->addChild( child2 ), vl::duplicate );
	BOOST_CHECK_EQUAL( child2->getParent(), node );
	BOOST_CHECK_NO_THROW( child2->addChild( child1 ) );
	BOOST_CHECK_EQUAL( child1->getParent(), child2 );
	BOOST_CHECK_NO_THROW( node->addChild( child1 ) );
	BOOST_CHECK_EQUAL( child1->getParent(), node );

	//	Setting parent same multiple times just returns
	BOOST_CHECK_EQUAL( child2->getParent(), node );
	BOOST_CHECK_NO_THROW( child2->setParent( node ) );
	BOOST_CHECK_EQUAL( child2->getParent(), node );

	// Disallow setting node as it's own parent
	BOOST_CHECK_EQUAL( child2->getParent(), node );
	BOOST_CHECK_THROW( child2->addChild( child2 ), vl::exception );
	BOOST_CHECK_EQUAL( child2->getParent(), node );

	// Allow setting null parent
	BOOST_CHECK_NE( child2->getParent(), SceneNodeRefPtr() );
	BOOST_CHECK_NO_THROW( child2->setParent( SceneNodeRefPtr() ) );
	BOOST_CHECK_EQUAL( child2->getParent(), SceneNodeRefPtr() );
}

BOOST_AUTO_TEST_CASE( node_creation_test )
{
	MOCK_EXPECT( man, getRootNode ).once().returns( node );
	MOCK_EXPECT( man, createNode ).exactly(2).with("Child1").returns( child1 );
	MOCK_EXPECT( man, createNode ).once().with("Child2").returns( child2 );

	BOOST_CHECK_EQUAL( man->getRootNode(), node );
	BOOST_CHECK_EQUAL( node->getName(), "Node" );

	// Creating childs, that is calling SceneManager
	BOOST_CHECK_NO_THROW( child1 = node->createChild("Child1") );
	BOOST_CHECK_NO_THROW( child2 = node->createChild("Child2") );
	BOOST_CHECK_THROW( node->createChild("Child1"), vl::duplicate );

}

BOOST_AUTO_TEST_CASE( attachment_test )
{
	BOOST_CHECK_EQUAL( node->numAttached(), 0 );
	BOOST_CHECK_NO_THROW( node->attachObject(obj1) );
	BOOST_CHECK_EQUAL( node->numAttached(), 1 );
	if( node->numAttached() > 0 )
	{ BOOST_CHECK_EQUAL( node->getAttached().at(0), obj1 ); }
	BOOST_CHECK_THROW( node->attachObject(obj1), vl::duplicate );
	BOOST_CHECK_EQUAL( node->numAttached(), 1 );

	BOOST_CHECK_NO_THROW( node->attachObject(obj2) );
	BOOST_CHECK_EQUAL( node->numAttached(), 2 );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( FindSceneNode, FindNodeFixture )

BOOST_AUTO_TEST_CASE( finding )
{
}

BOOST_AUTO_TEST_SUITE_END()
