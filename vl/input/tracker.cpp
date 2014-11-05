/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-05
 *	@file tracker.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	
 *	@date 2010-11
 *	Added event system support, rudimentary at the moment
 *	Triggers can be created separately from actions so they can be created from
 *	config file.
 *	Triggers can be retrieved by name and actions can be mapped to the triggers.
 *	Should be possible to get it working from python relatively quickly.
 *
 *	Generic Tracker implementation.
 */

#include "tracker.hpp"

/// ------------------------------ Global ------------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::TrackerSensor const &s)
{
	if(s.getDefaultTransform().isIdentity())
	{
		os << " : default transform is identity." << std::endl;
	}
	else
	{ os << "default transform " << s.getDefaultTransform(); }
	os << "\n";

	if(s.getCurrentTransform().isIdentity())
	{
		os << " : current transform is identity." << std::endl;
	}
	else
	{ os << " : current transform " << s.getCurrentTransform(); }
	os << "\n";

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::Tracker const &t)
{
	os << t.getName();

	for(size_t i = 0; i < t.getNSensors(); ++i)
	{
		os << "Sensor " << i << " : " << t.getSensor(i) << "\n";
	}

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::Clients const &c)
{
	os << "Clients : \n";
	for( size_t i = 0; i < c.getNTrackers(); ++i )
	{
		os << "Tracker : " << c.getTracker(i) << "\n";
	}

	return os;
}

/// ------------------------------ Sensor ------------------------------------
vl::TrackerSensor::TrackerSensor(const Ogre::Vector3& default_pos, const Ogre::Quaternion& default_quat)
	: _trigger(0)
	, _default_value( default_pos, default_quat )
{}

void
vl::TrackerSensor::setDefaultPosition( Ogre::Vector3 const &pos )
{
	// Save the reference for later
	_default_value.position = pos;

	// We need to update the trigger with the new defaults
	// So that if the values are not beign updated we are using the new defaults
	// Problem if the values are updated every 8ms this will cause a slight clitch
	update( _default_value );
}

void
vl::TrackerSensor::setDefaultOrientation( Ogre::Quaternion const &quat )
{
	_default_value.quaternion = quat;

	// We need to update the trigger with the new defaults
	// So that if the values are not beign updated we are using the new defaults
	// Problem if the values are updated every 8ms this will cause a slight clitch
	update( _default_value );
}

void 
vl::TrackerSensor::setDefaultTransform(vl::Transform const &t)
{
	_default_value = t;
	update(_default_value);
}

vl::TrackerTrigger *
vl::TrackerSensor::getTrigger( void )
{
	return _trigger;
}

void
vl::TrackerSensor::setTrigger( vl::TrackerTrigger* trigger )
{
	_trigger = trigger;
	// We need to update the new trigger
	update( _default_value );
}

void 
vl::TrackerSensor::update(const vl::Transform& data)
{
	_last_value = data;

	if( _trigger )
	{
		_trigger->update(data);
	}
}


/// --------- Tracker --------------
vl::Tracker::Tracker(std::string const &trackerName)
	: _name(trackerName)
	, incorrect_quaternion(false)
	, _scale(Ogre::Vector3::UNIT_SCALE)
	, _permute(Ogre::Vector3(0, 1, 2))
	, _sign(Ogre::Vector3::UNIT_SCALE)
	, _neutral_position(Ogre::Vector3::ZERO)
	, _neutral_quaternion(Ogre::Quaternion::IDENTITY)
{}

void
vl::Tracker::setSensor(size_t i, vl::TrackerSensor const &sensor)
{
	_sensors.at(i) = sensor;
}

void
vl::Tracker::addSensor(size_t i, vl::TrackerSensor const &sensor)
{
	if(_sensors.size() <= i)
	{ _sensors.resize(i+1); }
	_sensors.at(i) = sensor;
}

void 
vl::Tracker::setNSensors(size_t size)
{
	_sensors.resize(size);
}
