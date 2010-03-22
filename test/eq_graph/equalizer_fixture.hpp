#ifndef EQUALIZER_FIXTURE_HPP
#define EQUALIZER_FIXTURE_HPP

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

class Node : public eq::Node
{
public :
	Node( eq::Config *parent )
		: eq::Node(parent), state(0), man(0), node(0), _id(0)
	{
		BOOST_TEST_MESSAGE( "Node::Node" );
	}

	virtual ~Node( void )
	{
		delete node;
		delete man;
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( eq::Node::configInit( initID ) );

		_id = initID;
		BOOST_REQUIRE( initID != EQ_ID_INVALID );

		BOOST_REQUIRE( man = new mock_scene_manager() );
		node = new vl::cl::SceneNode(man);
		return true;
	}

	virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{
		eq::Node::frameStart( frameID, frameNumber );
		BOOST_TEST_MESSAGE( "Node::frameStart" );

		// Sync the node if the initial mapping is done
		if( state > 0 )
		{ node->sync(); }

		switch( state )
		{
			case 0 :
			{
				BOOST_REQUIRE( getSession() );
				// We can not register the object in configInit for some reason
				// equalizer syncs the object and crashes...
				// All registrations have to be done after the first frameStart
				BOOST_REQUIRE( getSession()->mapObject( node, _id ) );
				BOOST_TEST_MESSAGE( "SceneNode mapped" );
				++state;
			}
			break;

			case 1 :
			{
				node->sync();
				BOOST_CHECK( vl::equal( node->getPosition(), TRANS_VEC[0] ) );
				++state;

			}
			break;

			case 2 :
			{
				node->sync();
				BOOST_CHECK( vl::equal( node->getOrientation(), ROT_QUAT[0] ) );
				++state;
			}
			break;
			case 3 :
			{
				BOOST_CHECK( vl::equal( node->getScale(), SCALE_VEC[0] ) );
				BOOST_CHECK( vl::equal( node->getOrientation(), ROT_QUAT[1] ) );
				BOOST_CHECK( vl::equal( node->getPosition(), TRANS_VEC[1] ) );

				++state;
			}
			break;

			case 4 :
			{
				// TODO test attachement

				++state;
			}
			break;

			case 5 :
			{ 
				// TODO test children
				++state;
			}
			break;

			default :
			break;
		}

	}

	vl::graph::SceneManager *man;
	vl::cl::SceneNode *node;
	int state;
	uint32_t _id;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Node *createNode( eq::Config *parent )
	{ return new ::Node( parent ); }
};

const int argc = 2;
char NAME[] = "TEST\0";
char *argv[argc] = { NAME, "\0" };

struct EqFixture
{
	// Init code for this test
	EqFixture( void )
		: error( false ), frameNumber(0), config(0), man(0), node(0),
		  log_file( "equalize.log" )
	{
		// Redirect logging
		eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, &nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		man = new mock_scene_manager;
		node = new vl::cl::SceneNode( man );
		BOOST_REQUIRE( config->registerObject( node ) );
		BOOST_REQUIRE( node->getID() != EQ_ID_INVALID );

		// 3. init config
		BOOST_REQUIRE( config->init( node->getID() ));

	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );
	
		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~EqFixture( void )
	{
		// All registered objects need to be deregistered before exiting the
		// config, otherwise equalizer crashes...
		config->deregisterObject( node );

		// 5. exit config
		if( config )
		{ BOOST_CHECK( config->exit() ); }

		// 6. release config
		eq::releaseConfig( config );

		delete man;
		delete node;

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	bool error;
	uint32_t frameNumber;
	eq::Config *config;
	vl::graph::SceneManager *man;
	vl::cl::SceneNode *node;
	NodeFactory nodeFactory;
	std::ofstream log_file;
};

#endif
