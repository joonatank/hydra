/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file physics/physics_world.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "physics_world.hpp"

/// Physics objects
#include "motion_state.hpp"
#include "physics_constraints.hpp"
#include "tube.hpp"

#include "base/exceptions.hpp"

// Necessary for creating tubes
#include "game_manager.hpp"

/// Concrete implementation
#include "physics_world_bullet.hpp"

/// -------------------------------- Global ----------------------------------
std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::World const &w)
{
	os << "Physics World : "
		<< " gravity " << w.getGravity()
		<< " with " << w._rigid_bodies.size() << " rigid bodies"
		<< " and " << w._constraints.size() << " constraints"
		<< " and " << w._tubes.size() << " tubes.\n";
	
	/*
	os << "Bodies : \n";
	RigidBodyList::const_iterator iter;
	for( iter = w._rigid_bodies.begin(); iter != w._rigid_bodies.end(); ++iter )
	{
		os << (*iter)->getName();
		if( iter+1 != w._rigid_bodies.end() )
		{ os << "\n"; }
	}
	*/

	return os;
}

/// -------------------------------- Public ----------------------------------
vl::physics::WorldRefPtr
vl::physics::World::create(GameManager *man)
{
	WorldRefPtr world(new BulletWorld);
	world->_game = man;
	return world;
}


vl::physics::World::~World(void)
{}

void
vl::physics::World::removeAll(void)
{
	std::clog << "vl::physics::World::removeAll" << std::endl;
	// Tubes can just be cleared, their bodies and constraints
	// are in the other lists
	// They are first so we don't have dangling pointers
	std::vector<TubeRefPtr> tubes = _tubes;
	for(std::vector<TubeRefPtr>::iterator iter = tubes.begin(); iter != tubes.end(); ++iter)
	{
		removeTube(*iter);
	}

	// Call removeConstraint for all members
	// ConstraintList _constraints;
	ConstraintList constraints = _constraints;
	for(ConstraintList::iterator iter = constraints.begin(); iter != constraints.end(); ++iter)
	{
		removeConstraint(*iter);
	}

	// Call removeRigidBody for all members
	// Make a copy because removing will invalidate iterators
	RigidBodyList bodies = _rigid_bodies;
	for(RigidBodyList::iterator iter = bodies.begin(); iter != bodies.end(); ++iter)
	{
		removeRigidBody(*iter);
	}

	// @todo this also needs to destroy all MotionStates attached to the bodies
	// making the assumption that they would be detached if the user wants
	// to reuse them is much more managable than assuming the user will destroy them
	// especially since python has no memory management system.
	//
	// Of course this can be directly in the destructor of RigidBody
	// we just need to make comments to the relevant sections of the headers
	// rigid_body.hpp, physics_world.hpp, (motion_state.hpp)
	//
	// We could also make MotionStates ref counted, 
	// which might be a better solution in the long run.


	// Reset parameters
	setGravity(Vector3(0, -9.81, 0));
}

void
vl::physics::World::destroyDynamicObjects(void)
{
	std::clog << "vl::physics::World::removeDynamicObjects" << std::endl;

	std::clog << "Destroying " << _tubes.size() << " tubes" << std::endl;
	// Remove all of them for now
	TubeList tubes = _tubes;
	for(TubeList::iterator iter = tubes.begin(); iter != tubes.end(); ++iter)
	{ removeTube(*iter); }

	std::clog << "Removing constraints" << std::endl;
	// @todo constraints need dynamic flags
	ConstraintList constraints_to_destroy;
	for(ConstraintList::iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		if((*iter)->isDynamic())
		{ constraints_to_destroy.push_back(*iter); }
	}
	std::clog << "Destroying " << constraints_to_destroy.size() << " constraints." << std::endl;
	for(ConstraintList::iterator iter = constraints_to_destroy.begin();
		iter != constraints_to_destroy.end(); ++iter)
	{
		removeConstraint(*iter);
	}

	// Destroy Rigid Bodies
	std::clog << "Removing rigid bodies" << std::endl;
	RigidBodyList bodies_to_destroy;
	for(RigidBodyList::iterator iter = _rigid_bodies.begin(); 
		iter != _rigid_bodies.end(); ++iter)
	{
		if((*iter)->isDynamic())
		{ bodies_to_destroy.push_back(*iter); }
	}

	std::clog << "Destroying " << bodies_to_destroy.size() << " rigid bodies." << std::endl;
	for(RigidBodyList::iterator iter = bodies_to_destroy.begin(); 
		iter != bodies_to_destroy.end(); ++iter)
	{
		removeRigidBody(*iter);
	}

	std::clog << "vl::physics::World::removeDynamicObjects : DONE" << std::endl;
}

// @warning added udata as function argument, used purely for raycast testing:
vl::physics::RigidBodyRefPtr
vl::physics::World::createRigidBodyEx(RigidBody::ConstructionInfo const &info)
{
	if(hasRigidBody(info.name))
	{
		std::string err( "RigidBody with that name is already in the scene." );
		BOOST_THROW_EXCEPTION( vl::duplicate() << vl::desc(err) );
	}

	RigidBodyRefPtr body = RigidBody::create(info);
	assert(body);

	_rigid_bodies.push_back(body);
	assert(body->getMotionState() == info.state);
	// Add the body to the physics engine
	_addRigidBody(info.name, body, info.kinematic);

	// Collision detection is auto enabled so disable it if necessary
	body->disableCollisions(!_collision_detection_enabled);

	return body;
}


vl::physics::RigidBodyRefPtr
vl::physics::World::createRigidBody( const std::string& name, vl::scalar mass,
									 vl::physics::MotionState *state,
									 CollisionShapeRefPtr shape,
									 Ogre::Vector3 const &inertia)
{
	RigidBody::ConstructionInfo info(name, mass, state, shape, inertia);

	return createRigidBodyEx(info);
}

vl::physics::RigidBodyRefPtr
vl::physics::World::createDynamicRigidBody(std::string const &name, vl::scalar mass,
								MotionState *state, CollisionShapeRefPtr shape,
								Ogre::Vector3 const &inertia)

{
	std::clog << "vl::physics::World::createDynamicRigidBody" << std::endl;
	RigidBody::ConstructionInfo info(name, mass, state, shape, inertia, false, true);
	return createRigidBodyEx(info);
}

vl::physics::RigidBodyRefPtr
vl::physics::World::getRigidBody( const std::string& name ) const
{
	RigidBodyRefPtr body = _findRigidBody(name);
	if( !body )
	{
		// TODO add a better exception to this
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	return body;
}

vl::physics::RigidBodyRefPtr
vl::physics::World::removeRigidBody( const std::string& name )
{
	RigidBodyRefPtr body = getRigidBody(name);
	removeRigidBody(body);
	return body;
}

void
vl::physics::World::removeRigidBody(vl::physics::RigidBodyRefPtr body)
{
	if(!body)
	{ return; }

	_removeBody(body);

	RigidBodyList::iterator iter = std::find(_rigid_bodies.begin(), _rigid_bodies.end(), body);
	if(iter != _rigid_bodies.end())
	{ _rigid_bodies.erase(iter); }
}

bool
vl::physics::World::hasRigidBody( const std::string& name ) const
{
	return _findRigidBody(name);
}

vl::physics::MotionState *
vl::physics::World::createMotionState(vl::Transform const &trans, vl::ObjectInterface *node)
{
	return MotionState::create(trans, node);
}

void
vl::physics::World::destroyMotionState( vl::physics::MotionState *state )
{
	delete state;
}

void 
vl::physics::World::addConstraint(vl::physics::ConstraintRefPtr constraint, bool disableCollisionBetweenLinked)
{
	ConstraintList::iterator iter = std::find(_constraints.begin(), _constraints.end(), constraint);
	
	if(iter == _constraints.end())
	{
		_constraints.push_back(constraint);
		_addConstraint(constraint, disableCollisionBetweenLinked);
	}
}

void 
vl::physics::World::removeConstraint(vl::physics::ConstraintRefPtr constraint)
{
	ConstraintList::iterator iter = std::find(_constraints.begin(), _constraints.end(), constraint);
	
	if(iter != _constraints.end())
	{
		_removeConstraint(*iter);
		_constraints.erase(iter);
	}
}

bool
vl::physics::World::hasConstraint(vl::physics::ConstraintRefPtr constraint) const
{
	return( std::find(_constraints.begin(), _constraints.end(), constraint) != _constraints.end() );
}

bool
vl::physics::World::hasConstraint(std::string const &name) const
{
	return (getConstraint(name) != ConstraintRefPtr());
}

vl::physics::ConstraintRefPtr
vl::physics::World::getConstraint(std::string const &name) const
{
	for(ConstraintList::const_iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		if((*iter)->getName() == name)
		{
			return *iter;
		}
	}

	return ConstraintRefPtr();
}

/// ----------------------- Tubes --------------------------
vl::physics::TubeRefPtr
vl::physics::World::createTubeEx(vl::physics::Tube::ConstructionInfo const &info)
{
	TubeRefPtr tube(new Tube(this, _game->getSceneManager(), info));
	_tubes.push_back(tube);
	return tube;
}

vl::physics::TubeRefPtr
vl::physics::World::createTube(RigidBodyRefPtr start_body, RigidBodyRefPtr end_body,
		vl::scalar length, vl::scalar radius, vl::scalar mass_per_meter)
{
	Tube::ConstructionInfo info;
	info.start_body = start_body;
	info.end_body = end_body;
	info.length = length;
	info.radius = radius;
	info.mass_per_meter = mass_per_meter;

	return createTubeEx(info);
}

bool
vl::physics::World::hasTube(vl::physics::TubeConstRefPtr tube) const
{
	return(std::find(_tubes.begin(), _tubes.end(), tube) != _tubes.end());
}

void
vl::physics::World::removeTube(vl::physics::TubeRefPtr tube)
{
	if(!tube)
	{ return; }

	std::vector<TubeRefPtr>::iterator iter = std::find(_tubes.begin(), _tubes.end(), tube);

	if(iter != _tubes.end())
	{
		(*iter)->removeFromWorld();
		_tubes.erase(iter);
	}
}

void
vl::physics::World::enableCollisionDetection(bool enable)
{
	if(_collision_detection_enabled != enable)
	{
		/// @todo we should really use a system wide parameter for this
		/// would assume that all physics engine have them
		for(RigidBodyList::iterator iter = _rigid_bodies.begin(); 
			iter != _rigid_bodies.end(); ++iter)
		{
			(*iter)->disableCollisions(!enable);
		}
		_collision_detection_enabled = enable;
	}
}

/// ------------------------------- Protected --------------------------------
vl::physics::World::World(void)
	: _collision_detection_enabled(true)
	, _game(0)
{}

vl::physics::RigidBodyRefPtr
vl::physics::World::_findRigidBody(const std::string& name) const
{
	RigidBodyList::const_iterator iter;
	for( iter = _rigid_bodies.begin(); iter != _rigid_bodies.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{ return *iter; }
	}

	return RigidBodyRefPtr();
}
