#include "eq_entity.hpp"

vl::cl::Entity::Entity( std::string const &name, 
	vl::NamedValuePairList const &params )
	: eq::Object(), _name(name), _params(params)
{}

void
vl::cl::Entity::setManager( vl::graph::SceneManagerRefPtr man )
{ _manager = man; }

void
vl::cl::Entity::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	if( DIRTY_PARAMS & dirtyBits )
	{
		os << _params;
	}
}

void
vl::cl::Entity::deserialize( eq::net::DataIStream& is,
		const uint64_t dirtyBits )
{
	if( DIRTY_PARAMS & dirtyBits )
	{
		is >> _params;
	}
}

// ------- EntityFactory --------
const std::string vl::cl::EntityFactory::TYPENAME("Entity");

vl::graph::MovableObjectRefPtr
vl::cl::EntityFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	return vl::graph::MovableObjectRefPtr( new Entity( name, params ) );
}
