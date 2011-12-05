/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file: vrpn_analog_client.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "vrpn_analog_client.hpp"

namespace {

void VRPN_CALLBACK handle_analog(void *userdata, const vrpn_ANALOGCB t)
{
	vl::vrpn_analog_client *analog = (vl::vrpn_analog_client *)userdata;
	analog->_update(t);
}

}


vl::vrpn_analog_client::vrpn_analog_client(void)
	: _vrpn_analog(0)
{
}

void
vl::vrpn_analog_client::mainloop(void)
{
	assert(_vrpn_analog);
	_vrpn_analog->mainloop();
}

void
vl::vrpn_analog_client::_create(std::string const &name)
{
	_vrpn_analog = new vrpn_Analog_Remote(name.c_str());

	// Set the callback function
	_vrpn_analog->shutup = true;
	_vrpn_analog->register_change_handler(this, handle_analog);
}

void
vl::vrpn_analog_client::setNSensors(size_t size)
{
	size_t old_size = _sensors.size();
	_sensors.resize(size);
	for(size_t i = old_size; i < size; ++i)
	{ _sensors.at(i).reset(new analog_sensor); }
}

void
vl::vrpn_analog_client::_update(vrpn_ANALOGCB const &val)
{
	if(_sensors.size() < val.num_channel)
	{ _sensors.resize(val.num_channel); }

	for(size_t i = 0; i < val.num_channel; ++i)
	{
		if(!_sensors.at(i))
		{ _sensors.at(i).reset(new analog_sensor); }

		_sensors.at(i)->update(val.channel[i]);
	}
}
