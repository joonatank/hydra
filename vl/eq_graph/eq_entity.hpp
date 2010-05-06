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

#ifndef VL_EQ_GRAPH_ENTITY_HPP
#define VL_EQ_GRAPH_ENTITY_HPP

#include "interface/entity.hpp"

#include <eq/eq.h>

#include <string>

#include "interface/scene_manager.hpp"

namespace vl
{

namespace cl
{
	class Entity : public eq::Object, public vl::graph::Entity
	{
		public :
			Entity( std::string const &name,
					vl::NamedValuePairList const &params );
			
			virtual ~Entity( void ) {}

			virtual void setManager( vl::graph::SceneManagerRefPtr man );

			virtual std::string const &getName( void )
			{ return _name; }

			// Function to really do the loading of the mesh
			// Only usefull on Nodes
			virtual void load( void ) {}

			virtual void setCastShadows( bool castShadows)
			{}

			virtual void setMaterialName( std::string const &materialFile)
			{}

			// Equalizer overrides

			/* Object::serialize() */
			virtual void serialize( eq::net::DataOStream& os,
									const uint64_t dirtyBits );

			/* Object::deserialize() */
			virtual void deserialize( eq::net::DataIStream& is,
									  const uint64_t dirtyBits );

			/* Object::ChangeType */
			virtual ChangeType getChangeType() const
			{ return DELTA; }

			enum DirtyBits
			{
				// Dirty for the object container
				DIRTY_PARAMS = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 1
			};

		protected :
			vl::graph::SceneManagerWeakPtr _manager;

			std::string _name;
			vl::NamedValuePairList _params;

	};	// class Entity

	struct EntityFactory : public vl::graph::MovableObjectFactory
	{
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
