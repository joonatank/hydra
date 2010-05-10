/*	Joonatan Kuosa
 *	2010-05
 *	Test module for dotscene loader
 *	Creates xml file based on the test cases parses it with rapidxml and
 *	DotsceneLoader, verifies that correct objects are created based on the
 *	dotscene xml definition.
 *
 *	Uses vl::cl::SceneManager, vl::cl::Entity, vl::cl::Light, vl::cl::SceneNode,
 *	vl::cl::Camera
 *	Might want to replace the vl::cl object by mocks.
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE dotscene

#include <boost/test/unit_test.hpp>

#include "dotscene_loader.hpp"
#include "base/exceptions.hpp"

// vl::cl objects
#include "eq_graph/eq_scene_manager.hpp"
#include "eq_graph/eq_scene_node.hpp"
#include "eq_graph/eq_entity.hpp"
#include "eq_graph/eq_light.hpp"
#include "eq_graph/eq_camera.hpp"

struct DotsceneFixture
{
	DotsceneFixture( void )
		: xml_data(0), sm( new vl::cl::SceneManager("Man") )
	{
		// Set factories
		sm->setSceneNodeFactory( vl::graph::SceneNodeFactoryPtr(
					new vl::cl::SceneNodeFactory ) );
		sm->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::cl::CameraFactory ) );
		sm->addMovableObjectFactory( vl::graph::MovableObjectFactoryPtr(
					new vl::cl::EntityFactory ) );
	}

	~DotsceneFixture( void )
	{
		delete [] xml_data;
	}

	// Generated data for the xml parser
	char *xml_data;
	vl::graph::SceneManagerRefPtr sm;
};

struct ResourceFixture : public DotsceneFixture
{
	ResourceFixture( void )
	{
	}

	~ResourceFixture( void )
	{
	}
};

struct EnvironmentFixture : public DotsceneFixture
{
	EnvironmentFixture( void )
	{
	}

	~EnvironmentFixture( void )
	{
	}
};

struct SceneNodeFixture : public DotsceneFixture
{
	SceneNodeFixture( void )
	{
	}

	~SceneNodeFixture( void )
	{
	}
};

BOOST_FIXTURE_TEST_SUITE( resources, ResourceFixture )

// Resource location test cases
BOOST_AUTO_TEST_CASE( test_resources_locations )
{

}

BOOST_AUTO_TEST_SUITE_END()

// Enviroment test cases
BOOST_FIXTURE_TEST_SUITE( environment, EnvironmentFixture )

BOOST_AUTO_TEST_CASE( test_viewport )
{

}

BOOST_AUTO_TEST_CASE( test_camera )
{

}

BOOST_AUTO_TEST_CASE( test_skybox )
{

}

BOOST_AUTO_TEST_CASE( test_dome )
{

}

BOOST_AUTO_TEST_CASE( test_ambient )
{

}

BOOST_AUTO_TEST_SUITE_END()

// Nodes test cases
BOOST_FIXTURE_TEST_SUITE( nodes, SceneNodeFixture )


BOOST_AUTO_TEST_CASE( test_scene_node )
{

}

BOOST_AUTO_TEST_CASE( test_entity )
{

}

BOOST_AUTO_TEST_CASE( test_light )
{

}

BOOST_AUTO_TEST_CASE( test_camera )
{

}

BOOST_AUTO_TEST_SUITE_END()