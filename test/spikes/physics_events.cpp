/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include "physics_events.hpp"

#include "base/exceptions.hpp"

void
vl::physics::ApplyForce::execute( void )
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_body->applyForce( _force, btVector3(0,0,0) );
}
