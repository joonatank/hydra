/**
 *	Copyright (c) 2014 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2014-11
 *	@file input/razer_hydra.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "razer_hydra.hpp"

#include "base/exceptions.hpp"

#include <sixense.h>

std::ostream &
vl::operator<<(std::ostream &os, vl::RazerHydraEvent const &e)
{
	if(e.joystick == RH_LEFT)
	{ os << "left joystick "; }
	else
	{ os << "right joystick "; }
	os << " transform = " << e.transform 
		<< " axis = (" << e.axis_x << ", " << e.axis_y << ")"
		<< " trigger = " << e.trigger << std::endl;
	
	// @todo add buttons

	return os;
}


vl::RazerHydra::RazerHydra(void)
{
	std::cout << "vl::RazerHydra::RazerHydra" << std::endl;
	int  res = sixenseInit();
	assert(res == SIXENSE_SUCCESS);

	/// Find if base and controller is connected
	/// we should have one base and two controllers
	//
	// For some reason is base connected always returs 0
	//
	// setActiveBase and other functions so far tested
	// always return 0
	// so no way to tell if the Razer Hydra device is connected or not

	// these always returns success so checking them pointless
	// res = sixenseIsBaseConnected(0);
	// res = sixenseSetActiveBase(0);

	assert(sixenseIsControllerEnabled(0) == SIXENSE_SUCCESS);
}
	
vl::RazerHydra::~RazerHydra(void)
{
	sixenseExit();
}

int
vl::RazerHydra::addListener(Tripped::slot_type const &slot)
{
	_signal.connect(slot);
	return 1;
}

void
vl::RazerHydra::update(void)
{
	int res = sixenseSetActiveBase(0);
	assert(res == SIXENSE_SUCCESS);
	sixenseAllControllerData acd;
	res = sixenseGetAllNewestData( &acd );
	assert(res == SIXENSE_SUCCESS);

/*	
typedef struct _sixenseControllerData {
  float pos[3];
  float rot_mat[3][3];
  float joystick_x;
  float joystick_y;
  float trigger;
  unsigned int buttons;
  unsigned char sequence_number;
  float rot_quat[4];
  unsigned short firmware_revision;
  unsigned short hardware_revision;
  unsigned short packet_type;
  unsigned short magnetic_frequency;
  int enabled;
  int controller_index;
  unsigned char is_docked;
  unsigned char which_hand;
  unsigned char hemi_tracking_enabled;
} sixenseControllerData;
*/
	// for now we support only two controllers
	// this is the case for Razer Hydra
	// it's not the case for the Sixsense general purpose tracker
	// that is released for developers soon

	for(size_t i = 0; i < 2; ++i)
	{
		// the only way to check if the controller/base is connected to the
		// computer is by checking the actual update message
		// so we'll just ignore any messages here if controllers are not connected
		if(acd.controllers[i].enabled == 0)
		{ continue; }

		// @todo we are assuming first one is left which might not be the case
		// @todo missing buttons
		RazerHydraEvent evt;
		// @todo figure out a nicer way to select joystick, 
		// how do we know if it's the left or right one?
		evt.joystick = (RH_JOYSTICK)i;
		// the values are in millimeters
		evt.transform.position = Ogre::Vector3(acd.controllers[i].pos)/1000.0;
		evt.transform.quaternion.w = acd.controllers[i].rot_quat[3];
		evt.transform.quaternion.x = acd.controllers[i].rot_quat[0];
		evt.transform.quaternion.y = acd.controllers[i].rot_quat[1];
		evt.transform.quaternion.z = acd.controllers[i].rot_quat[2];
		evt.axis_x = acd.controllers[i].joystick_x;
		evt.axis_y = acd.controllers[i].joystick_y;
		evt.trigger = acd.controllers[i].trigger;
	
		_signal(evt);
	}

	// parameter documentation
	// sequence_number : no idea seems like a random character
	// firmware_revision : zero
	// hardware_revision : zero when not connected 174 when connected
	// packet_type : 52428 when not connected, 1 when connected
	// magnetic_frequency : zero
	// which_hand : empty
	// hemi_tracking_enabled : empty
	// is_docked : random char
}
