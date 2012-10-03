/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file recording.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**
 *	Recording of the the user interaction
 *	Can be loaded using ResourceManager and integrates to the clusters resource system.
 *
 *	At the moment only supports only VRPN data recordings.
 *
 * File format for VRPN data recording
 * # time\t sensor\t position\t orientation" << std::endl
 * # delimeter is tabulator (\t), vector elements are separated with ','
 * # time is a floating point, the absolute program time in seconds
 * # sensor is an integer
 * # position is (x, y, z) vector in meters
 * # orientation (w, x, y, z) quaternion
 */

#ifndef HYDRA_RECORDING_HPP
#define HYDRA_RECORDING_HPP

#include "math/transform.hpp"

#include "cluster/message.hpp"

#include "base/time.hpp"

#include "resource.hpp"

namespace vl
{

/// @class Recording
/// @brief Maps points in time to VRPN sensor data
/// @todo add functions for removing elements using time
/// One would use (start_time, stop_time) for example to clip anything outside the range
/// Or set_sampling_rate(rate) to remove extra samples
class Recording
{
public :
	struct sensor
	{
		std::map<vl::time, vl::Transform> transforms;
	};

	Recording(std::string const &name = std::string());

	virtual ~Recording(void);

	void read(vl::Resource &res);

	std::vector<sensor> sensors;

	std::string const &getName(void) const
	{ return _name; }

	void setName(std::string const &name)
	{ _name = name; }

private :
	void _parse_line(std::string const &input_line);

	std::string _name;

};	// class Recording

std::ostream &
operator<<(std::ostream &os, vl::Recording const &rec);

namespace cluster
{

template<>
ByteStream &operator<<(ByteStream &msg, vl::Recording const &rec);

template<>
ByteStream &operator>>(ByteStream &msg, vl::Recording &rec);

}	// namespace cluster

}	// namespace vl

#endif // HYDRA_RECORDING_HPP
