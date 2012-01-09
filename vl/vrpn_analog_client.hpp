/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file: vrpn_analog_client.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */


#ifndef HYDRA_VRPN_ANALOG_CLIENT_HPP
#define HYDRA_VRPN_ANALOG_CLIENT_HPP

#include "typedefs.hpp"
#include "math/types.hpp"

#include <boost/signal.hpp>

#include <vrpn_Analog.h>

namespace vl
{

class analog_sensor
{
	typedef boost::signal<void (vl::scalar)> Tripped;
public :
	analog_sensor(void) {}

	// boost::signal is noncopiable so we need to implement copy constructors
	analog_sensor(analog_sensor const &) {}
	analog_sensor &operator=(analog_sensor const &) {}

	int addListener(Tripped::slot_type const &slot)
	{ _signal.connect(slot); return 1; }

	void update(vl::scalar v)
	{
		_value = v;
		_signal(v);
	}

	vl::scalar value(void) const
	{ return _value; }

private :
	Tripped _signal;

	vl::scalar _value;

};	// class analog_sensor

inline
std::ostream &operator<<(std::ostream &os, analog_sensor const &s)
{
	os << "analog sensor : with value = " << s.value() << std::endl;
	return os;
}

class vrpn_analog_client
{
public :
	vrpn_analog_client(void);

	void mainloop(void);

	void _create(std::string const &name);

	// Callback function
	void _update(vrpn_ANALOGCB const &);

	analog_sensor_ref_ptr getSensor(size_t i)
	{ return _sensors.at(i); }

	size_t getNSensors(void) const
	{ return _sensors.size(); }

	void setNSensors(size_t size);

private :
	vrpn_Analog_Remote *_vrpn_analog;

	std::vector<analog_sensor_ref_ptr> _sensors;

};	// class vrpn_analog_client

inline
std::ostream &operator<<(std::ostream &os, vrpn_analog_client const &c)
{
	os << "vrpn analog client : with " << c.getNSensors() << " sensors." << std::endl;
	return os;
}

}	// namespace vl

#endif	// HYDRA_VRPN_ANALOG_CLIENT_HPP
