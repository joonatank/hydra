/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Updated 2011-01
 *
 */

#include "physics_events.hpp"

#include "base/exceptions.hpp"

void
vl::physics::ApplyForce::execute( void )
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_body->applyCentralForce(_force);
}


void
vl::physics::ApplyTorque::execute( void )
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_body->applyTorqueImpulse(_torque);
}

/// @todo fix to use the parameters
void
vl::physics::KinematicAction::move(Ogre::Vector3 const &v, bool local)
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_body->setLinearVelocity(_move_dir*_speed);
}


void
vl::physics::KinematicAction::rotate(Ogre::Quaternion const &v, bool local)
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_body->setAngularVelocity(_rot_dir*_angular_speed.valueRadians());
}

void
vl::physics::DynamicAction::move(Ogre::Vector3 const &v, bool local)
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	if( !_move_dir.isZeroLength() )
	{
		vl::scalar x = _force.x*_move_dir.x;
		vl::scalar y = _force.y*_move_dir.y;
		vl::scalar z = _force.z*_move_dir.z;
		_body->applyForce(Ogre::Vector3(x,y,z), Ogre::Vector3::ZERO );
	}
}

void
vl::physics::DynamicAction::rotate(Ogre::Quaternion const &v, bool local)
{
	if( !_body )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	if( !_rot_dir.isZeroLength() )
	{
		vl::scalar x = _torque.x*_move_dir.x;
		vl::scalar y = _torque.y*_move_dir.y;
		vl::scalar z = _torque.z*_move_dir.z;
		_body->applyTorque( Ogre::Vector3(x, y, z) );
	}
}
