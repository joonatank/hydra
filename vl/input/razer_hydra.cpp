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
	int max_bases = sixenseGetMaxBases();
	std::cout << max_bases << " bases." << std::endl;
	// this is wrong, I'm assuming it's SIXSENSE_SUCCESS instead
	// but need to test
	bool connected = (sixenseIsBaseConnected(0) == 1);
	/*
	if(!connected)
	{
		std::string err("Base is not connected.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err));
	}
	//sixenseSetActiveBase( int i );
	
	int max_controllers = sixenseGetMaxControllers();
	if(!sixenseIsControllerEnabled(0))
	{
		std::string err("Controller 0 not connected.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err));
	}
	if(!sixenseIsControllerEnabled(1))
	{
		std::string err("Controller 1 not connected.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err));
	}
	*/
	//sixenseGetNumActiveControllers();

	std::cout << "vl::RazerHydra::RazerHydra : DONE" << std::endl;
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
	sixenseSetActiveBase(0);
	sixenseAllControllerData acd;
	sixenseGetAllNewestData( &acd );

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
	// @todo we are assuming first one is left which might not be the case
	// @todo missing buttons
	RazerHydraEvent evt;
	evt.joystick = RH_LEFT;
	evt.transform.position = Ogre::Vector3(acd.controllers[0].pos);
	evt.transform.quaternion.w = acd.controllers[0].rot_quat[3];
	evt.transform.quaternion.x = acd.controllers[0].rot_quat[0];
	evt.transform.quaternion.y = acd.controllers[0].rot_quat[1];
	evt.transform.quaternion.z = acd.controllers[0].rot_quat[2];
	evt.axis_x = acd.controllers[0].joystick_x;
	evt.axis_y = acd.controllers[0].joystick_y;
	evt.trigger = acd.controllers[0].trigger;
	
	_signal(evt);

	evt.joystick = RH_RIGHT;
	evt.transform.position = Ogre::Vector3(acd.controllers[1].pos);
	evt.transform.quaternion.w = acd.controllers[0].rot_quat[3];
	evt.transform.quaternion.x = acd.controllers[0].rot_quat[0];
	evt.transform.quaternion.y = acd.controllers[0].rot_quat[1];
	evt.transform.quaternion.z = acd.controllers[0].rot_quat[2];
	evt.axis_x = acd.controllers[1].joystick_x;
	evt.axis_y = acd.controllers[1].joystick_y;
	evt.trigger = acd.controllers[1].trigger;

	_signal(evt);

	// acd.controllers[1].which_hand
	// acd.controllers[1].hemi_tracking_enabled
	// are empty unless we use the utilities

	_signal(RazerHydraEvent());
}
