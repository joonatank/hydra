#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE sync_eq_scene_node
#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_scene_node.hpp"

#include "scenenode_equalizer_fixture.hpp"
#include "equalizer_fixture.hpp"

// TODO this should be moved to different directory where we have
// all the synchronization tests.
BOOST_FIXTURE_TEST_CASE( equalizer_test, EqFixture )
{
	BOOST_CHECK( config );

	// Test transmitting translation

	while( sync_fixture->testRemaining() )
	{
		mainloop();
	}
}
