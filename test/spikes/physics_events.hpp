/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Event Handling classes for Physics events
 *	Mostly Actions for now
 */

#ifndef VL_PHYSICS_EVENTS_HPP
#define VL_PHYSICS_EVENTS_HPP

// Base classes for event handling
#include "eq_cluster/event.hpp"

// For now we use bullet objects
#include <bullet/btBulletDynamicsCommon.h>

namespace vl
{

namespace physics
{

class PhysicsOperation : public eqOgre::Operation
{
public :
	PhysicsOperation( void )
		: _body(0)
	{}

	void setRigidBody( btRigidBody *body )
	{ _body = body; }

	btRigidBody *getRigidBody( void )
	{ return _body; }

protected :
	btRigidBody *_body;
};

class ApplyForce : public PhysicsOperation
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

	virtual std::string const &getTypeName( void ) const;

protected :
	btVector3 _force;

};

class ApplyForceFactory : public eqOgre::OperationFactory
{
public :
	virtual eqOgre::Operation *create( void )
	{ return new ApplyForce; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_EVENTS_HPP