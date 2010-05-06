#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ogre_root

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_root.hpp"
#include "../fixtures.hpp"

BOOST_GLOBAL_FIXTURE( InitFixture )

BOOST_AUTO_TEST_CASE( constructor_test )
{
	// Test we can create multiple roots with single application
	// i.e. we can bypass the problem of Ogre::Root singleton.
	vl::ogre::Root *root1, *root2;
	BOOST_CHECK_NO_THROW( root1 = new vl::ogre::Root );
	BOOST_CHECK_NO_THROW( root2 = new vl::ogre::Root );
	delete root1;
	delete root2;
}

BOOST_AUTO_TEST_CASE( init )
{
	vl::graph::Root *root = 0;
	BOOST_CHECK_NO_THROW( root = new vl::ogre::Root );

	BOOST_CHECK_NO_THROW( root->createRenderSystem() );

	// Window needs to be created before init, because init loads the
	// resources which need the Rasterizer to be initialized, which needs
	// the window.
	vl::graph::RenderWindowRefPtr win;
	BOOST_CHECK_NO_THROW( win = root->createWindow("win", 800, 600) );
	BOOST_CHECK_NO_THROW( root->init() );
	BOOST_CHECK( win );

	// TODO check window creation with different parameters
}
