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

#include "eq_movable_object.hpp"

#include "base/typedefs.hpp"

#include <string>

#include "interface/scene_manager.hpp"

namespace vl
{

namespace cl
{
	// Forward decalrations
	class SceneManager;

	class Entity : public MovableObject
	{
		public :
			Entity( std::string const &name,
					vl::NamedValuePairList const &params )
				: MovableObject(name, params)
			{}

			virtual ~Entity( void ) {}

			// Function to really do the loading of the mesh
			// Only usefull on Nodes
			virtual void load( vl::graph::SceneManager *) {}

	};	// class Entity

}	// namespace graph

}	// namespace vl

#endif
