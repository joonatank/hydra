#ifndef EQ_TEST_FIXTURE_HPP
#define EQ_TEST_FIXTURE_HPP

// Equalizer
#include <eq/eq.h>
// Log file
#include <fstream>
// require and check macros
#include <boost/test/unit_test.hpp>

const int argc = 4;
extern char *argv[argc];

extern eq::NodeFactory *g_nodeFactory;

struct EqFixture 
{
	// Init code for this test
	EqFixture( void )
		: error( false ), frameNumber(0), config(0),
		  log_file( "render_test.log" )
	{
		// Redirect logging
		//eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( argc, argv, g_nodeFactory ) );
		
		// 2. get a configuration
		config = eq::getConfig( argc, argv );
		BOOST_REQUIRE( config );

		// 3. init config
		BOOST_REQUIRE( config->init(0));
	}

	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );
	
		BOOST_CHECK_NO_THROW( config->startFrame( ++frameNumber ) );
		BOOST_CHECK_NO_THROW( config->finishFrame() );
	}

	// Clean up code for this test
	~EqFixture( void )
	{
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
	//NodeFactory nodeFactory;
	std::ofstream log_file;
};

#endif
