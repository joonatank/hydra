/**
 *	Copyright (c) 2014 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2014-11
 *	@file input/razer_hydra.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_INPUT_RAZER_HYDRA_HPP
#define HYDRA_INPUT_RAZER_HYDRA_HPP

#include "input.hpp"

#include "math/types.hpp"
#include "math/transform.hpp"

#include <boost/signal.hpp>

/// Assumption is made that one machine is never connected to more than one
/// Razer hydra devices
/// also
/// we don't support Razer hydra in other than master machine unlike other joysticks.

namespace vl
{

enum RH_JOYSTICK
{
	RH_LEFT,
	RH_RIGHT,
};

/// Listing all the data in the hydra razer
/// should we have separate listeners for each
/// or should we use two events with separate data, but differentiate them with
/// RH_LEFT, RH_RIGHT for the joystick they correspond?
struct RazerHydraEvent
{
	// joystick id
	RH_JOYSTICK joystick;
	// tracker data
	Transform transform;
	// buttons (8 element bit field for now)
	std::bitset<32>	buttons;
	// axes (two axis joystick in both)
	vl::scalar axis_x;
	vl::scalar axis_y;

	vl::scalar trigger;
};

std::ostream &
operator<<(std::ostream &os, RazerHydraEvent const &e);

/**	@class RazerHydra
 *	
 *	@todo should we have a Trigger for this or do we link it directly to the device.
 */
class RazerHydra : public InputDevice
{
	typedef boost::signal<void (RazerHydraEvent const &)> Tripped;

public :
	RazerHydra(void);
	
	~RazerHydra(void);
	
	// @todo do we need to check the state outside of listeners?

	int addListener(Tripped::slot_type const &slot);

	void update(void);

private :
	Tripped _signal;
};

}	// namespace vl

#endif // HYDRA_INPUT_RAZER_HYDRA_HPP
