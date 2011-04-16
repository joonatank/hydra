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
class KinematicAction : public vl::MoveAction
{
public :
	KinematicAction( void )
		: _body(0)
	{}

	virtual std::string getTypeName( void ) const
	{ return "KinematicAction"; }

	static KinematicAction *create( void )
	{ return new KinematicAction; }

	void setRigidBody(vl::physics::RigidBody *body)
	{ _body = body; }

	vl::physics::RigidBody *getRigidBody( void )
	{ return _body; }

/// Private virtual methods
private :
	virtual void move(Ogre::Vector3 const &v, bool local);
	virtual void rotate(Ogre::Quaternion const &v, bool local);

protected :
	vl::physics::RigidBody *_body;

};	// class KinematicAction

class DynamicAction : public vl::MoveAction
{
public:
	DynamicAction( void )
		: _body(0), _force(0,0,0), _torque(0,0,0)
	{}

	virtual std::string getTypeName( void ) const
	{ return "DynamicAction"; }

	static DynamicAction *create( void )
	{ return new DynamicAction; }

	void setRigidBody(vl::physics::RigidBody *body)
	{ _body = body; }

	vl::physics::RigidBody *getRigidBody( void )
	{ return _body; }

	void setForce( Ogre::Vector3 const &vec )
	{ _force = vec; }

	Ogre::Vector3 const &getForce( void )
	{ return _force; }

	void setTorque( Ogre::Vector3 const &vec )
	{ _torque = vec; }

	Ogre::Vector3 const &getTorque( void )
	{ return _torque; }

/// Private virtual methods
private :
	virtual void move(Ogre::Vector3 const &v, bool local);
	virtual void rotate(Ogre::Quaternion const &v, bool local);

private:
	vl::physics::RigidBody *_body;

	Ogre::Vector3 _force;
	Ogre::Vector3 _torque;

};	// class DynamicAction


}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_EVENTS_HPP

