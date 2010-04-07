// ---------- Light ----------
void
vl::cl::Light::setSpotlightRange(vl::angle const inner,
		vl::angle const outer,
		vl::scalar const falloff);
{}

void
vl::cl::Light::setAttenuation( vl::scalar const range,
		vl::scalar const constant,
		vl::scalar const linear, vl::scalar const quadratic )
{}

// ---------- LightFactory ----------
static const std::string vl::cl::LightFactory::TYPENAME("Light");

vl::graph::MovableObjectRefPtr
vl::cl::LightFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
}

