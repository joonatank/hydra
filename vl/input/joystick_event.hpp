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

//Used for event data for buttons:
//#include "boost/dynamic_bitset.hpp"

namespace vl {

typedef int dev_id_t;
typedef std::string vendor_id_t;
typedef int input_manager_id_t;


struct JoystickInfo {
	JoystickInfo(void): vendor_id(""), dev_id(0), input_manager_id(0) {}
	
	vendor_id_t const &getVendorID() const
	{return vendor_id;}
	
	dev_id_t const &getDevID() const
	{return dev_id;}
	
	input_manager_id_t const &getOwnerID() const
	{return input_manager_id;}

	vendor_id_t			vendor_id;
	dev_id_t			dev_id;
	input_manager_id_t	input_manager_id;

};

inline std::ostream &
operator<<(std::ostream &os, JoystickInfo const &info)
{ 
	os << "Vendor ID: " << info.vendor_id << std::endl;
	os << "Device ID: " << info.dev_id << std::endl;
	os << "InputManager/Owner ID: " << info.input_manager_id << std::endl;
	return os;
}

struct JoystickState
{
	JoystickState(void) {}
	std::vector<char>			buttons;
	std::vector<vl::scalar>		axes;
	//sliders are missing:
	//std::vector<Slider>		sliders;
	//currently pov is missing:
	//std::vector<POV>			povs;
	std::vector<vl::Vector3>	vectors;
	
};


inline std::ostream &
operator<<(std::ostream &os, JoystickState const &state)
{ 
	os << "Buttons: " << std::endl;
	size_t vecsize = state.buttons.size();
	for(size_t i = 0; i < vecsize; ++i)
	{os << "#" << i << ": " << state.buttons.at(i) << std::endl;}
	
	os << "Axes: "<< std::endl;
	vecsize = state.axes.size();
	for(size_t i = 0; i < vecsize; ++i)
	{os << "#" << i << ": " << state.axes.at(i) << std::endl;}
	
	os << "Vectors: " << std::endl;
	vecsize = state.vectors.size();
	for(size_t i = 0; i < vecsize; ++i)
	{os << "#" << i << ": " << state.vectors.at(i) << std::endl;}
	
	return os;
}

//Serialization is at the end of this file!
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

	JoystickEvent() : info(JoystickInfo()), state(JoystickState()) {}
	
	JoystickInfo info;
	JoystickState state;
};


inline std::ostream &
operator<<(std::ostream &os, JoystickEvent const &evt)
{ 
	os << "----JOYSTICK EVENT----" << std::endl;
	os << "--InfoPart--" << std::endl;
	os << evt.info;
	os << "--StatePart--"<< std::endl;
	os << evt.state;
	return os;
}


namespace cluster {

template<> inline
ByteStream &operator<<(ByteStream &msg, vl::JoystickEvent const &evt)
{
	msg << evt.info.dev_id << evt.info.input_manager_id << evt.info.vendor_id;
	msg << evt.state.axes << evt.state.buttons << evt.state.vectors;
	return msg;
}

template<> inline
ByteStream &operator>>(ByteStream &msg, vl::JoystickEvent &evt)
{
	msg >> evt.info.dev_id >> evt.info.input_manager_id >> evt.info.vendor_id;
	msg >> evt.state.axes >> evt.state.buttons >> evt.state.vectors;
	return msg;
}

/*
template<> inline
ByteStream &operator << < std::vector<bool> >(ByteStream &msg, std::vector<bool> const &vec)
{
	size_t vecsize = vec.size();
	msg << vecsize;
	if(vecsize > 0)
	{
		for(size_t i = 0; i < vecsize; ++i)
		{
			msg << vec.at(i);
		}
	}
	return msg;
}

template<> inline
ByteStream &operator >> < std::vector<bool> >(ByteStream &msg, std::vector<bool> &vec)
{
	size_t vecsize = 0;
	msg >> vecsize;
	if(vecsize > 0)
	{
		for(size_t i = 0; i < vecsize; ++i)
		{
			msg >> vec.at(i);
		}
	}
	return msg;
}
*/


} // namespace cluster

} // namespace vl

#endif //

