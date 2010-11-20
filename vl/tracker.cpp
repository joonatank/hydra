/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 *	Added event system support, rudimentary at the moment
 *	Triggers can be created separately from actions so they can be created from
 *	config file.
 *	Triggers can be retrieved by name and actions can be mapped to the triggers.
 *	Should be possible to get it working from python relatively quickly.
 *
 *	Generic Tracker implementation.
 */

#include "tracker.hpp"

void vl::NodeTrackerAction::execute(const vl::SensorData& data)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setPosition( data.position );
	_node->setOrientation( data.quaternion );
}


std::string const &
vl::NodeTrackerAction::getTypeName(void ) const
{ return NodeTrackerActionFactory::TYPENAME; }

const std::string vl::NodeTrackerActionFactory::TYPENAME = "NodeTrackerAction";


/// ----------- TrackerTrigger ------------
void vl::TrackerTrigger::update(const vl::SensorData& data)
{
	if( _action )
	{
		_action->execute(data);
	}
}

void vl::TrackerTrigger::setAction(vl::TrackerActionPtr action)
{
	if( _action != action )
	{
		_action = action;
		// TODO this should update but we don't have the data anymore
	}
}

std::string const &
vl::TrackerTrigger::getTypeName(void ) const
{ return TrackerTriggerFactory::TYPENAME; }

const std::string vl::TrackerTriggerFactory::TYPENAME = "TrackerTrigger";



/// ------------ Sensor ---------------

void
vl::Sensor::setDefaultPosition( Ogre::Vector3 const &pos )
{
	// Save the reference for later
	_default_value.position = pos;

	// We need to update the trigger with the new defaults
	// So that if the values are not beign updated we are using the new defaults
	// Problem if the values are updated every 8ms this will cause a slight clitch
	update( _default_value );
}

void
vl::Sensor::setDefaultOrientation( Ogre::Quaternion const &quat )
{
	_default_value.quaternion = quat;

	// We need to update the trigger with the new defaults
	// So that if the values are not beign updated we are using the new defaults
	// Problem if the values are updated every 8ms this will cause a slight clitch
	update( _default_value );
}

vl::TrackerTrigger *
vl::Sensor::getTrigger( void )
{
	return _trigger;
}

void
vl::Sensor::setTrigger( vl::TrackerTrigger* trigger )
{
	_trigger = trigger;
	// We need to update the new trigger
	update( _default_value );
}

void vl::Sensor::update(const vl::SensorData& data)
{
	if( _trigger )
	{
		_trigger->update(data);
	}
}


/// --------- Tracker --------------
void
vl::Tracker::setSensor(size_t i, vl::SensorRefPtr sensor)
{
	if( _sensors.size() <= i )
	{ _sensors.resize( i+1 ); }
	_sensors.at(i) = sensor;
}

vl::SensorRefPtr
vl::Tracker::getSensor(size_t i)
{
	if( _sensors.size() <= i )
	{ return SensorRefPtr(); }
	return _sensors.at(i);
}
