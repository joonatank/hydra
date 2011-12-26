/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file base/state_machines.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#ifndef HYDRA_BASE_STATE_MACHINES_HPP
#define HYDRA_BASE_STATE_MACHINES_HPP

// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>

namespace vl
{

namespace msm = boost::msm;
namespace mpl = boost::mpl;

using boost::msm::front::none;

// Defined so that states are polymorphic
struct state
{
	virtual ~state(void) {}
};

}

#endif	// HYDRA_BASE_STATE_MACHINES_HPP