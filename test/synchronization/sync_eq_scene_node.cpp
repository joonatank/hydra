#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE sync_eq_scene_node

#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_scene_node.hpp"

// Test helpers
#include "sync_fixture.hpp"
#include "../eq_graph/mocks.hpp"
#include "equalizer_fixture.hpp"
#include "threaded_test_runner.hpp"

namespace vl
{
	using namespace graph;
}


// Test variables we transmit over the synchronization
vl::vector const TRANS_VEC[3] =
{
	vl::vector(1, 2, 3),
	vl::vector(0, 10, 50),
	vl::vector(-50, -100, 5000)
};

vl::scalar const HALF_A[3] =
{
	(M_PI/4)/2,
	(M_PI/2)/2,
	(M_PI/4)/2
};

vl::quaternion const ROT_QUAT[3] = 
{
	vl::quaternion(0, std::sin(HALF_A[0]), 0, std::cos(HALF_A[0])),
	vl::quaternion(std::sin(HALF_A[1]), 0, 0, std::cos(HALF_A[1])),
	vl::quaternion(std::sin(HALF_A[2]), 0, 0, std::cos(HALF_A[2]))
};

vl::vector const SCALE_VEC[3] =
{
	vl::vector(2, 2, 2),
	vl::vector(0.5, 2, 5),
	vl::vector(1, 1, 1)
};

struct SceneNodeTest : public vl::Test
{
	SceneNodeTest( bool mast, vl::cl::SceneNodeRefPtr n, mock::SceneManagerPtr m )
		: Test(mast), node(n), man(m)
	{
	}

	virtual bool test( void )
	{
		if( master )
		{
			// Do something
			bool ret = modify();
			node->commit();

			return ret;
		}
		else
		{
			node->sync();
			// Check
			return verify();
		}
	}

	// Verifies that the object in the slave is in correct state
	virtual bool verify( void ) = 0;

	// Modifies the object on the master
	virtual bool modify( void ) = 0;

	vl::cl::SceneNodeRefPtr node;
	mock::SceneManagerPtr man;
};

struct TranslateTest : public SceneNodeTest
{
	TranslateTest( bool mast, vl::cl::SceneNodeRefPtr n, mock::SceneManagerPtr m )
		: SceneNodeTest( mast, n, m )
	{}

	virtual bool verify( void )
	{
		BOOST_MESSAGE( "Check translation synchronization" );
		BOOST_CHECK( vl::equal( node->getPosition(), TRANS_VEC[0] ) );
		return true;
	}

	virtual bool modify( void )
	{
		node->setPosition( TRANS_VEC[0] );
		return true;
	}

};

struct RotateTest : public SceneNodeTest
{
	RotateTest( bool mast, vl::cl::SceneNodeRefPtr n, mock::SceneManagerPtr m )
		: SceneNodeTest( mast, n, m )
	{}

	virtual bool verify( void )
	{
		BOOST_MESSAGE( "Check rotation synchronization" );
		BOOST_CHECK( vl::equal( node->getOrientation(), ROT_QUAT[0] ) );
		return true;
	}

	virtual bool modify( void )
	{
		node->setOrientation( ROT_QUAT[0] );
		return true;
	}

};

struct TransformTest : public SceneNodeTest
{
	TransformTest( bool mast, vl::cl::SceneNodeRefPtr n, mock::SceneManagerPtr m )
		: SceneNodeTest( mast, n, m )
	{
	}

	virtual bool verify( void )
	{
		BOOST_MESSAGE( "Check tranfromation synchronization" );
		BOOST_CHECK( vl::equal( node->getScale(), SCALE_VEC[0] ) );
		BOOST_CHECK( vl::equal( node->getOrientation(), ROT_QUAT[1] ) );
		BOOST_CHECK( vl::equal( node->getPosition(), TRANS_VEC[1] ) );
		return true;
	}

	virtual bool modify( void )
	{
		node->setScale( SCALE_VEC[0] );
		node->setOrientation( ROT_QUAT[1] );
		node->setPosition( TRANS_VEC[1] );
		return true;
	}
};

// TODO write and add
struct ChildTest : public SceneNodeTest
{
	ChildTest( bool mast, vl::cl::SceneNodeRefPtr n, mock::SceneManagerPtr m )
		: SceneNodeTest( mast, n, m )
	{}

	virtual bool verify( void )
	{
		BOOST_MESSAGE( "Check child synchronization" );
		return true;
	}

	virtual bool modify( void )
	{
		MOCK_EXPECT( man, createNode ).once().with("child1")
			.returns( boost::shared_ptr<vl::cl::SceneNode>( new vl::cl::SceneNode(man, "child1") ) );
		MOCK_EXPECT( man, createNode ).once().with("child2")
			.returns( boost::shared_ptr<vl::cl::SceneNode>( new vl::cl::SceneNode(man, "child2") ) );

		// Creating childs, that is calling SceneManager
		//vl::graph::SceneNode *child = 0;
		//vl::graph::SceneNode *child2 = 0;
		node->createChild( "child1" );
		node->createChild( "child2" );
		return true;
	}
};

// TODO write and add
struct AttachTest : public SceneNodeTest
{
	AttachTest( bool mast, vl::cl::SceneNodeRefPtr n, mock::SceneManagerPtr m )
		: SceneNodeTest( mast, n, m )
	{}

	virtual bool verify( void )
	{
		BOOST_MESSAGE( "Check attached object synchronization" );
		return true;
	}

	virtual bool modify( void )
	{
		return true;
	}
};

// TODO most of this can be moved to be equalizer specific but object independent
// using eq::Object
struct SceneNodeSyncFixture : public vl::EqSyncFixture
{
	SceneNodeSyncFixture( bool m)
		: EqSyncFixture( m, 0 ),
		  man( new mock::SceneManager ),
		  node( new vl::cl::SceneNode( man, "node") )
	{
		object = node.get();
		runner.addTest( new TranslateTest( master, node, man ) );
		runner.addTest( new RotateTest( master, node, man ) );
		runner.addTest( new TransformTest( master, node, man ) );
		runner.addTest( new ChildTest( master, node, man) );
		runner.addTest( new AttachTest( master, node, man) );
	}

	~SceneNodeSyncFixture( void ) {}

	mock::SceneManagerPtr man;
	vl::cl::SceneNodeRefPtr node;
};

vl::SyncFixture *::EqFixture::sync_fixture = new SceneNodeSyncFixture(true);
vl::SyncFixture *::Node::sync_fixture = new SceneNodeSyncFixture(false);

BOOST_FIXTURE_TEST_CASE( sync_test, EqFixture )
{
	while( sync_fixture->remaining() )
	{
		mainloop();
	}
}

