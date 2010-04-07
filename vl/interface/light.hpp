/*	Joonatan Kuosa
 *	2010-04
 *
 *	Light is a class of MovableObjects
 *
 *	This class should not depend on any external library.
 *	All conversions to external libraries should be done in separate class
 *	either through inheritance or aggregation.
 */

#ifndef VL_GRAPH_LIGHT_HPP
#define VL_GRAPH_LIGHT_HPP

#include "movable_object.hpp"

namespace vl
{

namespace graph
{
	class Light : public MovableObject
	{
		public :
			enum TYPE
			{
				LT_POINT,
				LT_DIRECTIONAL,
				LT_SPOTLIGHT
			};

			virtual ~Light( void ) {}

			virtual void setSpotlightRange(vl::angle const inner,
					vl::angle const outer,
					vl::scalar const falloff) = 0;

			virtual void setAttenuation( vl::scalar const range,
					vl::scalar const constant,
					vl::scalar const linear, vl::scalar const quadratic ) = 0;

			virtual void setType( TYPE type ) = 0;

			virtual void setVisible( bool visible ) = 0;

			virtual void setCastShadows( bool shadows ) = 0;

			virtual void setDiffuseColour( vl::colour col ) = 0;

			virtual void setSpecularColour( vl::colour col ) = 0;

	};	// class Light

}	// namespace graph

}	// namespace vl

#endif

