/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file physics/motion_state_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_PHYSICS_MOTION_STATE_BULLET_HPP
#define HYDRA_PHYSICS_MOTION_STATE_BULLET_HPP

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>

#include "motion_state.hpp"

#include "math/math.hpp"
#include "math/conversion.hpp"

namespace vl
{

namespace physics
{

/** @class BulletMotionState
 *
 */
class BulletMotionState : public btMotionState, public vl::physics::MotionState
{
public:
	BulletMotionState(vl::Transform const &trans, vl::ObjectInterface *node = 0)
		: MotionState(trans, node)
	{}

	virtual ~BulletMotionState()
	{}

	virtual void getWorldTransform(btTransform &worldTrans) const
	{
		worldTrans = vl::math::convert_bt_transform(_trans);
	}

	virtual void setWorldTransform(const btTransform &worldTrans)
	{
		MotionState::setWorldTransform(vl::math::convert_transform(worldTrans));
	}

};	// class BulletMotionState

}	// namespace physics

}	// namespace vl

#endif // HYDRA_PHYSICS_MOTION_STATE_BULLET_HPP
