#include "eq_light.hpp"

// ---------- Light ----------
std::string const&
vl::cl::Light::getTypename(void ) const
{ return LightFactory::TYPENAME; }

void
vl::cl::Light::setDirection(const vl::vector& dir)
{

}

void
vl::cl::Light::setSpotlightRange(vl::angle const inner,
		vl::angle const outer, vl::scalar const falloff)
{}

void
vl::cl::Light::setAttenuation( vl::scalar const range,
		vl::scalar const constant, vl::scalar const linear,
		vl::scalar const quadratic )
{}

void
vl::cl::Light::setCastShadows(bool shadows)
{

}

void
vl::cl::Light::setDiffuseColour(vl::colour col)
{

}

void
vl::cl::Light::setSpecularColour(vl::colour col)
{

}

void
vl::cl::Light::setType(vl::graph::Light::TYPE type)
{

}

void
vl::cl::Light::setVisible(bool visible)
{

}

// ---------- LightFactory ----------
const std::string vl::cl::LightFactory::TYPENAME("Light");

vl::graph::MovableObjectRefPtr
vl::cl::LightFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	vl::graph::MovableObjectRefPtr obj( new Light( name, params ) );

	return obj;
}

