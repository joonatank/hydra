/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-07
 *	@file physics_world_bullet.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
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
	virtual void step(void);

	virtual Ogre::Vector3 getGravity(void) const;

	virtual void setGravity(Ogre::Vector3 const &gravity);

protected :
	/// Virtual overrides
	virtual void _addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body);
	virtual void _addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked);
	virtual void _removeConstraint(vl::physics::ConstraintRefPtr constraint);

private :
	
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
