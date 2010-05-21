/*	Joonatan Kuosa
 *	2010-05
 *
 *	Test eq_cluster/observer.cpp
 *	These are as unit tests as possible, they require full equalizer without
 *	any windows to be initialized for correct usage.
 *
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE test_window

#include <eq/eq.h>

#include <boost/test/unit_test.hpp>

// Tested class
#include "eq_cluster/window.hpp"

// Helpers
#include "eqogre_fixture.hpp"

class NodeFactory : public eq::NodeFactory
{
public :
	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }
};

eq::NodeFactory *g_nodeFactory = new ::NodeFactory;

BOOST_FIXTURE_TEST_SUITE( EqualizerTest, EqOgreFixture )

BOOST_AUTO_TEST_CASE( constructor )
{
	fs::path cmd( test::master_test_suite().argv[0] );

	vl::Args args;
	std::string conf_name("1-window.eqc");

	initArgs(&args, cmd, conf_name);

	BOOST_TEST_MESSAGE( "args = " << args );

	BOOST_CHECK_NO_THROW( init( args ) );

	eq::Window *eq_win = config->getNodes().at(0)->getPipes().at(0)->getWindows().at(0);
	eqOgre::Window *win = dynamic_cast<eqOgre::Window *>(eq_win);
	BOOST_CHECK( win );
}

BOOST_AUTO_TEST_SUITE_END()