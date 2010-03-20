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

namespace vl
{

namespace graph
{
	// Forward decalrations
	class SceneManager;

	class Entity : public MovableObject
	{
		public :
			// Function to really do the loading of the mesh
			// Only usefull on Nodes
			virtual void load( SceneManager *) = 0;

	};	// class Entity

}	// namespace graph

}	// namespace vl

#endif
