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

#include "interface/light.hpp"

#include <string>

#include "math/math.hpp"

namespace vl
{

namespace cl
{
	class Light : public graph::Light
	{
		public :
			Light( std::string const &name, vl::NamedValuePairList const &params )
				: _name(name)
			{}

			virtual ~Light( void ) {}

			virtual void setManager( vl::graph::SceneManagerRefPtr man )
			{ _manager = man; }

			virtual void setDirection( vl::vector const &dir ) {}
			
			virtual void setSpotlightRange( vl::angle const inner,
					vl::angle const outer,
					vl::scalar const falloff );

			virtual void setAttenuation( vl::scalar const range,
					vl::scalar const constant,
					vl::scalar const linear, vl::scalar const quadratic );

			virtual void setType( TYPE type );

			virtual void setVisible( bool visible ) {}

			virtual void setCastShadows( bool shadows ) {}

			virtual void setDiffuseColour( vl::colour col ) {}

			virtual void setSpecularColour( vl::colour col ) {}

		protected :
			vl::graph::SceneManagerRefPtr _manager;
			std::string _name;

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


