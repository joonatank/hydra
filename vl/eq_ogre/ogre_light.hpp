/*	Joonatan Kuosa
 *	2010-05
 */

#ifndef VL_OGRE_LIGHT_HPP
#define VL_OGRE_LIGHT_HPP

#include "ogre_movable_object.hpp"

#include "eq_graph/eq_light.hpp"

#include <OGRE/OgreLight.h>

namespace vl
{

namespace ogre
{
	class Light : public cl::Light, public ogre::MovableObject
	{
		public :
			Light( std::string const &name,
				   vl::NamedValuePairList const &params )
				: cl::Light( name, params )
			{}
			
			virtual ~Light( void ) {}

			virtual Ogre::MovableObject *getNative( void )
			{ return _ogre_light; }

			virtual void setManager( vl::graph::SceneManagerRefPtr man );

			virtual void setDirection( vl::vector const &dir );

			virtual void setSpotlightRange(vl::angle const inner,
					vl::angle const outer,
					vl::scalar const falloff);

			virtual void setAttenuation( vl::scalar const range,
					vl::scalar const constant,
					vl::scalar const linear, vl::scalar const quadratic );

			virtual void setType( TYPE type );

			virtual void setVisible( bool visible );

			virtual void setCastShadows( bool shadows );

			virtual void setDiffuseColour( vl::colour col );

			virtual void setSpecularColour( vl::colour col );

		protected :
			Ogre::Light *_ogre_light;
			
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

}	// namespace ogre

}	// namespace vl

#endif

