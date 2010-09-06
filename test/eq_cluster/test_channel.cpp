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
#include "../debug.hpp"

class NodeFactory : public eq::NodeFactory
{
public :
	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new eqOgre::Channel( parent ); }

	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }
};

::NodeFactory factory;

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_FIXTURE_TEST_SUITE( EqualizerTest, EqOgreFixture )

BOOST_AUTO_TEST_CASE( constructor )
{
	vl::SettingsRefPtr settings = getSettings( test::master_test_suite().argv[0] );
	BOOST_REQUIRE( settings );

	BOOST_TEST_MESSAGE( "args = " << settings->getEqArgs() );

	BOOST_CHECK_NO_THROW( init( settings, &factory ) );

	eq::Channel *eq_channel = config->getNodes().at(0)->getPipes().at(0)->getWindows().at(0)->getChannels().at(0);
	BOOST_REQUIRE( eq_channel );
	eqOgre::Channel *channel = dynamic_cast<eqOgre::Channel *>(eq_channel);
	BOOST_CHECK( channel );
}

BOOST_AUTO_TEST_SUITE_END()