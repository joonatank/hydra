/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_body.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*	Kinematic body used to interface custom animation framework.
 *	Interface is similar to SceneNode and RigidBody but works on animation
 *	nodes and links instead.
 */

#include "kinematic_body.hpp"

#include "scene_node.hpp"

#include <iostream>

/// ---------------------------------- Global --------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::KinematicBody const &body)
{
	os << "KinematicBody : " << body.getName();
	if(!body.getMotionState())
	{ os << " without a MotionState!"; }
	
	os << " and ";
	if(!body.getAnimationNode())
	{ os << " without an Animation Node!"; }
	else 
	{ os << *body.getAnimationNode(); }
	
	os<< std::endl;

	return os;
}


/// ------------------------------ KinematicBody -----------------------------
vl::KinematicBody::KinematicBody(std::string const &name, KinematicWorld *world, 
	animation::NodeRefPtr node, vl::physics::MotionState *ms)
	: _world(world)
	, _state(ms)
	, _node(node)
	, _name(name)
	, _dirty_transformation(true)
	, _use_dirty(false)
	, _disable_updates(false)
	, _assume_node_is_in_world(false)
{
	assert(_world);
	assert(_state);
	assert(_node);
}


vl::KinematicBody::~KinematicBody(void)
{
}

void
vl::KinematicBody::translate(Ogre::Vector3 const &v)
{
	transform(Transform(v));
}

void
vl::KinematicBody::rotate(Ogre::Quaternion const &q)
{
	transform(Transform(q));
}

void
vl::KinematicBody::transform(vl::Transform const &t)
{
	_node->setTransform(_node->getTransform()*t);
//	_transformed_cb(_node->getWorldTransform());
}

void
vl::KinematicBody::setWorldTransform(vl::Transform const &trans)
{
	assert(_node);
	_dirty_transformation = true;
	_node->setWorldTransform(trans);
//	_transformed_cb(trans);
}

vl::Transform
vl::KinematicBody::getWorldTransform(void) const
{
	assert(_node);
	return _node->getWorldTransform();
}

void
vl::KinematicBody::setVisibility(bool visible)
{
	if(getSceneNode())
	{ getSceneNode()->setVisibility(visible); }
}

bool
vl::KinematicBody::isVisible(void) const
{
	if(getSceneNode())
	{ return getSceneNode()->isVisible(); }
	
	return true;
}

vl::SceneNodePtr
vl::KinematicBody::getSceneNode(void) const
{
	// dirty hack to handle the necessity of having this function
	return (SceneNodePtr)_state->getNode();
}


void
vl::KinematicBody::_update(void)
{
	assert(_state && _node && _state->getNode());

	Transform wt = _node->getWorldTransform();
	if(_state->getWorldTransform() != wt)
	{
		_state->setWorldTransform(wt);
		_transformed_cb(wt);
	}
}

void
vl::KinematicBody::popLastTransform(void)
{
	_node->popLastTransform();
}
