/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_body.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Kinematic body used to interface custom animation framework.
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
}

void
vl::KinematicBody::setWorldTransform(vl::Transform const &trans)
{
	assert(_node);
	_node->setWorldTransform(trans);
}

vl::Transform
vl::KinematicBody::getWorldTransform(void) const
{
	// @todo this should operate on links
	assert(_node);
	return _node->getWorldTransform();
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
