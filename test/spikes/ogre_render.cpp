#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ogre_render

#include <boost/test/unit_test.hpp>


#include "base/sleep.hpp"
#include "base/exceptions.hpp"

// Test helpers
#include "../fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_FIXTURE_TEST_CASE( render_test, OgreFixture )
{
	// Lets find in where the config is
	fs::path conf = find_conf_path();
	BOOST_REQUIRE( fs::exists( conf ) );
	init( conf );

	BOOST_CHECK_EQUAL( _robot->numAttachedObjects(), 1 );
	BOOST_CHECK_EQUAL( _feet->numAttachedObjects(), 1 );
	BOOST_CHECK( _feet->isInSceneGraph() );
	BOOST_CHECK( _robot->isInSceneGraph() );
	BOOST_CHECK( _entity->isInScene() );

	for( size_t i = 0; i < 4000; i++ )
	{ 
		mainloop();
		vl::msleep(1);
	}
}
