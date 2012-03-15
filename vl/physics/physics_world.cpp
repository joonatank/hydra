/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file physics/physics_world.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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

/// Concrete implementations
#ifdef USE_BULLET
#include "physics_world_bullet.hpp"
#else if USE_NEWTON
#include "physics_world_newton.hpp"
#endif

/// -------------------------------- Global ----------------------------------
std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::World const &w)
{
	os << "Physics World : "
		<< " gravity " << w.getGravity()
		<< " with " << w._rigid_bodies.size() << " rigid bodies "
		<< "\n";
	
	os << "Bodies : \n";
	RigidBodyList::const_iterator iter;
	for( iter = w._rigid_bodies.begin(); iter != w._rigid_bodies.end(); ++iter )
	{
		os << (*iter)->getName();
		if( iter+1 != w._rigid_bodies.end() )
		{ os << "\n"; }
	}

	return os;
}

/// -------------------------------- Public ----------------------------------
vl::physics::WorldRefPtr
vl::physics::World::create(GameManager *man)
{
	WorldRefPtr world;
#ifdef USE_BULLET
	world.reset(new BulletWorld);
#else if USE_NEWTON
#endif
	world->_game = man;
	return world;
}


vl::physics::World::~World(void)
{}


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
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
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
