/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Event Handling classes for Physics events
 *	Mostly Actions for now
 *
 *	Updated 2011-01
 *	Moved to the main tree
 */

#ifndef VL_PHYSICS_EVENTS_HPP
#define VL_PHYSICS_EVENTS_HPP

// Base classes for event handling
#include "action.hpp"

#include "rigid_body.hpp"

#include "actions_transform.hpp"

namespace vl
{

namespace physics
{

class ApplyForce : public vl::BasicAction
{
public :
	ApplyForce( void )
		: _body(0), _force(0,0,0)
	{}

	void setForce( Ogre::Vector3 const &vec )
	{ _force = vec; }

	Ogre::Vector3 const &getForce( void ) const
	{ return _force; }

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ApplyForce"; }

	static ApplyForce *create( void )
	{ return new ApplyForce; }

	void setRigidBody( vl::physics::RigidBody *body )
	{ _body = body; }

	vl::physics::RigidBody *getRigidBody( void )
	{ return _body; }

protected :
	vl::physics::RigidBody *_body;

	Ogre::Vector3 _force;
};

class ApplyTorque : public vl::BasicAction
{
public :
	ApplyTorque( void )
		: _body(0), _torque(0,0,0)
	{}

	Ogre::Vector3 const &getTorque( void ) const
	{ return _torque; }

	void setTorque( Ogre::Vector3 const &vec )
	{ _torque = vec; }

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ApplyTorque"; }

	static ApplyTorque *create( void )
	{ return new ApplyTorque; }

	void setRigidBody(vl::physics::RigidBody *body)
	{ _body = body; }

	vl::physics::RigidBody *getRigidBody( void )
	{ return _body; }

protected :
	vl::physics::RigidBody *_body;

	Ogre::Vector3 _torque;

};

/// @todo the implementation of this should use two actions ApplyForce and ApplyTorque
/// @todo this should use velocities
/// we should have a different force based action
/// basically this one is kinematic action and it would be dynamics action
class MoveAction : public vl::TransformationAction
{
public :
	MoveAction( void )
		: _body(0), _force(0,0,0), _torque(0,0,0), _move_dir(0,0,0),
		_rot_dir(0,0,0)
	{}

	void setForce( Ogre::Vector3 const &vec )
	{ _force = vec; }

	Ogre::Vector3 const &getForce( void )
	{ return _force; }

	void setTorque( Ogre::Vector3 const &vec )
	{ _torque = vec; }

	Ogre::Vector3 const &getTorque( void )
	{ return _torque; }

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "PhysicsMoveAction"; }

	static MoveAction *create( void )
	{ return new MoveAction; }

	void setRigidBody(vl::physics::RigidBody *body)
	{ _body = body; }

	vl::physics::RigidBody *getRigidBody( void )
	{ return _body; }

	void setMoveDir( Ogre::Vector3 const &mov_dir )
	{
		_move_dir = mov_dir;
	}

	void setRotDir( Ogre::Vector3 const &rot_dir )
	{
		_rot_dir = rot_dir;
	}

protected :
	vl::physics::RigidBody *_body;

	Ogre::Vector3 _force;
	Ogre::Vector3 _torque;
	Ogre::Vector3 _move_dir;
	Ogre::Vector3 _rot_dir;

};	// class MoveAction


}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_EVENTS_HPP

