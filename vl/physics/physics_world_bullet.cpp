/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file physics_world_bullet.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*
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
	// @todo for some reason normal gravity will break the constraints
	// would guess that the impulse for the constraints is too small to work against the gravity
	// fix the gravity after finding a real solution.
	// This is a problem only with small objects, 
	// using a high internal timestep like 1/240 or 1/480
	// will fix this problem with a performance hit.
	_dynamicsWorld->setGravity( btVector3(0,-9.81,0) );

	std::clog << "Original solver parameters : " << std::endl
		<< "\t" << " restitution " << _dynamicsWorld->getSolverInfo().m_restitution << std::endl
		<< "\t" << " max error reduction " << _dynamicsWorld->getSolverInfo().m_maxErrorReduction << std::endl
		<< "\t" << " ERP " << _dynamicsWorld->getSolverInfo().m_erp << std::endl
		<< "\t" << " ERP2 " << _dynamicsWorld->getSolverInfo().m_erp2 << std::endl
		<< "\t" << " global CFM " << _dynamicsWorld->getSolverInfo().m_globalCfm << std::endl;
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
vl::physics::BulletWorld::step(vl::time const &time_step)
{
	// Some hard-coded parameters for the simulation
	// Small internal timestep is necessary for small objects < 100mm
	_dynamicsWorld->stepSimulation((double)time_step, _solver_params.max_sub_steps, _solver_params.internal_time_step);
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
vl::physics::BulletWorld::setSolverParameters(vl::physics::SolverParameters const &p)
{
	_solver_params = p;
	// set global parameters
	_dynamicsWorld->getSolverInfo().m_restitution = p.restitution;
	_dynamicsWorld->getSolverInfo().m_maxErrorReduction = p.max_error_reduction;
	// don't change erp for now, don't know what values it takes
	// using 1e-5 - 1e-4 will cause all bodies fall slowly.
	_dynamicsWorld->getSolverInfo().m_erp = p.erp;
	_dynamicsWorld->getSolverInfo().m_erp2 = p.erp2;
	_dynamicsWorld->getSolverInfo().m_globalCfm = p.global_cfm;
}

void
vl::physics::BulletWorld::_addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body)
{
	// for some reason we can not do static_pointer_cast here
	BulletRigidBodyRefPtr b = boost::dynamic_pointer_cast<BulletRigidBody>(body);

	assert(b && b->getNative());
	_dynamicsWorld->addRigidBody(b->getNative());
}

void
vl::physics::BulletWorld::_addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked)
{
	// for some reason we can not do static_pointer_cast here
	BulletConstraintRefPtr c = boost::dynamic_pointer_cast<BulletConstraint>(constraint);

	assert(c && c->getNative());
	_dynamicsWorld->addConstraint(c->getNative(), disableCollisionBetweenLinked);
}

void
vl::physics::BulletWorld::_removeConstraint(vl::physics::ConstraintRefPtr constraint)
{
	// for some reason we can not do static_pointer_cast here
	BulletConstraintRefPtr c = boost::dynamic_pointer_cast<BulletConstraint>(constraint);

	assert(c && c->getNative());
	_dynamicsWorld->removeConstraint(c->getNative());
}
