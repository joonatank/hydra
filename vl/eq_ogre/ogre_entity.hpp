/*	Joonatan Kuosa
 *	2010-02
 *
 *	Entity is a class of MovableObjects, has a mesh which atm is represented by
 *	the name of the mesh file.
 *
 *	This class should not depend on any external library.
 *	All conversions to external libraries should be done in separate class
 *	either through inheritance or aggregation.
 */

#ifndef VL_OGRE_ENTITY_HPP
#define VL_OGRE_ENTITY_HPP

#include <OGRE/OgreEntity.h>

#include "ogre_movable_object.hpp"

#include "eq_graph/eq_entity.hpp"

namespace vl
{

namespace ogre
{
	class Entity : public vl::cl::Entity, public vl::ogre::MovableObject
	{
		public :
			Entity( std::string const &name,
					vl::NamedValuePairList const &params )
				: vl::cl::Entity(name, params), _ogre_entity(0)
			{}

			virtual ~Entity( void ) {}

			virtual Ogre::MovableObject *getNative( void )
			{ return _ogre_entity; }

			// Function to really do the loading of the mesh
			// Only usefull on Nodes
			virtual void load( vl::graph::SceneManager *sm );

		protected :
			Ogre::Entity *_ogre_entity;

	};	// class Entity

}	// namespace graph

}	// namespace vl

#endif
