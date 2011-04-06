/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *
 *	Bullet Physics World used to initialise the physics with some default
 *	values. Later they might be controllable by the user.
 *
 *	Also contains mixed physics classes for now.
 */

#ifndef VL_PHYSICS_WORLD_HPP
#define VL_PHYSICS_WORLD_HPP

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>

#include <vector>
#include <string>

#include "base/exceptions.hpp"

#include "scene_node.hpp"

/// Physics objects
#include "motion_state.hpp"
#include "rigid_body.hpp"

namespace vl
{

namespace physics
{

/** @class World
 *
 */
class World
{
public :
	World( void );

	~World( void );

	void step( void );

	Ogre::Vector3 getGravity( void );

	void setGravity( Ogre::Vector3 const &gravity );

	/// @TODO replace name, when you have the time to fix the overloads for python
	vl::physics::RigidBody *createRigidBodyEx( std::string const &name,
											 btRigidBody::btRigidBodyConstructionInfo const &info );

	vl::physics::RigidBody *createRigidBody( std::string const &name, vl::scalar mass,
								  vl::physics::MotionState *state, btCollisionShape *shape,
								  Ogre::Vector3 const &inertia = Ogre::Vector3(1, 1, 1) );

	/// @todo should be removed as they should be created using createRigidBody
	void addRigidBody( std::string const &name, vl::physics::RigidBody *body);

	vl::physics::RigidBody *getRigidBody( std::string const &name );

	/// @TODO implement
	/// Should this remove the body from the world and return a pointer or
	/// should it just destroy the body altogether
	/// if this returns a shared_ptr it's not a problem at all
	vl::physics::RigidBody *removeRigidBody( std::string const &name );

	bool hasRigidBody( std::string const &name );

	MotionState *createMotionState( vl::Transform const &trans, vl::SceneNode *node = 0 );

	void destroyMotionState( MotionState *state );

	btStaticPlaneShape *createPlaneShape( Ogre::Vector3 const &normal, vl::scalar constant );

	btSphereShape *createSphereShape( vl::scalar radius );

	void destroyShape( btCollisionShape *shape );

private :
	RigidBody *_findRigidBody( std::string const &name );

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
	/// @todo move to using shared_ptrs
	std::vector<RigidBody *> _rigid_bodies;

};	// class World

}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_EVENTS_HPP

