/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_body.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Kinematic body used to interface custom animation framework.
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

namespace vl
{

class KinematicBody : public ObjectInterface
{
public :
	// Normal pointer for world, because it can not go out of scope before
	// the bodies.
	KinematicBody(KinematicWorld *world, animation::NodeRefPtr node, vl::SceneNodePtr sn);

	~KinematicBody(void);

	virtual std::string const &getName(void) const;

	void translate(Ogre::Vector3 const &);

	void translate(Ogre::Real x, Ogre::Real y, Ogre::Real z)
	{ translate(Vector3(x, y, z)); }

	void rotate(Ogre::Quaternion const &);

	void transform(vl::Transform const &t);

	virtual void setWorldTransform(vl::Transform const &trans);

	virtual vl::Transform getWorldTransform(void) const;

	SceneNodePtr getSceneNode(void) const
	{ return _scene_node; }

	/// @todo not implemented
	vl::KinematicBodyRefPtr clone(void) const
	{ return KinematicBodyRefPtr(); }

	/// @todo not implemented
	vl::KinematicBodyRefPtr clone(std::string const &) const
	{ return KinematicBodyRefPtr(); }

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

	// -------------------- Callbacks -----------------------
	int addListener(TransformedCB::slot_type const &slot)
	{ _transformed_cb.connect(slot); return 1; }

	/// @internal
	void _update(void);

private :
	// Copying is forbidden
	KinematicBody(KinematicBody const &);
	KinematicBody &operator=(KinematicBody const &);

	// the creator
	KinematicWorld *_world;

	SceneNodePtr _scene_node;
	animation::NodeRefPtr _node;

	// Some parameters to avoid updating transformation every frame
	bool _dirty_transformation;
	bool _use_dirty;
	bool _disable_updates;
	bool _assume_node_is_in_world;

	// Callbacks
	TransformedCB _transformed_cb;

};	// class KinematicBody

std::ostream &
operator<<(std::ostream &os, KinematicBody const &body);

}	// namespace vl

#endif	// HYDRA_ANIMATION_KINEMATIC_BODY_HPP
