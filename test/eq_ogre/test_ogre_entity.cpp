#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ogre_entity

#include <boost/test/unit_test.hpp>

#include "eq_ogre/ogre_entity.hpp"

BOOST_AUTO_TEST_CASE( constructors_test )
{

}

// Requirements for loading Ogre Mesh are that Ogre is initialised
// we have a SceneManager and a Rendering Window.
// So we need a test fixture for this, just a simple one that initialises
// Ogre and creates a window.
BOOST_AUTO_TEST_CASE( loading )
{

}
