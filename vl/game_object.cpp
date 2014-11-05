/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file game_object.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "game_object.hpp"

#include "game_manager.hpp"

// Necessary for managing kinematic body
#include "animation/kinematic_body.hpp"
#include "animation/kinematic_world.hpp"

// Necessary for managing rigid body
#include "physics/rigid_body.hpp"
#include "physics/physics_world.hpp"
// Necessary for collision detection
#include "physics/shapes.hpp"
#include "entity.hpp"

// Necessary for managing scene node
#include "scene_manager.hpp"
#include "scene_node.hpp"

/// ----------------------------- Global -------------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::GameObject const &obj)
{
	os << "GameObject : " << obj.getName();
	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, GameObjectList const &list)
{
	for(GameObjectList::const_iterator iter = list.begin();
		iter != list.end(); ++iter)
	{
		os << *(*iter) << std::endl;
	}

	return os;
}


/// -------------------------- GameObject ------------------------------------
/// ----------------------------- Public -------------------------------------
vl::GameObject::GameObject(std::string const &name, GameManager *manager, bool dynamic)
	: _name(name)
	, _graphics_node(0)
	, _is_dynamic(dynamic)
	, _creator(manager)
{
	_graphics_node = _creator->getSceneManager()->createSceneNode(_name);
	assert(_graphics_node);
}

vl::GameObject::~GameObject(void)
{
	// @todo remove scene node
	// @todo remove kinematic object
	// @todo remove rigid body
}

void
vl::GameObject::createRigidBody(vl::scalar mass, Ogre::Vector3 const &inertia)
{
	if(_rigid_body)
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Rigid body already exists") << vl::name(_name));
	}

	assert(_creator);
	assert(_graphics_node);
	assert(_creator->getPhysicsWorld());
	if(!_collision_shape)
	{ _createCollisionShape(); }

	physics::MotionState *ms = _creator->getPhysicsWorld()->createMotionState(getWorldTransform(), _graphics_node);
	_rigid_body = _creator->getPhysicsWorld()->createRigidBody(_name, mass, ms, _collision_shape, inertia);
	

}

void
vl::GameObject::enableCollisionDetection(bool enable)
{
	if(enable)
	{
		// Create a kinematic collision body if dynamics are not enabled for this object
		if(!_rigid_body)
		{
			assert(_creator->getPhysicsWorld());
			//physics::MotionState *state = _creator->getPhysicsWorld()->createMotionState();
			if(!_collision_shape)
			{ _createCollisionShape(); }

			physics::RigidBody::ConstructionInfo info(_name, 0, _kinematic_body->getMotionState(), _collision_shape, Ogre::Vector3(0, 0, 0), true);

			_rigid_body = _creator->getPhysicsWorld()->createRigidBodyEx(info);
			
			//_rigid_body->enableKinematicObject(true);
					
			if(_kinematic_body)
			{
				// necessary to add callback so the kinematic object updates 
				// the the collision model.
				_kinematic_body->addListener(boost::bind(&physics::RigidBody::setWorldTransform, _rigid_body, _1));
				// Used by the collision detection to pop last transformation.
				_rigid_body->setUserData(_kinematic_body.get());
			}
		}
	}
	
	if(_rigid_body)
	{
		_rigid_body->disableCollisions(!enable);
	}
}

bool
vl::GameObject::isCollisionDetectionEnabled(void) const
{
	if(_rigid_body)
	{ return !_rigid_body->isCollisionsDisabled(); }

	return false;
}

void
vl::GameObject::setKinematic(bool kinematic)
{
	if(kinematic)
	{
		// Create the kinematic object if one does not already exists
		if(!_kinematic_body)
		{
			assert(_creator->getKinematicWorld());
			assert(_graphics_node);
			_kinematic_body = _creator->getKinematicWorld()->createKinematicBody(_graphics_node);
		}
	}
	else
	{
		// Destroy the kinematic object if it already exists
		if(_kinematic_body)
		{
			assert(_creator->getKinematicWorld());
			_creator->getKinematicWorld()->removeKinematicBody(_kinematic_body);
			_kinematic_body.reset();
			if(_rigid_body)
			{
				// @todo should remove listener also
				_rigid_body->setUserData(0);
			}
		}
	}

	// Set rigid body to kinematic
	if(_rigid_body && kinematic)
	{
		assert(_kinematic_body);
		// Hack to avoid adding the same listener multiple times
		if(_kinematic_body.get() != _rigid_body->getUserData())
		{
			_rigid_body->enableKinematicObject(true);
			// necessary to add callback so the kinematic object updates 
			// the the collision model.
			// @todo this is problematic if the kinematic body already has a listener
			// especially if it's the same rigid body
			// The if hack above should take care of it, but it's rather dirty
			_kinematic_body->addListener(boost::bind(&physics::RigidBody::setWorldTransform, _rigid_body, _1));
			// Used by the collision detection to pop last transformation.
			_rigid_body->setUserData(_kinematic_body.get());
		}
	}
}

bool
vl::GameObject::isKinematic(void) const
{
	return _kinematic_body;
}

void
vl::GameObject::setCollisionModel(vl::MeshRefPtr mesh)
{
	setCollisionModel(physics::ConvexHullShape::create(mesh));
}

void
vl::GameObject::setCollisionModel(vl::physics::CollisionShapeRefPtr shape)
{
	if(_rigid_body)
	{
		std::clog << "vl::GameObject::setCollisionModel : " 
			<< "changing collision shape is not yet supported." << std::endl;
		return;
	}

	_collision_shape = shape;
}

vl::physics::CollisionShapeRefPtr
vl::GameObject::getCollisionModel(void) const
{
	return _collision_shape;
}

vl::Transform const &
vl::GameObject::getTransform(void) const
{
	// @todo for some reason if we return the kinematic body transformation
	// here we get really large numbers.
	/*
	if(_kinematic_body)
	{
		return _kinematic_body->getWorldTransform();
	}
	else if(_rigid_body)
	{
		return _rigid_body->getWorldTransform();
	}
	else
		*/
	{
		assert(_graphics_node);
		return _graphics_node->getTransform();
	}
}

void
vl::GameObject::setTransform(vl::Transform const &t)
{
	if(_kinematic_body)
	{
		_kinematic_body->setWorldTransform(t);
	}
	else if(_rigid_body)
	{
		_rigid_body->setWorldTransform(t);
	}
	else
	{
		assert(_graphics_node);
		_graphics_node->setTransform(t);
	}
}

void
vl::GameObject::setWorldTransform(vl::Transform const &trans)
{
	if(_kinematic_body)
	{
		_kinematic_body->setWorldTransform(trans);
	}
	else if(_rigid_body)
	{
		_rigid_body->setWorldTransform(trans);
	}
	else
	{
		assert(_graphics_node);
		_graphics_node->setWorldTransform(trans);
	}
}

vl::Transform
vl::GameObject::getWorldTransform(void) const
{
	if(_kinematic_body)
	{
		return _kinematic_body->getWorldTransform();
	}
	else if(_rigid_body)
	{
		return _rigid_body->getWorldTransform();
	}
	else
	{
		assert(_graphics_node);
		return _graphics_node->getWorldTransform();
	}
}

void
vl::GameObject::setVisibility(bool visible)
{
	assert(_graphics_node);
	return _graphics_node->setVisibility(visible);
}

bool
vl::GameObject::isVisible(void) const
{
	assert(_graphics_node);
	return _graphics_node->isVisible();
}

int
vl::GameObject::addListener(TransformedCB::slot_type const &slot)
{
	// @todo implement
	return -1;
}

void
vl::GameObject::addChild(GameObjectRefPtr child)
{
	std::clog << "NOT implemented : vl::GameObject::addChild" << std::endl;
	// @todo implement
	// weird function in every way though
}

void
vl::GameObject::_createCollisionShape(void)
{
	assert(_graphics_node);
	vl::MovableObjectList const &objs = _graphics_node->getObjects();
	for(vl::MovableObjectList::const_iterator iter = objs.begin(); 
		iter != objs.end(); ++ iter)
	{
		// Grap the first entity in the list for collision shape
		// this can handle most of the cases anyway.
		if((*iter)->getTypeName() == "Entity")
		{
			vl::Entity *ent = static_cast<vl::Entity *>(*iter);
			vl::MeshRefPtr mesh = ent->getMesh();
			assert(mesh);
			_collision_shape = physics::ConvexHullShape::create(mesh);
		}
	}
}
