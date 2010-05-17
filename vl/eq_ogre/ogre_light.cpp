#include "ogre_light.hpp"

#include "base/exceptions.hpp"
#include "math/conversion.hpp"

#include "ogre_scene_manager.hpp"

// ----------------- Light -----------------
void
vl::ogre::Light::setDirection(const vl::vector& dir)
{
	vl::cl::Light::setDirection(dir);
	if( _ogre_light )
	{ _ogre_light->setDirection( vl::math::convert(dir) ); }
}

void
vl::ogre::Light::setManager(vl::graph::SceneManagerRefPtr man)
{
	vl::cl::Light::setManager(man);

	boost::shared_ptr<vl::ogre::SceneManager> og_man =
		boost::dynamic_pointer_cast<vl::ogre::SceneManager>( man );
	if( !og_man )
	{ throw vl::null_pointer( "vl::ogre::Light::Light" ); }

	_ogre_light = og_man->getNative()->createLight( _name );

	if( !_ogre_light )
	{ throw vl::null_pointer( "vl::ogre::Light::Light" ); }
}

void
vl::ogre::Light::setAttenuation( const vl::scalar range,
								 const vl::scalar constant,
								 const vl::scalar linear,
								 const vl::scalar quadratic )
{
	vl::cl::Light::setAttenuation(range, constant, linear, quadratic);
	if( _ogre_light )
	{ _ogre_light->setAttenuation(range, constant, linear, quadratic); }
}

void
vl::ogre::Light::setCastShadows(bool shadows)
{
	vl::cl::Light::setCastShadows(shadows);
	if( _ogre_light )
	{ _ogre_light->setCastShadows(shadows); }
}

void
vl::ogre::Light::setDiffuseColour(vl::colour col)
{
	vl::cl::Light::setDiffuseColour(col);
	if( _ogre_light )
	{ _ogre_light->setDiffuseColour( vl::math::convert( col ) ); }
}

void
vl::ogre::Light::setSpecularColour(vl::colour col)
{
	vl::cl::Light::setSpecularColour(col);
	if( _ogre_light )
	{ _ogre_light->setSpecularColour( vl::math::convert(col) ); }
}

void
vl::ogre::Light::setSpotlightRange(const vl::angle inner, const vl::angle outer, const vl::scalar falloff)
{
	vl::cl::Light::setSpotlightRange(inner, outer, falloff);
	if( _ogre_light )
	{ _ogre_light->setSpotlightRange( vl::math::convert(inner), vl::math::convert(outer), falloff); }
}

void
vl::ogre::Light::setType(vl::graph::Light::TYPE type)
{
	vl::cl::Light::setType(type);
	Ogre::Light::LightTypes og_type;
	switch( type )
	{
		case LT_DIRECTIONAL :
			og_type = Ogre::Light::LT_DIRECTIONAL;
			break;
		case LT_SPOTLIGHT :
			og_type = Ogre::Light::LT_SPOTLIGHT;
			break;
		case LT_POINT :
			og_type = Ogre::Light::LT_POINT;
			break;
	}
	
	if( _ogre_light )
	{ _ogre_light->setType( og_type ); }
}

void
vl::ogre::Light::setVisible(bool visible)
{
	vl::cl::Light::setVisible(visible);
	if( _ogre_light )
	{ _ogre_light->setVisible(visible); }
}

// ---------- LightFactory ----------
const std::string vl::ogre::LightFactory::TYPENAME("Light");

vl::graph::MovableObjectRefPtr
vl::ogre::LightFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	vl::graph::MovableObjectRefPtr obj( new Light( name, params ) );

	return obj;
}