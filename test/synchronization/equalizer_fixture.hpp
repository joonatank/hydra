#ifndef EQUALIZER_FIXTURE_HPP
#define EQUALIZER_FIXTURE_HPP

#include <eq/eq.h>

#include <fstream>

#include "sync_fixture.hpp"

const int argc = 2;
char NAME[] = "TEST\0";
char END[] = "\0";
char *argv[argc] = { NAME, END };

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
	SyncFixture *sync_fixture;
	NodeFactory nodeFactory;
	std::ofstream log_file;
};

#endif

