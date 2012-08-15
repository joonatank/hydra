/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file physics_world_bullet.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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

// Necessary for updating kinematic bodies with collision detection
#include "animation/kinematic_body.hpp"

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
	_dynamicsWorld->stepSimulation((double)time_step, _solver_params.max_sub_steps, _solver_params.internal_time_step);

	// Check for collisions
	// here instead of a tick callback because we only store the transformations
	// once every frame.
	// This function takes about 10us in complex models
	// So it's completely inconsequential.
	_collision_feedback();
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
vl::physics::BulletWorld::_addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body, bool kinematic)
{
	// for some reason we can not do static_pointer_cast here
	BulletRigidBodyRefPtr b = boost::dynamic_pointer_cast<BulletRigidBody>(body);

	assert(b && b->getNative());
	// kinematic objects need to collide with static objects
	if(kinematic)
	{
		// Use group and mask to control collisions
		// we want collisions to other kinematic objects (CharacterFilter)
		// Static objects (StaticFilter)
		// and dynamics objects (DefaultFilter)
		//
		// @todo
		// Disabled kinematic object collisions for now because there are
		// some problems with it.
		short group = btBroadphaseProxy::CharacterFilter;
		short mask = btBroadphaseProxy::DefaultFilter|btBroadphaseProxy::StaticFilter; //|btBroadphaseProxy::CharacterFilter;
		_dynamicsWorld->addRigidBody(b->getNative(), group, mask);
		body->enableKinematicObject(true);
	}
	else
	{ _dynamicsWorld->addRigidBody(b->getNative()); }

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

void
vl::physics::BulletWorld::_collision_feedback(void)
{
	if(!_collision_detection_enabled)
	{ return; }

	//Assume world->stepSimulation or world->performDiscreteCollisionDetection has been called
	
	// @todo this should be replaced with GhostObjects and a custom interface
	// GhostObjects for speed (so we don't need to do the checking for all objects)
	// custom interface so we don't need to do the dirty casting here and we can remove
	// the kinematic body include.

	// @fixme this does work for rigid bodies and kinematic bodies which is
	// exactly what we don't want.
	int numManifolds = _dynamicsWorld->getDispatcher()->getNumManifolds();
	for(int i=0; i < numManifolds; ++i)
	{
		btPersistentManifold* contactManifold =  _dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());

		// We are only interested in collisions between kinematic objects (KinematicBody)
		// and static objects.
		// Static - Static collisions should never happen anyway.
		// Kinematic - Kinematic collisions are disabled using collision flags
		// because there is some problems with them for now.
		//
		// We need to check the collision flags from bt objects because we use them to
		// disable collision in bullet dynamics objects against our kinematic objects.
		bool aColEnabled = !(obA->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);
		bool bColEnabled = !(obB->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);
		if(obA->isStaticOrKinematicObject() && obB->isStaticOrKinematicObject() && aColEnabled && bColEnabled)
		{
			int numContacts = contactManifold->getNumContacts();
			for(int j=0; j < numContacts; ++j)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if(pt.getDistance() < 0.f)
				{
					const btVector3& ptA = pt.getPositionWorldOnA();
					const btVector3& ptB = pt.getPositionWorldOnB();
					const btVector3& normalOnB = pt.m_normalWorldOnB;

					// Update the motion states so that kinematic objects can copy them back
					KinematicBody *bA = (KinematicBody *)obA->getUserPointer();
					KinematicBody *bB = (KinematicBody *)obB->getUserPointer();

					// Only pop the first body transformation if both are kinematic
					// @todo this might need ignoring all collisions between kinematic objects
					// or ignoring them when directly linked.
					// @todo we should have a flag for these that could be set
					// because the simulation needs modifications to either collision models
					// or kinematics and setting collisions flags on objects because of this.
					if( bA && bB)
					{
						// Kinematic-Kinematic collisions

						/// @todo where this checking belongs?
						/// Here because this is the only place where they are both valid
						if(bB->isCollisionsEnabled() && bA->isCollisionsEnabled() )
						{
							// @todo here we need to check if the two parts are joined or not
							// because of collision model inaccuracies joined objects should never
							// collide (directly joined that is).
							/*
							if(!bA->isJoined(bB))
							{
								std::clog << "Collision between non joined bodies " << bA->getName()
									<< " and " << bB->getName() << std::endl;
							}
							*/
						}
					}
					else if(bA && bA->isCollisionsEnabled())
					{	
						bA->popLastTransform();
					}			
					else if(bB && bB->isCollisionsEnabled())
					{
						bB->popLastTransform();
					}
				}
			}
		}
	}
}
