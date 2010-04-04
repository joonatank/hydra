#ifndef SCENENODE_EQUALIZER_FIXTURE_HPP
#define SCENENODE_EQUALIZER_FIXTURE_HPP
#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_scene_node.hpp"

#include "../eq_graph/mock_scene_manager.hpp"

#include "sync_fixture.hpp"
//#include "equalizer_fixture.hpp"

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

struct SceneNodeSyncFixture : public SyncFixture
{
	SceneNodeSyncFixture( eq::Config *conf )
		: SyncFixture( conf, 6),
		  man( new mock_scene_manager ),
		  node( new vl::cl::SceneNode(man) )
	{}

	virtual ~SceneNodeSyncFixture( void )
	{
		if( node->isMaster() )
		{ config->deregisterObject( node.get() ); }
	}

	uint32_t init( uint32_t id = EQ_ID_INVALID )
	{
		BOOST_REQUIRE( config );

		if( id == EQ_ID_INVALID )
		{
			// Register new object
			BOOST_REQUIRE( config->registerObject( node.get() ) );
			BOOST_REQUIRE( node->getID() != EQ_ID_INVALID );
			return node->getID();
		}
		else
		{
			// Map an object
			// We can not register the object in configInit for some reason
			// equalizer syncs the object and crashes...
			// All registrations have to be done after the first frameStart
			BOOST_REQUIRE( config->mapObject( node.get(), id ) );
			return id;
		}
	}

	void update( void )
	{
		if( node->isMaster() )
		{
			test(state);
			//node->commit();
		}
		else
		{
			//node->sync();
			test(state);
		}

		++state;
	}

	void test( int state )
	{
		switch( state )
		{
			// Test position
			case 0 :
			{
				if( node->isMaster() )
				{
					node->setPosition( TRANS_VEC[0] );
					node->commit();
				}
				else 
				{
					node->sync();
					BOOST_CHECK( vl::equal(
								node->getPosition(), TRANS_VEC[0] ) );
				}
			}
			break;

			// Test orientation
			case 1 :
			{
				if( node->isMaster() )
				{
					node->setOrientation( ROT_QUAT[0] );
					node->commit();
				}
				else
				{
					node->sync();
					BOOST_CHECK( vl::equal(
								node->getOrientation(), ROT_QUAT[0] ) );
				}
			}
			break;

			case 2 :
			{
				if( node->isMaster() )
				{
					node->setScale( SCALE_VEC[0] );
					node->setOrientation( ROT_QUAT[1] );
					node->setPosition( TRANS_VEC[1] );
					node->commit();
				}
				else
				{
					node->sync();
					BOOST_CHECK( vl::equal(
								node->getScale(), SCALE_VEC[0] ) );
					BOOST_CHECK( vl::equal(
								node->getOrientation(), ROT_QUAT[1] ) );
					BOOST_CHECK( vl::equal(
								node->getPosition(), TRANS_VEC[1] ) );
				}
			}
			break;

			// child attachement
			case 3 :
			{
				if( node->isMaster() )
				{
					MOCK_EXPECT( man, createNode ).once().with("child1")
						.returns( boost::shared_ptr<vl::cl::SceneNode>(
									new vl::cl::SceneNode(man, "child1") ) );
					MOCK_EXPECT( man, createNode ).once().with("child2")
						.returns( boost::shared_ptr<vl::cl::SceneNode>(
									new vl::cl::SceneNode(man, "child2") ) );

					// Creating childs, that is calling SceneManager
					//vl::graph::SceneNode *child = 0;
					//vl::graph::SceneNode *child2 = 0;
					node->createChild( "child1" );
					node->createChild( "child2" );
					node->commit();
				}
				else
				{
					// Create childs with incorrect names
					// We should have the node calling these I think.
					MOCK_EXPECT( man, pushChildAddedStack ).exactly(2);
					node->sync();

					/*
					BOOST_CHECK( vl::equal( node->numChildren(), 2 ) );
					BOOST_CHECK( node->getChild( "child1" ) );
					BOOST_CHECK( node->getChild( "child2" ) );
					*/
				}
			}
			break;

			// child removal
			case 4 :
			{
				if( node->isMaster() )
				{
					node->removeChild( "child2" );
					node->commit();
				}
				else
				{
					MOCK_EXPECT( man, pushChildRemovedStack ).exactly(1);
					node->sync();
					/*
					BOOST_CHECK( vl::equal( node->numChildren(), 1 ) );
					BOOST_CHECK( node->getChild( "child1" ) );
					*/
				}
			}
			break;

			// TODO test attachement
			case 5 :
			{ 
				if( node->isMaster() )
				{

				}
				else
				{
				}
			}
			break;

			default :
			break;
		}
	}

	boost::shared_ptr<mock_scene_manager> man;
	boost::shared_ptr<vl::cl::SceneNode> node;
};

#endif
