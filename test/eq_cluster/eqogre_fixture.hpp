#ifndef VL_TEST_EQOGRE_FIXTURE_HPP
#define VL_TEST_EQOGRE_FIXTURE_HPP

#include <eq/eq.h>

#include <boost/test/unit_test.hpp>

#include "base/exceptions.hpp"
#include "base/args.hpp"
#include "base/filesystem.hpp"

#include "eq_cluster/config.hpp"

// Test includes
#include "../fixtures.hpp"

namespace test = boost::unit_test::framework;

void initArgs(vl::Args *args, fs::path const &exe_path, std::string const &conf_name )
{
	// 0-window.eqc should be in the same directory as the exe
	fs::path conf_dir = exe_path.parent_path();
	fs::path conf = conf_dir / "1-window.eqc";
	fs::exists( conf );

	args->add(exe_path.file_string().c_str());
	args->add("--eq-config");
	args->add(conf.file_string().c_str());
}

struct EqOgreFixture
{
	// Init code for this test
	EqOgreFixture( void )
		: error( false ), frameNumber(0), config(0), log_file( "render_test.log" )
	{
		BOOST_TEST_MESSAGE( "EqOgreFixture::EqOgreFixture" );
	}

	void init( vl::Args &args, eq::NodeFactory *fact )
	{
		// Redirect logging
		eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( args.size(), args.getData(), fact ) );

		// 2. get a configuration
		config = eq::getConfig( args.size(), args.getData() );
		BOOST_REQUIRE( config );

		// 3. init config
		BOOST_REQUIRE( config->init(0) );
	}
	
	void init( vl::SettingsRefPtr settings, eq::NodeFactory *fact )
	{
		// Redirect logging
		//eq::base::Log::setOutput( log_file );
		vl::Args &args = settings->getEqArgs();

		// 1. Equalizer initialization
		BOOST_REQUIRE(  eq::init( args.size(), args.getData(), fact ) );

		// 2. get a configuration
		config = eq::getConfig( args.size(), args.getData() );
		BOOST_REQUIRE( config );

		eqOgre::Config *vl_config = dynamic_cast<eqOgre::Config *>(config);
		BOOST_REQUIRE( vl_config );
		vl_config->setSettings( settings );

		// 3. init config
		BOOST_REQUIRE( config->init(0) );
	}
	// Controlled mainloop function so the test can run the loop
	void mainloop( void )
	{
		BOOST_REQUIRE( config->isRunning() );

		config->startFrame( ++frameNumber );
		config->finishFrame();
	}

	// Clean up code for this test
	~EqOgreFixture( void )
	{
		// 5. exit config
		if( config )
		{ BOOST_CHECK( config->exit() ); }

		// 6. release config
		eq::releaseConfig( config );

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	std::ofstream log_file;
	bool error;
	uint32_t frameNumber;
	eq::Config *config;
	//std::ofstream log_file;
};

#endif