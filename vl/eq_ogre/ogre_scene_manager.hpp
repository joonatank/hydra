#ifndef VL_OGRE_SCENE_MANAGER_HPP
#define VL_OGRE_SCENE_MANAGER_HPP

#include "eq_graph/eq_scene_manager.hpp"

#include <OGRE/OgreSceneManager.h>

#include "eq_ogre/ogre_camera.hpp"
#include "eq_ogre/ogre_scene_node.hpp"

namespace vl
{
namespace ogre
{
	class SceneManager : public vl::cl::SceneManager
	{
		public :

			SceneManager( std::string const &name );

			virtual ~SceneManager( void );

			Ogre::SceneManager *getNative( void )
			{ return _ogre_sm; }

		protected :
			Ogre::SceneManager *_ogre_sm;

	};	// class SceneManager

}	// namespace ogre

} 	// namespace vl
#endif

