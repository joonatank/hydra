#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ogre_root

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"

BOOST_AUTO_TEST_CASE( constructor_test )
{
	BOOST_CHECK_NO_THROW( vl::ogre::Root() );
}

BOOST_AUTO_TEST_CASE( init )
{
	vl::graph::Root *root = 0;
	BOOST_CHECK_NO_THROW( root = new vl::ogre::Root );

	BOOST_CHECK_NO_THROW( root->createRenderSystem() );
	BOOST_CHECK_NO_THROW( root->init() );
	vl::graph::RenderWindow *win = 0;
	BOOST_CHECK_NO_THROW( win = root->createWindow("win", 800, 600) );
	BOOST_CHECK( win );

	// TODO check window creation with different parameters
}
