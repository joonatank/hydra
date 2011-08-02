/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *
 *	Physics World used to initialise the physics with some default
 *	values. Later they might be controllable by the user.
 *
 *	Uses Bullet for now
 *	Later this will provide object management for the general interface
 *	objects the abstract inteface for physics world.
 *
 */

#ifndef HYDRA_PHYSICS_WORLD_HPP
#define HYDRA_PHYSICS_WORLD_HPP

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>

#include <vector>
#include <string>

#include "base/exceptions.hpp"

#include "scene_node.hpp"

/// Physics objects
#include "physics_constraints.hpp"

namespace vl
{

namespace physics
{

typedef std::vector<RigidBodyRefPtr> RigidBodyList;
typedef std::vector<btCollisionShape *> CollisionShapeList;
typedef std::vector<ConstraintRefPtr> ConstraintList;

/** @class World
 *
 */
class World
{
public :
	World( void );

	~World( void );

	void step( void );

	Ogre::Vector3 getGravity( void ) const;

	void setGravity( Ogre::Vector3 const &gravity );

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

	MotionState *createMotionState( vl::Transform const &trans = vl::Transform(), vl::SceneNode *node = 0 );

	void destroyMotionState(vl::physics::MotionState *state);

	/// @brief add a constraint i.e. a joint to the world
	/// @param constraint the constraint to add to the world
	/// @param disableCollisionBetweenLinked no collision detection between linked rigid bodies
	/// The constraint is only active if it's added to the current world.
	/// Otherwise it's created and defined but not used in the calculations.
	/// @todo should this throw if the constraint already exists?
	void addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked = false);

	void removeConstraint(vl::physics::ConstraintRefPtr constraint);

	friend std::ostream &operator<<(std::ostream &os, World const &w);

private :
	void _addRigidBody( std::string const &name, vl::physics::RigidBodyRefPtr body);

	RigidBodyRefPtr _findRigidBody( std::string const &name ) const;

	/// Bullet physics world objects
	/// The order of them is important don't change it.
	/// @todo move to using scoped ptrs if possible
	btBroadphaseInterface *_broadphase;
	btCollisionConfiguration *_collision_config;
	btCollisionDispatcher *_dispatcher;
	btSequentialImpulseConstraintSolver *_solver;
	btDiscreteDynamicsWorld *_dynamicsWorld;

	/// Rigid bodies
	/// World owns all of them
	RigidBodyList _rigid_bodies;
	ConstraintList _constraints;

};	// class World

std::ostream &operator<<(std::ostream &os, World const &w);

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_WORLD_HPP
