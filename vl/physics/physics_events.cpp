/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Updated 2011-01
 *
 */

#include "physics_events.hpp"

#include "base/exceptions.hpp"

void vl::physics::ApplyForce::execute(void )
{
	_body->applyForce( btVector3(_force.x, _force.y, _force.z), btVector3(0,0,0) );
}


void vl::physics::ApplyTorque::execute(void )
{
	_body->applyTorque( btVector3(_torque.x, _torque.y, _torque.z) );
}

void
vl::physics::MoveAction::execute( void )
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	if( !_move_dir.isZeroLength() )
	{
		vl::scalar x = _force.x*_move_dir.x;
		vl::scalar y = _force.y*_move_dir.y;
		vl::scalar z = _force.z*_move_dir.z;
		_body->applyForce( btVector3(x, y, z), btVector3(0,0,0) );
	}

	if( !_rot_dir.isZeroLength() )
	{
		vl::scalar x = _torque.x*_move_dir.x;
		vl::scalar y = _torque.y*_move_dir.y;
		vl::scalar z = _torque.z*_move_dir.z;
		_body->applyTorque( btVector3(x, y, z) );
	}
}
