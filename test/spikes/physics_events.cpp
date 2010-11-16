/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include "physics_events.hpp"

void
vl::physics::ApplyForce::execute( void )
{
	if( _body )
	{
		_body->applyForce( _force, btVector3(0,0,0) );
	}
}


std::string const &vl::physics::ApplyForce::getTypeName( void ) const
{
	return ApplyForceFactory::TYPENAME;
}

const std::string vl::physics::ApplyForceFactory::TYPENAME = "ApplyForce";