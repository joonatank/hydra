
#ifndef VL_GRAPH_MOVABLE_OBJECT_HPP
#define VL_GRAPH_MOVABLE_OBJECT_HPP

#include "base/typedefs.hpp"

namespace vl
{

namespace graph
{
	class MovableObject
	{
		public :
			virtual ~MovableObject( void ) {}

			virtual void setManager( vl::graph::SceneManagerRefPtr man ) = 0;

	};	// class MovableObject

	class MovableObjectFactory
	{
		public :
			virtual ~MovableObjectFactory( void ) {}

			virtual MovableObjectRefPtr create( std::string const &name,
					vl::NamedValuePairList const &params ) = 0;
			
			virtual std::string const &typeName( void ) = 0;

	};	// class MovableObjectFactory

}	// namespace graph

}	// namespace vl

#endif
