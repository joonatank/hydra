/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file base/state_machines.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_BASE_STATE_MACHINES_HPP
#define HYDRA_BASE_STATE_MACHINES_HPP

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30 
#define BOOST_MPL_LIMIT_MAP_SIZE 30

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
