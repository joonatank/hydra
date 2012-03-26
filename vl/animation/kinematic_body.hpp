/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_body.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**	Kinematic body used to interface custom animation framework.
 *	Interface is similar to SceneNode and RigidBody but works on animation
 *	nodes and links instead.
 */

#ifndef HYDRA_ANIMATION_KINEMATIC_BODY_HPP
#define HYDRA_ANIMATION_KINEMATIC_BODY_HPP

// Abstract interface
#include "object_interface.hpp"

#include "typedefs.hpp"

#include <string>
#include <vector>

#include "animation.hpp"

#include "physics/motion_state.hpp"

namespace vl
{

class KinematicBody : public ObjectInterface
{
public :
	// Normal pointer for world, because it can not go out of scope before
	// the bodies.
	KinematicBody(std::string const &name, KinematicWorld *world, animation::NodeRefPtr node, vl::physics::MotionState *ms);

	virtual ~KinematicBody(void);

	virtual std::string const &getName(void) const
	{ return _name; }

	void translate(Ogre::Vector3 const &);

	void rotate(Ogre::Quaternion const &);

	void transform(vl::Transform const &t);

	virtual void setWorldTransform(vl::Transform const &trans);

	virtual vl::Transform getWorldTransform(void) const;

	virtual Ogre::Vector3 const &getPosition(void) const
	{ return _node->getWorldTransform().position; }

	virtual void setPosition(Ogre::Vector3 const &v)
	{
		Transform t = _node->getWorldTransform();
		t.position = v;
		_node->setWorldTransform(t);
	}

	virtual Ogre::Quaternion const &getOrientation(void) const
	{ return _node->getWorldTransform().quaternion; }

	virtual void setOrientation(Ogre::Quaternion const &q)
	{
		Transform t = _node->getWorldTransform();
		t.quaternion = q;
		_node->setWorldTransform(t);
	}

	vl::Transform transformToLocal(vl::Transform const &t) const
	{
		vl::Transform wtB(getWorldTransform());
		wtB.invert();
		return wtB*t;
	}

	virtual void setVisibility(bool visible);

	virtual bool isVisible(void) const;

	/// @todo not implemented
	vl::KinematicBodyRefPtr clone(void) const
	{ return KinematicBodyRefPtr(); }

	/// @todo not implemented
	vl::KinematicBodyRefPtr clone(std::string const &) const
	{ return KinematicBodyRefPtr(); }

	// -------------------- Callbacks -----------------------
	int addListener(TransformedCB::slot_type const &slot)
	{ _transformed_cb.connect(slot); return 1; }

	/// @depricated
	vl::SceneNodePtr vl::KinematicBody::getSceneNode(void) const;
	
	physics::MotionState *getMotionState(void) const
	{ return _state; }

	// @todo do we need this?
	// or can we use animation::Link for all purposes necessary?
	std::vector<Constraint *> const &getConstraints(void)
	{ return std::vector<Constraint *>(); }

	animation::NodeRefPtr getAnimationNode(void) const
	{ return _node; }

	/// Update parameters
	/// default values use lots of resources (prosessing time) 
	/// but should work in every situtation
	/// Setting some or all of these on should increase performance
	
	/// @brief disable updates for this body (useful if it's hidden and not used)
	void setDisableUpdate(bool disable)
	{ _disable_updates = disable; }

	bool isDisableUpdate(void) const
	{ return _disable_updates; }

	/// @brief use transformation dirties and only update if setTransformation is called
	void setUseDirties(bool use)
	{ _use_dirty = use; }

	bool isUseDirties(void) const
	{ return _use_dirty; }

	/// @brief assume that the node we control is already in world frame
	void setAssumeInWorld(bool enable)
	{ _assume_node_is_in_world = enable; }

	bool isAssumeInWorld(void) const
	{ return _assume_node_is_in_world; }

	/// @internal
	void _update(void);

	/// @brief for collision detection returns the last transformation
	/// this assumes that the problem was the kinematic object
	/// so the collision detection system needs to check that the collision 
	/// was against a static object or another kinematic object.
	void popLastTransform(void);

	void enableCollisions(bool enable)
	{ _collisions_enabled = enable; }

	bool isCollisionsEnabled(void) const
	{ return _collisions_enabled; }

private :
	// Copying is forbidden
	KinematicBody(KinematicBody const &);
	KinematicBody &operator=(KinematicBody const &);

	std::string _name;

	// the creator
	KinematicWorld *_world;

	vl::physics::MotionState *_state;
	animation::NodeRefPtr _node;

	// Some parameters to avoid updating transformation every frame
	bool _dirty_transformation;
	bool _use_dirty;
	bool _disable_updates;
	bool _assume_node_is_in_world;

	bool _collisions_enabled;

	// Callbacks
	TransformedCB _transformed_cb;

};	// class KinematicBody

std::ostream &
operator<<(std::ostream &os, KinematicBody const &body);

}	// namespace vl

#endif	// HYDRA_ANIMATION_KINEMATIC_BODY_HPP
