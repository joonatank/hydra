/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file physics/rigid_body.cpp
 *
 *	This file is part of Hydra a VR game engine.
 */

/// Interface
#include "rigid_body.hpp"

#include "shapes.hpp"

/// Concrete implementations
#ifdef USE_BULLET
#include "rigid_body_bullet.hpp"
#else if USE_NEWTON
#include "rigid_body_newton.hpp"
#endif

/// --------------------------------- Global ---------------------------------
std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::RigidBody const &body)
{
	os << "RigidBody " << body.getName() << " : motion state " << *body.getMotionState();
	if( body.isUserControlled() )
	{ os << " : user controlled"; }

	// TODO add the rest

	return os;
}

vl::physics::RigidBodyRefPtr
vl::physics::RigidBody::create(vl::physics::RigidBody::ConstructionInfo const &info)
{
	RigidBodyRefPtr body;
#ifdef USE_BULLET
	body.reset(new BulletRigidBody(info));
#else if USE_NEWTON
#endif
	return body;
}

void 
vl::physics::RigidBody::applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos, vl::physics::RigidBodyRefPtr ref)
{
	if(!ref)
	{ applyForce(force, rel_pos); }
	else
	{ applyForce(ref->getWorldTransform().quaternion*force, rel_pos); }
}

void 
vl::physics::RigidBody::applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos, bool local)
{
	if(!local)
	{ applyForce(force, rel_pos); }
	else
	{
		vl::Transform from_world = getWorldTransform();
		applyForce(from_world.quaternion*force, rel_pos);
	}
}

void 
vl::physics::RigidBody::applyCentralForce(Ogre::Vector3 const &force, vl::ObjectInterface *ref)
{
	if(ref)
	{ applyCentralForce(ref->getWorldTransform().quaternion * force); }
	else
	{ applyCentralForce(force); }
}

void
vl::physics::RigidBody::applyCentralForce(Ogre::Vector3 const &force, vl::physics::RigidBodyRefPtr ref)
{
	if(ref)
	{ applyCentralForce(ref->getWorldTransform().quaternion * force); }
	else
	{ applyCentralForce(force); }
}

vl::Transform
vl::physics::RigidBody::transformToLocal(vl::Transform const &t) const
{
	vl::Transform from_world =  getMotionState()->getWorldTransform();
	from_world.invert();
	return from_world*t;
}


Ogre::Vector3
vl::physics::RigidBody::positionToLocal(Ogre::Vector3 const &v) const
{
	vl::Transform from_world = getMotionState()->getWorldTransform();
	from_world.invert();
	return from_world*v;
}

/// --------------------------------- Protected ------------------------------
vl::physics::RigidBody::RigidBody(vl::physics::RigidBody::ConstructionInfo const &info)
	: _name(info.name)
	, _shape(info.shape)
{}
