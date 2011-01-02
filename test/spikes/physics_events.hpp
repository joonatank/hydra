/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Event Handling classes for Physics events
 *	Mostly Actions for now
 */

#ifndef VL_PHYSICS_EVENTS_HPP
#define VL_PHYSICS_EVENTS_HPP

// Base classes for event handling
#include "eq_cluster/action.hpp"

// For now we use bullet objects
#include <bullet/btBulletDynamicsCommon.h>

namespace vl
{

namespace physics
{

class PhysicsAction : public vl::BasicAction
{
public :
	PhysicsAction( void )
		: _body(0)
	{}

	void setRigidBody( btRigidBody *body )
	{ _body = body; }

	btRigidBody *getRigidBody( void )
	{ return _body; }

protected :
	btRigidBody *_body;
};

class ApplyForce : public PhysicsAction
{
public :
	ApplyForce( void )
		: _force(0,0,0)
	{}

	void setForce( btVector3 const &vec )
	{ _force = vec; }

	btVector3 const &getForce( void )
	{ return _force; }

	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ApplyForce"; }

	static ApplyForce *create( void )
	{ return new ApplyForce; }

protected :
	btVector3 _force;

};

}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_EVENTS_HPP

