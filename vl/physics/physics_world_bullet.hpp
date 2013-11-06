/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file physics_world_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**
 *	Bullet implementation of the physics world
 *
 */

#ifndef HYDRA_PHYSICS_WORLD_BULLET_HPP
#define HYDRA_PHYSICS_WORLD_BULLET_HPP

// Base class
#include "physics_world.hpp"

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>

// @warning: Needed for dynamic concave raycasting:
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>



namespace vl
{

namespace physics
{

class BulletWorld : public vl::physics::World
{
public :

	/// @brief Constructor
	BulletWorld(void);

	/// @brief Destructor
	virtual ~BulletWorld(void);

	/// Virtual overrides
	virtual void step(vl::time const &time_step);

	virtual Ogre::Vector3 getGravity(void) const;

	virtual void setGravity(Ogre::Vector3 const &gravity);

	virtual void setSolverParameters(SolverParameters const &p);

	virtual SolverParameters const &getSolverParameters(void) const
	{ return _solver_params; }

	virtual RayResult castRay(Ogre::Vector3 const &rayfrom, Ogre::Vector3 const &rayto) const;

protected :
	/// Virtual overrides
	virtual void _addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body, bool kinematic);
	virtual void _addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked);
	virtual void _removeConstraint(vl::physics::ConstraintRefPtr constraint);
	virtual void _removeBody(vl::physics::RigidBodyRefPtr body);

	virtual void _collision_feedback(void);

	vl::physics::RigidBodyRefPtr _findRigidBody(btRigidBody *body) const;
private :
	
	SolverParameters _solver_params;

	/// Bullet physics world objects
	/// The order of them is important don't change it.
	/// @todo move to using scoped ptrs if possible
	btBroadphaseInterface *_broadphase;
	btCollisionConfiguration *_collision_config;
	btCollisionDispatcher *_dispatcher;
	btSequentialImpulseConstraintSolver *_solver;
	btDiscreteDynamicsWorld *_dynamicsWorld;
	
};	// class BulletWorld

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_WORLD_BULLET_HPP
