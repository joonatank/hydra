/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Ville Lepokorpi <ville.lepokorpi@savantsimulators.com>
 *	@date 2011-07
 *	@file input/serial_joystick_event.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
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
#include "cluster/message.hpp"
#include "typedefs.hpp"

//Used for event data for buttons
#include <bitset>

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
	// 32 buttons supported at maximum
	std::bitset<32>				buttons;
	std::vector<vl::scalar>		axes;
	//sliders are missing:
	//std::vector<Slider>		sliders;
	//currently pov is missing:
	//std::vector<POV>			povs;
	// @todo why do we need vectors? I think they are only used for wii
	std::vector<vl::Vector3>	vectors;

	/// need to define functions 
	bool isButtonDown(size_t index) const
	{ return index < buttons.size() ? buttons.at(index) : false; }

	bool isAnyButtonDown(void) const
	{ return buttons.any(); }

	bool isNoButtonsDown(void) const
	{ return !buttons.any(); }

	bool isAllButtonsDown(void) const
	{ return buttons.all(); }

	std::vector<vl::scalar> const &getRanges(void) const
	{return axes;}
	
	std::vector<vl::Vector3> const &getVectors(void) const
	{return vectors;}
	
	//Get states eg. buttons
	std::vector<bool> const &getStates(void) const
	{
		
		size_t vecsize = buttons.size();
		std::vector<bool> vec;
		vec.resize(vecsize);

		for(size_t i = 0; i < vecsize; ++i)
		{vec.at(i) = buttons.at(i);}
		
		return vec;
	}


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
		BUTTON_PRESSED=0,
		BUTTON_RELEASED,
		AXIS,
		VECTOR,
		POV,
		SLIDER
	};

	JoystickEvent() : info(JoystickInfo()), state(JoystickState()) {}
	
	JoystickInfo const &getInfo(void) const
	{return info;}
	
	JoystickState const &getState(void) const
	{return state;}

	EventType const &getType(void) const
	{return type;}

	EventType	type;
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
	msg << evt.type << evt.info.dev_id << evt.info.input_manager_id << evt.info.vendor_id;
	msg << evt.state.axes << evt.state.buttons << evt.state.vectors;
	return msg;
}

template<> inline
ByteStream &operator>>(ByteStream &msg, vl::JoystickEvent &evt)
{
	msg >> evt.type >> evt.info.dev_id >> evt.info.input_manager_id >> evt.info.vendor_id;
	msg >> evt.state.axes >> evt.state.buttons >> evt.state.vectors;
	return msg;
}


} // namespace cluster

} // namespace vl

#endif // HYDRA_INPUT_JOYSTICK_EVENT_HPP

