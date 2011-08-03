/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-07
 *	@file physics_world_bullet.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Bullet implementation of the physics world
 *
 */

// Interface header
#include "physics_world_bullet.hpp"

#include "physics_constraints_bullet.hpp"
#include "rigid_body_bullet.hpp"

vl::physics::BulletWorld::BulletWorld(void)
	: _broadphase( new btDbvtBroadphase() ),
	  _collision_config( new btDefaultCollisionConfiguration() ),
	  _dispatcher( new btCollisionDispatcher(_collision_config) ),
	  _solver( new btSequentialImpulseConstraintSolver )
{
	_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher,_broadphase,_solver,_collision_config);
	_dynamicsWorld->setGravity( btVector3(0,-9.81,0) );
}

vl::physics::BulletWorld::~BulletWorld(void)
{
	// cleanup the world
	delete _dynamicsWorld;
	delete _solver;
	delete _dispatcher;
	delete _collision_config;
	delete _broadphase;
}

void
vl::physics::BulletWorld::step( void )
{
	// Some hard-coded parameters for the simulation
	_dynamicsWorld->stepSimulation(1/60.f,10);
}

Ogre::Vector3
vl::physics::BulletWorld::getGravity(void) const
{
	return vl::math::convert_vec(_dynamicsWorld->getGravity());
}

void
vl::physics::BulletWorld::setGravity(Ogre::Vector3 const &gravity)
{
	_dynamicsWorld->setGravity( vl::math::convert_bt_vec(gravity) );
}

void
vl::physics::BulletWorld::_addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body)
{
	BulletRigidBodyRefPtr b = boost::dynamic_pointer_cast<BulletRigidBody>(body);

	assert(b && b->getNative());
	_dynamicsWorld->addRigidBody(b->getNative());
}

void
vl::physics::BulletWorld::_addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked)
{
	BulletConstraintRefPtr c = boost::dynamic_pointer_cast<BulletConstraint>(constraint);

	assert(c && c->getNative());
	_dynamicsWorld->addConstraint(c->getNative(), disableCollisionBetweenLinked);
}

void
vl::physics::BulletWorld::_removeConstraint(vl::physics::ConstraintRefPtr constraint)
{
	BulletConstraintRefPtr c = boost::dynamic_pointer_cast<BulletConstraint>(constraint);

	assert(c && c->getNative());
	_dynamicsWorld->removeConstraint(c->getNative());
}
