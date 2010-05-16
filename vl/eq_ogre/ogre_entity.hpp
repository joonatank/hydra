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

			virtual void setManager( vl::graph::SceneManagerRefPtr man );
			
			virtual Ogre::MovableObject *getNative( void )
			{
				return _ogre_entity;
			}

			// Function to really do the loading of the mesh
			// Only usefull on Nodes
			virtual void load( void );

			virtual void setCastShadows( bool castShadows);

			virtual void setMaterialName( std::string const &materialFile);

		protected :
			Ogre::Entity *_ogre_entity;

			std::string _name;

	};	// class Entity

	class EntityFactory : public vl::graph::MovableObjectFactory
	{
		public :
			EntityFactory( void ) {}

			virtual ~EntityFactory( void ) {}

			virtual vl::graph::MovableObjectRefPtr create( std::string const &name,
					vl::NamedValuePairList const &params );
			
			virtual std::string const &typeName( void )
			{ return TYPENAME; }

			static const std::string TYPENAME;

	};	// class EntityFactory

}	// namespace graph

}	// namespace vl

#endif
