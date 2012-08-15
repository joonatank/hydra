/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file physics/motion_state.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "motion_state.hpp"

/// Concrete implementations
#ifdef USE_BULLET
#include "motion_state_bullet.hpp"
#else if USE_NEWTON
#include "motion_state_newton.hpp"
#endif

vl::physics::MotionState *
vl::physics::MotionState::create(vl::Transform const &t, vl::ObjectInterface *node)
{
#if defined USE_BULLET
	return new BulletMotionState(t, node);
#else 
#if defined USE_NEWTON
#error "Newton Motion State not yet implemented"
#else
	return new MotionState(t, node);
#endif
#endif
}
