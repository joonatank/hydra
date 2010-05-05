#ifndef EQUALIZER_FIXTURE_NEW_HPP
#define EQUALIZER_FIXTURE_NEW_HPP

#include <eq/eq.h>

#include <fstream>

#include "eq_graph/eq_scene_node.hpp"

// Test helpers
#include "../eq_graph/mocks.hpp"
#include "threaded_test_runner.hpp"
#include "sync_fixture.hpp"

namespace vl
{
	using namespace graph;
}

const int argc = 2;
char NAME[] = "TEST\0";
char END[] = "\0";
char *argv[argc] = { NAME, END };

class Node : public eq::Node
{
public :
	Node( eq::Config *parent )
		: eq::Node(parent), state(0)
	{
		BOOST_TEST_MESSAGE( "Node::Node" );

		sync_fixture->init( parent );
	}

	virtual ~Node( void )
	{
	}

	virtual bool configInit( const uint32_t initID )
	{
		BOOST_REQUIRE( initID != EQ_ID_INVALID );
		BOOST_REQUIRE( eq::Node::configInit( initID ) );

		BOOST_REQUIRE( sync_fixture );
		sync_fixture->reg( initID );

		return true;
	}

	virtual void frameStart( const uint32_t frameID,
			const uint32_t frameNumber )
	{
		eq::Node::frameStart( frameID, frameNumber );
		BOOST_TEST_MESSAGE( "Node::frameStart" );

		sync_fixture->test();
	}

	int state;
	static vl::SyncFixture *sync_fixture;
};

class NodeFactory : public eq::NodeFactory
{
public :
	virtual Node *createNode( eq::Config *parent )
	{ return new ::Node( parent ); }
};

struct EqFixture
{
	// Init code for this test
	EqFixture( void )
		: error( false ), frameNumber(0), config(0), log_file( "equalize.log" )
	{
		// Redirect logging
		eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, &nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		sync_fixture->init( config );

		uint32_t id = sync_fixture->reg( EQ_ID_INVALID );
		BOOST_REQUIRE( id != EQ_ID_INVALID );

		// 3. init config
		BOOST_REQUIRE( config->init( id ) );
	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );
	
		BOOST_REQUIRE( sync_fixture );
		sync_fixture->test();

		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~EqFixture( void )
	{
		// All registered objects need to be deregistered before exiting the
		// config, otherwise equalizer crashes...

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
	static vl::SyncFixture *sync_fixture;

	NodeFactory nodeFactory;
	std::ofstream log_file;
};

#endif

