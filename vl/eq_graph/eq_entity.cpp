#include "eq_entity.hpp"

void
vl::cl::Entity::setManager( vl::graph::SceneManagerRefPtr man )
{ _manager = man; }

std::string const &
vl::cl::Entity::getTypename(void ) const
{ return EntityFactory::TYPENAME; }

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
