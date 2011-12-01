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
	if(!body.getSceneNode())
	{ os << " without a SceneNode!"; }
	else
	{ os << " with " << body.getSceneNode()->getID() << " : " << body.getSceneNode()->getName(); }
	
	os << " and ";
	if(!body.getAnimationNode())
	{ os << " without an Animation Node!"; }
	else 
	{ os << *body.getAnimationNode(); }
	
	os<< std::endl;

	return os;
}


/// ------------------------------ KinematicBody -----------------------------
vl::KinematicBody::KinematicBody(KinematicWorld *world, 
	animation::NodeRefPtr node, vl::SceneNodePtr sn)
	: _world(world)
	, _scene_node(sn)
	, _node(node)
	, _dirty_transformation(true)
	, _use_dirty(false)
	, _disable_updates(false)
	, _assume_node_is_in_world(false)
{
	assert(_world);
	assert(_scene_node);
	assert(_node);

	// As the Node is not initialised yet we need to initialise
	// its transformation
	_node->setWorldTransform(_scene_node->getWorldTransform());
}


vl::KinematicBody::~KinematicBody(void)
{
}

std::string const &
vl::KinematicBody::getName(void) const
{
	return _scene_node->getName();
}

void
vl::KinematicBody::translate(Ogre::Vector3 const &v)
{
	Transform t(v);
	transform(t);
}

void
vl::KinematicBody::rotate(Ogre::Quaternion const &q)
{
	Transform t(q);
	transform(t);
}

void
vl::KinematicBody::transform(vl::Transform const &t)
{
	_node->setTransform(_node->getTransform()*t);
	_transformed_cb(_node->getWorldTransform());
}

void
vl::KinematicBody::setWorldTransform(vl::Transform const &trans)
{
	assert(_node);
	_dirty_transformation = true;
	_node->setWorldTransform(trans);
	_transformed_cb(trans);
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
	if(_scene_node)
	{ _scene_node->setVisibility(visible); }
}

bool
vl::KinematicBody::isVisible(void) const
{
	if(_scene_node)
	{ return _scene_node->isVisible(); }
	return true;
}

vl::SceneNodePtr
vl::KinematicBody::getSceneNode(void) const
{
	return _scene_node;
}


void
vl::KinematicBody::_update(void)
{
	assert(_scene_node && _node);

	Transform wt = _node->getWorldTransform();
	if(_scene_node->getWorldTransform() != wt)
	{
		_scene_node->setWorldTransform(wt);
		_transformed_cb(wt);
	}
}
