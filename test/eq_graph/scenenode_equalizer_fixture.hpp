#ifndef SCENENODE_EQUALIZER_FIXTURE_HPP
#define SCENENODE_EQUALIZER_FIXTURE_HPP

#include <eq/eq.h>

#include <fstream>

#include "eq_graph/eq_scene_node.hpp"

#include "mock_scene_manager.hpp"

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

struct SyncFixture
{
	SyncFixture( eq::Config *conf, int n_tests )
		: config( conf ), N_TESTS(n_tests), state(0)
	{}

	virtual ~SyncFixture( void )
	{}

	virtual uint32_t init( uint32_t id = EQ_ID_INVALID ) = 0;

	virtual void update( void ) = 0;

	virtual void test( int state ) = 0;

	bool testRemaining( void )
	{
		return state < N_TESTS-1;
	}

	int numTests( void )
	{
		return N_TESTS;
	}

	eq::Config *config;
	int N_TESTS;
	int state;
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
		{ config->deregisterObject( node ); }

		delete node;
		delete man;
	}

	uint32_t init( uint32_t id = EQ_ID_INVALID )
	{
		BOOST_REQUIRE( config );

		if( id == EQ_ID_INVALID )
		{
			// Register new object
			BOOST_REQUIRE( config->registerObject( node ) );
			BOOST_REQUIRE( node->getID() != EQ_ID_INVALID );
			return node->getID();
		}
		else
		{
			// Map an object
			// We can not register the object in configInit for some reason
			// equalizer syncs the object and crashes...
			// All registrations have to be done after the first frameStart
			BOOST_REQUIRE( config->mapObject( node, id ) );
			return id;
		}
	}

	void update( void )
	{
		if( node->isMaster() )
		{
			test(state);
			node->commit();
		}
		else
		{
			node->sync();
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
				}
				else 
				{
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
				}
				else
				{
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
				}
				else
				{
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
						.returns( new vl::cl::SceneNode(man, "child1") );
					MOCK_EXPECT( man, createNode ).once().with("child2")
						.returns( new vl::cl::SceneNode(man, "child2") );

					// Creating childs, that is calling SceneManager
					//vl::graph::SceneNode *child = 0;
					//vl::graph::SceneNode *child2 = 0;
					node->createChild( "child1" );
					node->createChild( "child2" );
				}
				else
				{
					// Create childs with incorrect names
					// We should have the node calling these I think.
					MOCK_EXPECT( man, createNode ).once().with("c1")
						.returns( new vl::cl::SceneNode(man, "c1") );
					MOCK_EXPECT( man, createNode ).once().with("c2")
						.returns( new vl::cl::SceneNode(man, "c2") );

					node->createChild( "c1" );
					node->createChild( "c2" );

					BOOST_CHECK( vl::equal( node->numChildren(), 2 ) );
					BOOST_CHECK( node->getChild( "child1" ) );
					BOOST_CHECK( node->getChild( "child2" ) );
				}
			}
			break;

			// child removal
			case 4 :
			{
				if( node->isMaster() )
				{
					delete node->removeChild( "child2" );
				}
				else
				{
					BOOST_CHECK( vl::equal( node->numChildren(), 1 ) );
					BOOST_CHECK( node->getChild( "child1" ) );
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

	mock_scene_manager *man;
	vl::cl::SceneNode *node;
};

class Node : public eq::Node
{
public :
	Node( eq::Config *parent )
		: eq::Node(parent), state(0), sync_fixture(0)
	{
		BOOST_TEST_MESSAGE( "Node::Node" );
	}

	virtual ~Node( void )
	{
		delete sync_fixture;
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( initID != EQ_ID_INVALID );
		BOOST_REQUIRE( eq::Node::configInit( initID ) );

		sync_fixture = new SceneNodeSyncFixture( getConfig() );

		sync_fixture->init( initID );

		return true;
	}

	virtual void frameStart( const uint32_t frameID,
			const uint32_t frameNumber )
	{
		eq::Node::frameStart( frameID, frameNumber );
		BOOST_TEST_MESSAGE( "Node::frameStart" );

		sync_fixture->update();
	}

	int state;
	SyncFixture *sync_fixture;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Node *createNode( eq::Config *parent )
	{ return new ::Node( parent ); }
};

const int argc = 2;
char NAME[] = "TEST\0";
char END[] = "\0";
char *argv[argc] = { NAME, END };

struct EqFixture
{
	// Init code for this test
	EqFixture( void )
		: error( false ), frameNumber(0), config(0), 
		  sync_fixture(0), log_file( "equalize.log" )
	{
		// Redirect logging
		eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, &nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		sync_fixture = new SceneNodeSyncFixture( config );

		uint32_t id = sync_fixture->init();
		BOOST_REQUIRE( id != EQ_ID_INVALID );

		// 3. init config
		BOOST_REQUIRE( config->init( id ) );
	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );
	
		sync_fixture->update();

		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~EqFixture( void )
	{
		// All registered objects need to be deregistered before exiting the
		// config, otherwise equalizer crashes...

		delete sync_fixture;

		// 5. exit config
		if( config )
		{ BOOST_CHECK( config->exit() ); }

		// 6. release config
		eq::releaseConfig( config );

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	bool error;
	uint32_t frameNumber;
	eq::Config *config;
	//mock_scene_manager *man;
	//vl::cl::SceneNode *node;
	SyncFixture *sync_fixture;
	NodeFactory nodeFactory;
	std::ofstream log_file;
};

#endif
