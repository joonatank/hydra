/*	Joonatan Kuosa
 *	2010-04
 *
 *	Light is a class of MovableObjects
 *
 *  This class handles Equalizer implementation of a Light.
 *  So it's data is distributed.
 */

#ifndef VL_CL_LIGHT_HPP
#define VL_CL_LIGHT_HPP

#include "movable_object.hpp"

namespace vl
{

namespace cl
{
	class Light : public graph::Light
	{
		public :
			Light( void ) {}

			virtual ~Light( void ) {}

			virtual void setSpotlightRange( vl::angle const inner,
					vl::angle const outer,
					vl::scalar const falloff );

			virtual void setAttenuation( vl::scalar const range,
					vl::scalar const constant,
					vl::scalar const linear, vl::scalar const quadratic );

			virtual void setType( TYPE type );

	};	// class Light

	class LightFactory : public vl::graph::MovableObjectFactory
	{
		public :
			LightFactory( void ) {}

			virtual ~LightFactory( void ) {}

			virtual vl::graph::MovableObjectRefPtr create( std::string const &name,
					vl::NamedValuePairList const &params );
			
			virtual std::string const &typeName( void )
			{ return TYPENAME; }

			static const std::string TYPENAME;

	};	// class LightFactory

}	// namespace cl

}	// namespace vl

#endif


