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

// Required project classes
#include "eq_cluster/config.hpp"

// Helpers
#include "../debug.hpp"
#include "../test_helpers.hpp"
#include "eqogre_fixture.hpp"

class NodeFactory : public eq::NodeFactory
{
public :
	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }

	// We require eqOgre::Config to have set/get Settings functions
	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }
};

::NodeFactory factory;

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_FIXTURE_TEST_SUITE( EqualizerTest, EqOgreFixture )


// This should throw as we don't have settings we don't have Ogre plugins or resources either
BOOST_AUTO_TEST_CASE( initialization_args )
{
	fs::path cmd( test::master_test_suite().argv[0] );

	vl::Args args;
	std::string conf_name("1-window.eqc");

	initArgs(&args, cmd, conf_name);

	BOOST_TEST_MESSAGE( "args = " << args );

	// Equalizer will not propagate the error message to config->init()
	// But the window is not created if Settings is missing
	BOOST_CHECK( init( args, &factory ) );

	BOOST_REQUIRE( config->getNodes().size() > 0 );
	eq::Node *node = config->getNodes().at(0);
	BOOST_REQUIRE( node->getPipes().size() > 0 );
	eq::Pipe *pipe = node->getPipes().at(0);
	BOOST_CHECK_EQUAL( pipe->getWindows().size(), 0u );
}

// This should work nicely and we should get correct pointers
// Even though there is no scene
// TODO add configuration copying
BOOST_AUTO_TEST_CASE( initialization_settings )
{
//	std::string conf_name("1-window.eqc");

	vl::SettingsRefPtr settings = getSettings( test::master_test_suite().argv[0], "test_window" );
	BOOST_REQUIRE( settings );

	BOOST_TEST_MESSAGE( "args = " << settings->getEqArgs() );

	BOOST_CHECK_NO_THROW( init( settings, &factory ) );

	eq::Window *eq_win = config->getNodes().at(0)->getPipes().at(0)->getWindows().at(0);
	eqOgre::Window *win = dynamic_cast<eqOgre::Window *>(eq_win);
	BOOST_CHECK( win );

	// Check that tracker is avail
	BOOST_CHECK( win->getTracker() );

	// Check that camera is avail
	BOOST_CHECK( win->getCamera() );

	// Check that Ogre::RenderWindow is avail
	BOOST_CHECK( win->getRenderWindow() );
}

// TODO test scene loading
/*
BOOST_AUTO_TEST_CASE( scene_loading )
{

}
*/

/*
BOOST_AUTO_TEST_CASE( rendering )
{
	fs::path cmd( test::master_test_suite().argv[0] );

	vl::Args args;
	std::string conf_name("1-window.eqc");

	initArgs(&args, cmd, conf_name);

	BOOST_TEST_MESSAGE( "args = " << args );

	BOOST_CHECK_NO_THROW( init( args, &factory ) );

	eq::Window *eq_win = config->getNodes().at(0)->getPipes().at(0)->getWindows().at(0);
	eqOgre::Window *win = dynamic_cast<eqOgre::Window *>(eq_win);
	BOOST_CHECK( win );

	mainloop();
}
*/
BOOST_AUTO_TEST_SUITE_END()