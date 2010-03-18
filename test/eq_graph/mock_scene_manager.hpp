#ifndef MOCK_SCENE_MANAGER_HPP
#define MOCK_SCENE_MANAGER_HPP

#include <turtle/mock.hpp>

#include "interface/scene_manager.hpp"

using vl::graph::SceneNode;
using vl::graph::SceneManager;
using vl::graph::MovableObject;

MOCK_BASE_CLASS( mock_scene_manager, SceneManager )
{
	MOCK_METHOD_EXT( destroy, 1, void( MovableObject* ), destroyObj)
	MOCK_METHOD_EXT( destroy, 1, void( SceneNode* ), destroyNode )
	MOCK_METHOD( getRootNode, 0 )
	MOCK_METHOD( createNode, 1 )
	MOCK_METHOD( createEntity, 2 )
	MOCK_METHOD( createCamera, 1 )
	MOCK_METHOD_EXT( getNode, 1, SceneNode*( uint32_t ), getById )
	MOCK_METHOD_EXT( getNode, 1, SceneNode*( std::string const & ), getByName )
	MOCK_METHOD( getObject, 1 )
};

#endif
