#ifndef VL_OGRE_SCENE_MANAGER_HPP
#define VL_OGRE_SCENE_MANAGER_HPP

#include "../graph/scene_manager.hpp"

#include <OGRE/OgreSceneManager.h>

#include "../math/conversion.hpp"

#include "ogre_camera.hpp"
#include "ogre_scene_node.hpp"

namespace vl
{
namespace ogre
{
	// Functors for creation and destruction of objects,
	// these handle mapping of objects.
	class CreateNodeFunc : public vl::graph::CreateNodeFunc
	{
		public :
			// SceneManager of which scene graph this functor operates
			CreateNodeFunc( vl::graph::SceneManager *sm );

			virtual vl::graph::SceneNode *operator()( uint32_t id );

	};	// class CreateNodeFunc

	class DeleteNodeFunc : public vl::graph::DeleteNodeFunc
	{
		public :
			// SceneManager of which scene graph this functor operates
			DeleteNodeFunc( vl::graph::SceneManager *sm );

			virtual vl::graph::SceneNode *operator()( uint32_t id );

	};	// class DeleteNodeFunc

	class CreateObjectFunc : public vl::graph::CreateObjectFunc
	{
		public :
			// SceneManager of which scene graph this functor operates
			CreateObjectFunc( vl::graph::SceneManager *sm );

			virtual vl::graph::MovableObject *operator()( uint32_t id );

	};	// class CreateObjectFunc

	class DeleteObjectFunc : public vl::graph::DeleteObjectFunc
	{
		public :
			// SceneManager of which scene graph this functor operates
			DeleteObjectFunc( vl::graph::SceneManager *sm );

			virtual vl::graph::MovableObject *operator()( uint32_t id );

	};	// class DeleteObjectFunc

	class SceneManager : public vl::graph::SceneManager
	{
		public :

			SceneManager( std::string const &name );

			Ogre::SceneManager *getNative( void )
			{ return _ogre_sm; }

			virtual vl::graph::Camera *createCamera( std::string const &name )
			{
				Ogre::Camera *ogre_cam = _ogre_sm->createCamera(name);
				return new Camera( ogre_cam );
			}

		protected :
			// Override creation methods
			virtual vl::graph::SceneNode *
				_createSceneNodeImpl( std::string const &name );

			virtual vl::graph::MovableObject* _createMovableObjectImpl(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() );

			// Override the container functors
			virtual void _createDistribContainers( void );

			Ogre::SceneManager *_ogre_sm;

	};	// class SceneManager

}	// namespace ogre

} 	// namespace vl
#endif

