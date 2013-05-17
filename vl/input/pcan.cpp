/**
 *	Copyright (c) 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-05
 *	@file input/pcan.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "pcan.hpp"

#include "base/exceptions.hpp"

vl::PCAN::PCAN(void)
	: _connected(false)
	, _channel(PCAN_USBBUS1)	// For now hardcoded
	, _baudrate(PCAN_BAUD_500K)
{
	/** @todo we can somehow get the available plug and play channels 
		from the PCAN drivers, we should add support for autoselecting one
		when the user does not provide us with one. 
	**/
	_initialise();
}

void
vl::PCAN::_initialise(void)
{
	/// Only channel and baudrate are required for pnp devices (e.g. usb)
	TPCANStatus res = CAN_Initialize(_channel, _baudrate);

	if(res != PCAN_ERROR_OK)
	{
		/// @todo print the error message
		/// @todo this needs to work without the CAN hardware
		/// so we need to fail here and print the error message but not fail the whole program
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("PCAN initialise failed."));
	}

	// Sets the connection status of the main-form
	//
	_set_connected(res == PCAN_ERROR_OK);
}

void
vl::PCAN::_disconnect(void)
{
	if(_connected)
	{
		TPCANStatus res = CAN_Uninitialize(_channel);
		_set_connected(false);
	}	
}

void
vl::PCAN::_set_connected(bool connected)
{
	_connected = connected;
}

vl::PCAN::~PCAN(void)
{
	_disconnect();
}

void
vl::PCAN::mainloop(void)
{
	if(_connected)
	{
		/// @todo move to while loop
		TPCANMsg can_msg;
		TPCANStatus res = CAN_Read(_channel, &can_msg, NULL);
		if(res != PCAN_ERROR_OK && res != PCAN_ERROR_QRCVEMPTY)
		{
			char buf[256];
			CAN_GetErrorText(res, 0x09, buf);
			std::cout << "Error reading the CAN data : err = " << buf << std::endl;
		}
		/// Read buffer is empty, not sure if this is efficient way of handling it though
		if(res == PCAN_ERROR_QRCVEMPTY)
		{
			//std::cout << "CAN read buffer is empty." << std::endl;
			// @testing fps, without this we get 320 in an empty scene
			// With this and console printing FPS starts from 250 and drops steadily
			// With this and without printing FPS is constantly 310-320
			// Without the whole PCAN module FPS is constant 325
			//
			// For testing
			//int8_t d[8] = {-125, 124, 0, 0, 0, 0, 0, 0};
			//CANMsg msg(0x380, d, 8);
			//_signal(msg);
		}
		else
		{
			CANMsg msg(can_msg.ID, can_msg.DATA, can_msg.LEN);
			_signal(msg);
		}
	}
}
