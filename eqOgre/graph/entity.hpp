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

#ifndef VL_GRAPH_ENTITY_HPP
#define VL_GRAPH_ENTITY_HPP

#include "movable_object.hpp"

#include "../typedefs.hpp"

#include <string>

namespace vl
{

namespace graph
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
			virtual void load( SceneManager *) 
			{}

	};	// class Entity

}	// namespace graph

}	// namespace vl

#endif
