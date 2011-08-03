/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *	@file physics/physics_world.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Physics World used to initialise the physics with some default
 *	values. Later they might be controllable by the user.
 *	Provides object management for and the general interface for physics engine world.
 *
 *	@update 2011-07
 *	Removed dependencies to Bullet for selectable physics engine.
 *
 */

#ifndef HYDRA_PHYSICS_WORLD_HPP
#define HYDRA_PHYSICS_WORLD_HPP

#include "typedefs.hpp"
// Necessary for time step
#include "base/timer.hpp"
// Necessary for vl::scalar
#include "math/types.hpp"
// Necessary for RigidBody::ConstructionInfo
#include "rigid_body.hpp"

#include <vector>
#include <string>

namespace vl
{

namespace physics
{

/** @class World
 *	Interface for physics world, provides concrete implementations of object
 *	management using our wrapper objects.
 *	Provides abstract interface for physics engine implementation.
 */
class World
{
public :
	static WorldRefPtr create(void);

	virtual ~World(void);

	virtual void step(vl::time const &time_step) = 0;

	virtual Ogre::Vector3 getGravity(void) const = 0;

	virtual void setGravity(Ogre::Vector3 const &gravity) = 0;

	/// ---------------------- RigidBodies ------------------
	/// @TODO replace name, when you have the time to fix the overloads for python
	vl::physics::RigidBodyRefPtr createRigidBodyEx(RigidBody::ConstructionInfo const &info);

	/// Default inertia is zero because setting it will mess up static objects. 
	/// For dynamic objects user should set it.
	/// Automatically adds the RigidBody to the world.
	vl::physics::RigidBodyRefPtr createRigidBody( std::string const &name, vl::scalar mass,
								  vl::physics::MotionState *state, CollisionShapeRefPtr shape,
								  Ogre::Vector3 const &inertia = Ogre::Vector3(0, 0, 0) );

	vl::physics::RigidBodyRefPtr getRigidBody( std::string const &name ) const;

	/// @TODO implement
	/// Should this remove the body from the world and return a pointer or
	/// should it just destroy the body altogether
	/// if this returns a shared_ptr it's not a problem at all
	vl::physics::RigidBodyRefPtr removeRigidBody( std::string const &name );

	bool hasRigidBody( std::string const &name ) const;


	/// ---------------------- MotionStates ------------------
	MotionState *createMotionState(vl::Transform const &trans = vl::Transform(), vl::SceneNodePtr node = 0);

	void destroyMotionState(vl::physics::MotionState *state);

	/// ---------------------- Constraints ------------------
	/// @brief add a constraint i.e. a joint to the world
	/// @param constraint the constraint to add to the world
	/// @param disableCollisionBetweenLinked no collision detection between linked rigid bodies
	/// The constraint is only active if it's added to the current world.
	/// Otherwise it's created and defined but not used in the calculations.
	/// @todo should this throw if the constraint already exists?
	void addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked = false);

	void removeConstraint(vl::physics::ConstraintRefPtr constraint);


	/// ----------------------- Tubes --------------------------
	TubeRefPtr createTube(RigidBodyRefPtr start_body, RigidBodyRefPtr end_body,
		vl::scalar length, vl::scalar radius = 0.1, vl::scalar mass = 50.0);


	friend std::ostream &operator<<(std::ostream &os, World const &w);

protected :

	// Protected because this is abstract class
	World(void);

	// Real engine implementation using template method pattern
	virtual void _addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked) = 0;
	virtual void _addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body) = 0;

	virtual void _removeConstraint(vl::physics::ConstraintRefPtr constraint) = 0;


	RigidBodyRefPtr _findRigidBody( std::string const &name ) const;

	/// Rigid bodies
	/// World owns all of them
	RigidBodyList _rigid_bodies;
	ConstraintList _constraints;

};	// class World

std::ostream &operator<<(std::ostream &os, World const &w);

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_WORLD_HPP
