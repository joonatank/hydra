/*	Joonatan Kuosa
 *	2010-05
 *
 *	Test eq_cluster/channel.cpp
 *	These are as unit tests as possible, they require full equalizer without
 *	any windows to be initialized for correct usage.
 *
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE test_channel

#include <eq/eq.h>

#include <boost/test/unit_test.hpp>
#include <turtle/mock.hpp>

#include "eq_cluster/window.hpp"

// Tested class
#include "eq_cluster/channel.hpp"

// Helpers
#include "eqogre_fixture.hpp"

class NodeFactory : public eq::NodeFactory
{
public :
	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new eqOgre::Channel( parent ); }
};

::NodeFactory factory;

BOOST_FIXTURE_TEST_SUITE( EqualizerTest, EqOgreFixture )

BOOST_AUTO_TEST_CASE( constructor )
{
	fs::path cmd( test::master_test_suite().argv[0] );

	vl::Args args;
	std::string conf_name("1-window.eqc");

	initArgs(&args, cmd, conf_name);

	BOOST_TEST_MESSAGE( "args = " << args );

	BOOST_CHECK_NO_THROW( init( args, &factory ) );
}

BOOST_AUTO_TEST_SUITE_END()