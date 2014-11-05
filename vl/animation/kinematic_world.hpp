/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_world.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	World object that deals with Kinematic bodies and constraints.
 *	Provides the same interface as physics world but using our own
 *	kinematic solver.
 */

#ifndef HYDRA_ANIMATION_KINEMATIC_WORLD_HPP
#define HYDRA_ANIMATION_KINEMATIC_WORLD_HPP

#include "base/time.hpp"

#include "typedefs.hpp"

#include "math/transform.hpp"

#include "animation.hpp"

#include <iostream>

namespace vl
{

class HYDRA_API KinematicWorld
{
public :
	KinematicWorld(GameManager *man);

	~KinematicWorld(void);

	/// @todo we need to use internal timer similar to the one Bullet uses
	/// this will fix issues in Hinge with high FPS
	/// Clamps to 120Hz (or something else) and this is nop if we are called before.
	void step(vl::time const &t);

	void finalise(void);

	/// @brief Remove everything from the world
	void removeAll(void);

	/// @brief remove at run time created objects
	void destroyDynamicObjects(void);

	KinematicBodyRefPtr getKinematicBody(std::string const &name) const;

	/// @brief retrieve already created by for the SceneNode
	/// @param sn the scene node which the body controls
	/// @return valid body if one already exists, None if not
	KinematicBodyRefPtr getKinematicBody(vl::SceneNodePtr sn) const;

	/// @brief retrieve already existing body for the SceneNode or create a new
	/// @param sn the scene node which this body controls
	/// @return always valid body node, new one if none exists
	KinematicBodyRefPtr createKinematicBody(vl::SceneNodePtr sn);

	KinematicBodyRefPtr createDynamicKinematicBody(vl::SceneNodePtr sn);

	void removeKinematicBody(KinematicBodyRefPtr body);

	/// -------------- Constraints -----------------------
	/// @todo createConstraint should be changed to creating the constraint
	/// without adding it to the world, this allows us to add/remove constraints
	/// without destroying them. This will break compatibility with older
	/// python scripts so it's not changed yet.

	ConstraintRefPtr createConstraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, vl::Transform const &trans);

	ConstraintRefPtr createConstraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, 
		vl::Transform const &frameInA, vl::Transform const &frameInB, std::string const &name);

	// For backwards compatibility
	ConstraintRefPtr createConstraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, 
		vl::Transform const &frameInA, vl::Transform const &frameInB);

	/// @brief python versions
	ConstraintRefPtr createDynamicConstraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, vl::Transform const &trans);

	ConstraintRefPtr createDynamicConstraint(std::string const &type,
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1,
		vl::Transform const &frameInA, vl::Transform const &frameInB);

	void removeConstraint(ConstraintRefPtr constraint);

	bool hasConstraint(vl::ConstraintRefPtr constraint) const;

	bool hasConstraint(std::string const &name) const;

	ConstraintRefPtr getConstraint(std::string const &name) const;


	/// -------------- List access ----------------------
	ConstraintList const &getConstraints(void) const;

	KinematicBodyList const &getBodies(void) const;

	// @brief switch collision detection on/off for all kinematic bodies
	// for now only works for bodies created after a call to this function
	// also collision models have hard coded part of name "*cb_"
	void enableCollisionDetection(bool enable);

	bool isCollisionDetectionEnabled(void) const
	{ return _collision_detection_on; }

	friend std::ostream &operator<<(std::ostream &os, KinematicWorld const &world);

private :

	KinematicBodyRefPtr _create_kinematic_body(vl::SceneNodePtr sn, bool dynamic);

	ConstraintRefPtr _create_constraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, 
		vl::Transform const &frameInA, vl::Transform const &frameInB, 
		std::string const &name, bool dynamic);

	void _addConstraint(vl::ConstraintRefPtr constraint);

	vl::animation::NodeRefPtr _createNode(vl::Transform const &initial_transform);

	void _progress_constraints(vl::time const &t);

	void _create_collision_body(KinematicBodyRefPtr body);

	bool _collision_detection_on;

	KinematicBodyList _bodies;
	ConstraintList _constraints;

	animation::GraphRefPtr _graph;

	GameManager *_game;

};	// class KinematicWorld

std::ostream &operator<<(std::ostream &os, KinematicWorld const &world);

std::ostream &operator<<(std::ostream &os, KinematicBodyList const &bodies);

}	// namespace vl

#endif	// HYDRA_ANIMATION_KINEMATIC_WORLD_HPP