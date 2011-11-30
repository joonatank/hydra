/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file physics_world_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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

protected :
	/// Virtual overrides
	virtual void _addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body);
	virtual void _addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked);
	virtual void _removeConstraint(vl::physics::ConstraintRefPtr constraint);

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
