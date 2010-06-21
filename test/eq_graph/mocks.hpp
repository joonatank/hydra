#ifndef VL_TEST_MOCKS_HPP
#define VL_TEST_MOCKS_HPP

#include <turtle/mock.hpp>

#include "interface/scene_node.hpp"
#include "interface/movable_object.hpp"
#include "interface/light.hpp"
#include "interface/entity.hpp"
#include "interface/camera.hpp"
#include "interface/scene_manager.hpp"

#include "settings.hpp"

namespace mock
{

MOCK_BASE_CLASS( Settings, vl::Settings )
{
	MOCK_METHOD( getFilePath, 0 )
	MOCK_METHOD( getEqConfigPath, 0 )
	MOCK_METHOD( getOgrePluginsPath, 0 )
	MOCK_METHOD( getOgreResourcePaths, 0 )
	MOCK_METHOD( getScenes, 0 )
	MOCK_NON_CONST_METHOD_EXT( getEqArgs, 0, vl::Args &( void ), getEqArgs )
	MOCK_CONST_METHOD_EXT( getEqArgs, 0, vl::Args const &( void ), getEqArgsConst )
	MOCK_METHOD( setExePath, 1 )
	MOCK_METHOD( setEqConfig, 1 )
	MOCK_METHOD( addPlugins, 1 )
	MOCK_METHOD( addResources, 1 )
	MOCK_METHOD( addScene, 1 );
	MOCK_METHOD( getTracking, 0 )
	MOCK_METHOD( addTracking, 1);
	MOCK_METHOD( findRoot, 1 );
	MOCK_METHOD( addRoot, 1 );
	MOCK_NON_CONST_METHOD_EXT( getRoot, 1, vl::Settings::Root &( size_t index ), getRoot )
	MOCK_CONST_METHOD_EXT( getRoot, 1, vl::Settings::Root const &( size_t index ), getRootConst )
	MOCK_METHOD( nRoots, 0);
	MOCK_METHOD( clear, 0 );
};

typedef boost::shared_ptr<Settings> SettingsPtr;

MOCK_BASE_CLASS( NodeFactory, vl::graph::SceneNodeFactory )
{
	MOCK_METHOD( create, 2 )
};

MOCK_BASE_CLASS( ObjectFactory, vl::graph::MovableObjectFactory )
{
	MOCK_METHOD( create, 2 )
	MOCK_METHOD( typeName, 0 )
};

MOCK_BASE_CLASS( MovableObject, vl::graph::MovableObject )
{
	MOCK_METHOD( getTypename, 0 )
	MOCK_METHOD( setManager, 1 )
};

MOCK_BASE_CLASS( Camera, vl::graph::Camera )
{
	MOCK_METHOD( getTypename, 0 )
	MOCK_METHOD( setManager, 1 )
	MOCK_METHOD( setViewMatrix, 1 )
	MOCK_METHOD( setProjectionMatrix, 1 )
	MOCK_METHOD( setFarClipDistance, 1 )
	MOCK_METHOD( setNearClipDistance, 1 )
	MOCK_METHOD( setPosition, 1 )
};

MOCK_BASE_CLASS( Entity, vl::graph::Entity )
{
	MOCK_METHOD( getTypename, 0 )
	MOCK_METHOD( setManager, 1 )
	MOCK_METHOD( setMaterialName, 1 )
	MOCK_METHOD( load, 0 )
	MOCK_METHOD( setCastShadows, 1 )
};

MOCK_BASE_CLASS( Light, vl::graph::Light )
{
	MOCK_METHOD( getTypename, 0 )
	MOCK_METHOD( setManager, 1 )
	MOCK_METHOD( setDirection, 1 )
	MOCK_METHOD( setSpotlightRange, 3 )
	MOCK_METHOD( setAttenuation, 4 )
	MOCK_METHOD( setType, 1 )
	MOCK_METHOD( setVisible, 1 )
	MOCK_METHOD( setCastShadows, 1 )
	MOCK_METHOD( setDiffuseColour, 1 )
	MOCK_METHOD( setSpecularColour, 1 )
};

MOCK_BASE_CLASS( SceneNode, vl::graph::SceneNode )
{
	MOCK_METHOD( getName, 0 )
	MOCK_METHOD( translate, 2 )
	MOCK_METHOD( setPosition, 2 )
	MOCK_METHOD( getPosition, 1 )
	MOCK_METHOD( rotate, 2 )
	MOCK_METHOD( setOrientation, 2 )
	MOCK_METHOD( getOrientation, 1 )
	MOCK_METHOD( lookAt, 1 )
	MOCK_METHOD( setDirection, 1 )
	MOCK_METHOD_EXT( scale, 1, void( vl::vector const & ), scaleVector )
	MOCK_METHOD_EXT( scale, 1, void( vl::scalar const ), scaleScalar )
	MOCK_METHOD( setScale, 1 )
	MOCK_METHOD( getScale, 0 )
	MOCK_METHOD( attachObject, 1 )
	MOCK_METHOD( detachObject, 1 )
	MOCK_METHOD( getAttached, 0 )
	MOCK_METHOD( numAttached, 0 )
	MOCK_METHOD( createChild, 1 )
	MOCK_METHOD( setParent, 1 )
	MOCK_METHOD( addChild, 1 )
	MOCK_METHOD_EXT( removeChild, 1, void( vl::graph::SceneNodeRefPtr ), removeChild )
	MOCK_METHOD_EXT( removeChild, 1,
			vl::graph::SceneNodeRefPtr( uint16_t ), removeChildByIndex )
	MOCK_METHOD_EXT( removeChild, 1,
			vl::graph::SceneNodeRefPtr( std::string const & ), removeChildByName )
	MOCK_METHOD_EXT( getChild, 1, vl::graph::SceneNodeRefPtr( uint16_t ), getChildByIndex )
	MOCK_METHOD_EXT( getChild, 1,
			vl::graph::SceneNodeRefPtr( std::string const & ), getChildByName )
	MOCK_METHOD( getChilds, 0 )
	MOCK_METHOD( numChildren, 0 )
	MOCK_METHOD( getParent, 0 )
	MOCK_METHOD( getManager, 0 )
};

MOCK_BASE_CLASS( SceneManager, vl::graph::SceneManager )
{
	MOCK_METHOD( destroyGraph, 0 )
	MOCK_METHOD( getName, 0 )
	MOCK_METHOD( getRootNode, 0 )
	MOCK_METHOD( setAmbientLight, 1 )
	MOCK_METHOD( getAmbientLight, 0 )
	MOCK_METHOD( setFog, 5 )
	MOCK_METHOD( createNode, 1 )
	MOCK_METHOD( createMovableObject, 3 )
	MOCK_METHOD( createEntity, 2 )
	MOCK_METHOD( createLight, 1 )
	MOCK_METHOD( createCamera, 1 )
	MOCK_METHOD_EXT( getNode, 1, vl::graph::SceneNodeRefPtr( uint32_t ), getById )
	MOCK_METHOD_EXT( getNode, 1, vl::graph::SceneNodeRefPtr( std::string const & ), getByName )
	MOCK_METHOD( getObject, 1 )
	MOCK_METHOD( getLight, 1 )
	MOCK_METHOD( getCamera, 1 )
	MOCK_METHOD( setSceneNodeFactory, 1 )
	MOCK_METHOD( addMovableObjectFactory, 2 )
	MOCK_METHOD_EXT( removeMovableObjectFactory, 1,
			void( vl::graph::MovableObjectFactoryPtr ), remMovableFactoryByPtr )
	MOCK_METHOD_EXT( removeMovableObjectFactory, 1,
			void( std::string const & ), remMovableFactoryByName )
	MOCK_METHOD( movableObjectFactories, 0 )
};

typedef boost::shared_ptr<NodeFactory> NodeFactoryPtr;
typedef boost::shared_ptr<ObjectFactory> ObjectFactoryPtr;
typedef boost::shared_ptr<MovableObject> MovableObjectPtr;
typedef boost::shared_ptr<Entity> EntityPtr;
typedef boost::shared_ptr<Light> LightPtr;
typedef boost::shared_ptr<Camera> CameraPtr;
typedef boost::shared_ptr<SceneNode> SceneNodePtr;
typedef boost::shared_ptr<SceneManager> SceneManagerPtr;

}	// namespace mock

#endif