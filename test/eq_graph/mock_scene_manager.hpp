#ifndef MOCK_SCENE_MANAGER_HPP
#define MOCK_SCENE_MANAGER_HPP

#include <turtle/mock.hpp>

#include "interface/scene_manager.hpp"

#include "base/typedefs.hpp"

using vl::graph::SceneNodeRefPtr;
using vl::graph::SceneManagerRefPtr;
using vl::graph::MovableObjectRefPtr;

MOCK_BASE_CLASS( mock_scene_manager, vl::graph::SceneManager )
{
//	MOCK_METHOD_EXT( destroy, 1, void( MovableObject* ), destroyObj)
//	MOCK_METHOD_EXT( destroy, 1, void( SceneNode* ), destroyNode )
	MOCK_METHOD( getRootNode, 0 )
	MOCK_METHOD( getName, 0 )
	MOCK_METHOD( destroyGraph, 0 )
	MOCK_METHOD( createNode, 1 )
	MOCK_METHOD( createMovableObject, 3 )
	MOCK_METHOD( createEntity, 2 )
	MOCK_METHOD( createCamera, 1 )
	MOCK_METHOD_EXT( getNode, 1, SceneNodeRefPtr( uint32_t ), getById )
	MOCK_METHOD_EXT( getNode, 1, SceneNodeRefPtr( std::string const & ), getByName )
	MOCK_METHOD( getObject, 1 )
	MOCK_METHOD( pushChildAddedStack, 2 )
	MOCK_METHOD( pushChildRemovedStack, 1 )
	MOCK_METHOD( setSceneNodeFactory, 1 )
	MOCK_METHOD( addMovableObjectFactory, 2 )
	MOCK_METHOD_EXT( removeMovableObjectFactory, 1,
			void( vl::graph::MovableObjectFactoryPtr ), remMovableFactoryByPtr )
	MOCK_METHOD_EXT( removeMovableObjectFactory, 1,
			void( std::string const & ), remMovableFactoryByName )
	MOCK_METHOD( movableObjectFactories, 0 )
};

#endif
