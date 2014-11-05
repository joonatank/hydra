/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file physics/motion_state.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "motion_state.hpp"

/// Concrete implementation
#include "motion_state_bullet.hpp"

vl::physics::MotionState *
vl::physics::MotionState::create(vl::Transform const &t, vl::ObjectInterface *node)
{
	return new BulletMotionState(t, node);
}
