/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file rigid_body.cpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#include "rigid_body.hpp"

vl::physics::RigidBody::RigidBody(vl::physics::RigidBody::ConstructionInfo const &constructionInfo)
	: _name(constructionInfo.name), _shape(constructionInfo.shape), _bt_body(0)
{
	_bt_body = new btRigidBody(constructionInfo.getBullet());
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
vl::physics::RigidBody::applyCentralForce(Ogre::Vector3 const &force, vl::SceneNodePtr ref)
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

void
vl::physics::RigidBody::setMass(Ogre::Real mass)
{
	if(!_bt_body->getInvInertiaDiagLocal().isZero())
	{
		btVector3 inv = _bt_body->getInvInertiaDiagLocal();
		btVector3 inertia(1/inv.x(), 1/inv.y(), 1/inv.z());
		_bt_body->setMassProps(mass, inertia);
	}
}

vl::Transform
vl::physics::RigidBody::transformToLocal(vl::Transform const &t) const
{
	MotionState *ms = (MotionState *)_bt_body->getMotionState();
	vl::Transform from_world = ms->getWorldTransform();
	from_world.invert();
	return from_world*t;
}


Ogre::Vector3
vl::physics::RigidBody::positionToLocal(Ogre::Vector3 const &v) const
{
	MotionState *ms = (MotionState *)_bt_body->getMotionState();
	vl::Transform from_world = ms->getWorldTransform();
	from_world.invert();
	return from_world*v;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::RigidBody const &body)
{
	os << "RigidBody " << body.getName() << " : motion state " << *body.getMotionState();
	if( body.isUserControlled() )
	{ os << " : user controlled"; }

	// TODO add the rest

	return os;
}
