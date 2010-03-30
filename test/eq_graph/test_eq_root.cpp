#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE ogre_root

#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_root.hpp"
#include "base/exceptions.hpp"

BOOST_AUTO_TEST_CASE( scene_manager )
{
	vl::graph::Root *root = new vl::cl::Root();
	// Test scene manager creation
	vl::graph::SceneManager *man = root->createSceneManager( "Man" );
	BOOST_CHECK( man );
//	BOOST_CHECK( dynamic_cast<vl::cl::SceneManager *>( man ) );
	BOOST_CHECK_THROW( root->createSceneManager(""), vl::empty_param );
	vl::graph::SceneManager *man2 = root->createSceneManager( "Manager" );
	
	// Test scene manager retrieval
	BOOST_CHECK_EQUAL( root->getSceneManager( "Man" ), man );
	BOOST_CHECK_EQUAL( root->getSceneManager( "Manager" ), man2 );
	BOOST_CHECK_EQUAL( root->getSceneManager( "Scene" ),
			(vl::graph::SceneManager *)0 );
	BOOST_CHECK_THROW( root->getSceneManager( "" ), vl::empty_param );
	
	// Test scene manager destruction
	BOOST_CHECK_NO_THROW( root->destroySceneManager( "Man" ) );
	BOOST_CHECK_EQUAL( root->getSceneManager( "Man" ),
			(vl::graph::SceneManager *)0 );
	BOOST_CHECK_THROW( root->destroySceneManager( "Scene" ), vl::no_object );
	BOOST_CHECK_THROW( root->destroySceneManager( "" ), vl::empty_param );

	delete root;
}
