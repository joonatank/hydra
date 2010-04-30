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

			virtual vl::graph::CameraRefPtr createCamera( std::string const &name );

		protected :
			// Override creation methods
			// TODO provide factory
			/*
			virtual vl::cl::SceneNodeRefPtr
				_createSceneNodeImpl( std::string const &name );

			virtual vl::graph::MovableObject* _createMovableObjectImpl(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() );
			*/

			Ogre::SceneManager *_ogre_sm;

	};	// class SceneManager

}	// namespace ogre

} 	// namespace vl
#endif

