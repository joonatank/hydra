/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Ville Lepokorpi <ville.lepokorpi@savantsimulators.com>
 *	@date 2011-07
 *	@file input/serial_joystick_event.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	Joystick Event structure, used by possibly all joysticks.
 */


#ifndef HYDRA_INPUT_JOYSTICK_EVENT_HPP
#define HYDRA_INPUT_JOYSTICK_EVENT_HPP

//#include <vector>
// Necessary for ostream implementation
#include <iostream>
// Necessary for vl::scalar and uintX_t types
#include "math/math.hpp"
//#include "typedefs.hpp"
#include "cluster/message.hpp"
#include "typedefs.hpp"


namespace vl {

typedef int dev_id_t;
typedef std::string vendor_id_t;
typedef int input_manager_id_t;


struct JoystickInfo {
	//Serialization is in the end of this file
	JoystickInfo(void): vendor_id(""), dev_id(0), input_manager_id(0) {}
	vendor_id_t			vendor_id;
	dev_id_t			dev_id;
	input_manager_id_t	input_manager_id;
};

struct JoystickState
{
	std::vector<bool>			buttons;
	std::vector<vl::scalar>		axes;
	//sliders are missing:
	//std::vector<Slider>		sliders;
	//currently pov is missing:
	//std::vector<POV>			povs;
	std::vector<vl::Vector3>	vectors;
};

struct JoystickEvent
{
	enum EventType
	{
		UNKNOWN=0,
		BUTTON_PRESSED,
		BUTTON_RELEASED,
		AXIS,
		VECTOR
	};

	JoystickEvent() : info(), state() {}
	~JoystickEvent() {}
	JoystickInfo info;
	JoystickState state;
};

/*
namespace cluster {

template<> inline
ByteStream &operator <<(ByteStream &msg, vl::JoystickEvent const &evt)
{
	msg << evt.info.dev_id << evt.info.input_manager_id << evt.info.vendor_id;
	msg << evt.state.axes << evt.state.buttons << evt.state.vectors;
	return msg;
}

template<> inline
ByteStream &operator >>(ByteStream &msg, vl::JoystickEvent &evt)
{
	msg >> evt.info.dev_id >> evt.info.input_manager_id >> evt.info.vendor_id;
	msg >> evt.state.axes >> evt.state.buttons >> evt.state.vectors;
	return msg;
}



} // namespace cluster
*/

} // namespace vl

#endif //

