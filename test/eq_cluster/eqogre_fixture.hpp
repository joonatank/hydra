#ifndef VL_TEST_EQOGRE_FIXTURE_HPP
#define VL_TEST_EQOGRE_FIXTURE_HPP

#include <eq/eq.h>

#include <boost/test/unit_test.hpp>

#include "base/exceptions.hpp"
#include "base/args.hpp"
#include "base/filesystem.hpp"

#include "eq_cluster/config.hpp"

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
		: frameNumber(0), config(0), log_file( "render_test.log" )
	{
		BOOST_TEST_MESSAGE( "EqOgreFixture::EqOgreFixture" );
	}

	bool init( vl::Args &args, eq::NodeFactory *fact )
	{
		BOOST_REQUIRE( _global_init( args, fact ) );

		// 3. init config
		return( config->init(0) );
	}
	
	bool init( vl::SettingsRefPtr settings, eq::NodeFactory *fact )
	{
		vl::Args &args = settings->getEqArgs();

		BOOST_REQUIRE( _global_init( args, fact ) );

		eqOgre::Config *vl_config = dynamic_cast<eqOgre::Config *>(config);
		BOOST_REQUIRE( vl_config );
		vl_config->setSettings( settings );

		// 3. init config
		return config->init(0);
	}

	bool _global_init( vl::Args &args, eq::NodeFactory *fact )
	{
		// Redirect logging
		eq::base::Log::setOutput( log_file );

		// 1. Equalizer initialization
		if( !eq::init( args.size(), args.getData(), fact ) )
		{ return false; }

		// 2. get a configuration
		config = eq::getConfig( args.size(), args.getData() );
		if( !config )
		{ return false; }

		return true;
	}

	// Controlled mainloop function so the test can run the loop
	bool mainloop( void )
	{
		if( config->isRunning() )
		{
			config->startFrame( ++frameNumber );
			config->finishFrame();
			return true;
		}
		else
		{ return false; }
	}

	// Clean up code for this test
	~EqOgreFixture( void )
	{
		// 5. exit config
		if( config )
		{ 
			BOOST_CHECK( config->exit() );
			// 6. release config
			eq::releaseConfig( config );
		}

		// 7. exit
		BOOST_CHECK( eq::exit() );
	}

	bool _inited;
	uint32_t frameNumber;
	eq::Config *config;
	std::ofstream log_file;
};

#endif
