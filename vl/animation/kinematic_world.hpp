/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_world.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**	World object that deals with Kinematic bodies and constraints.
 *	Provides the same interface as physics world but using our own
 *	kinematic solver.
 */

#ifndef HYDRA_ANIMATION_KINEMATIC_WORLD_HPP
#define HYDRA_ANIMATION_KINEMATIC_WORLD_HPP

#include "base/timer.hpp"

#include "typedefs.hpp"

#include "math/transform.hpp"

#include "animation.hpp"

#include <iostream>

namespace vl
{

class KinematicWorld
{
public :
	KinematicWorld(void);

	~KinematicWorld(void);

	void step(vl::time const &t);

	KinematicBodyRefPtr getKinematicBody(std::string const &name) const;

	/// @brief retrieve already created by for the SceneNode
	/// @param sn the scene node which the body controls
	/// @return valid body if one already exists, None if not
	KinematicBodyRefPtr getKinematicBody(vl::SceneNodePtr sn) const;

	/// @brief retrieve already existing body for the SceneNode or create a new
	/// @param sn the scene node which this body controls
	/// @return always valid body node, new one if none exists
	KinematicBodyRefPtr createKinematicBody(vl::SceneNodePtr sn);

	void removeKinematicBody(KinematicBodyRefPtr body);

	/// -------------- Constraints -----------------------
	ConstraintRefPtr createConstraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, vl::Transform const &trans);

	void removeConstraint(ConstraintRefPtr constraint);

	bool hasConstraint(vl::ConstraintRefPtr constraint) const;

	/// -------------- List access ----------------------
	ConstraintList const &getConstraints(void) const;

	KinematicBodyList const &getBodies(void) const;

private :
	void _addConstraint(vl::ConstraintRefPtr constraint);

	vl::animation::NodeRefPtr _createNode(void);

	void _progress_constraints(vl::time const &t);

	KinematicBodyList _bodies;
	ConstraintList _constraints;

	animation::GraphRefPtr _graph;

};	// class KinematicWorld

inline
std::ostream &
operator<<(std::ostream &os, KinematicWorld const &world)
{
	os << "KinematicWorld with " << world.getBodies().size() << " bodies and " 
		<< world.getConstraints().size() << " constraints." << std::endl;
	return os;
}

inline
std::ostream &
operator<<(std::ostream &os, KinematicBodyList const &bodies)
{
	os << "Kinematic body list : length = " << bodies.size() << std::endl;

	return os;
}

}	// namespace vl

#endif	// HYDRA_ANIMATION_KINEMATIC_WORLD_HPP