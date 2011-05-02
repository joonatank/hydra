/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-05 initial
 *	2010-06 some meat
 *	2010-11 added trigger and action support
 *
 *	Tracker interface.
 *	Real trackers should derive from this.
 *
 *	Has add callback method which user can use to add a callback functor for
 *	specific sensors.
 */

#ifndef VL_TRACKER_HPP
#define VL_TRACKER_HPP

#include "trigger.hpp"

#include "typedefs.hpp"

// Necessary for vl::scalar and vl::Transform
#include "math/math.hpp"

namespace vl
{

/// Trigger class that has a callback to an Action (new event handling design).
class TrackerTrigger : public vl::TransformActionTrigger
{
public :
	TrackerTrigger( void ) {}

	virtual std::string getTypeName( void ) const
	{ return "TrackerTrigger"; }

	/// -------------- Custom Methods ----------------

	/// Name is used for creation from XML and mapping from python
	void setName( std::string const &name )
	{ _name = name; }

	virtual std::string getName( void ) const
	{ return _name; }

protected :
	std::string _name;

};

class Sensor
{
public :
	Sensor( Ogre::Vector3 const &default_pos = Ogre::Vector3::ZERO,
			Ogre::Quaternion const &default_quat = Ogre::Quaternion::IDENTITY );

	/// Sensor values used when not connected to a tracker
	/// Uses different sensor data stack, so if connection is lost for long it can be
	/// defaulted to these values.
	/// For now supports default values if no tracker updates are done
	void setDefaultPosition( Ogre::Vector3 const &pos );
	void setDefaultOrientation( Ogre::Quaternion const &quat );

	void setDefaultTransform(vl::Transform const &t);

	vl::Transform const &getDefaultTransform(void) const
	{ return _default_value; }

	vl::Transform const &getCurrentTransform(void) const
	{ return _last_value; }

	void setTrigger( vl::TrackerTrigger *trigger );

	/// This will return the current trigger of a sensor
	vl::TrackerTrigger *getTrigger( void );

	/// Callback function for updating the Sensor data
	void update( vl::Transform const &data );

protected :
	vl::TrackerTrigger *_trigger;

	vl::Transform _default_value;
	vl::Transform _last_value;

};

std::ostream &
operator<<(std::ostream &os, Sensor const &s);

class Tracker
{
public :
	virtual ~Tracker( void ) {}

	/// @brief mainloop of the tracker for the base tracker empty
	virtual void mainloop(void) {}

	/// @brief Set a sensor does not change the number of sensors
	/// @param i sensor index
	/// @param sensor sensor definition
	void setSensor(size_t i, Sensor const &sensor);

	/// @brief Set a sensor and if there is not enough sensors add it
	/// @param i sensor index
	/// @param sensor sensor definition
	void addSensor(size_t i, vl::Sensor const &sensor);

	Sensor &getSensor(size_t i)
	{ return _sensors.at(i); }

	Sensor const &getSensor(size_t i) const
	{ return _sensors.at(i); }

	void setNSensors(size_t size);

	size_t getNSensors(void) const
	{ return _sensors.size(); }

	void setTransformation(vl::Transform const &trans)
	{ _transform = trans; }

	vl::Transform const &getTransformation(void) const
	{ return _transform; }

	std::string const &getName(void) const
	{ return _name; }

	static TrackerRefPtr create(std::string const &trackerName = std::string())
	{ return TrackerRefPtr(new Tracker(trackerName)); }

protected :
	/// Protected constructor so that user needs to call create
	Tracker(std::string const &trackerName);

	std::string _name;

	std::vector<Sensor> _sensors;

	/// @todo this needs a separate Transform class with permutation, scaling and flipping
	vl::Transform _transform;

};	// class Tracker


class Clients
{
public :
	Clients( vl::EventManagerPtr event_manager )
		: _event_manager(event_manager )
	{}

	void addTracker( TrackerRefPtr tracker )
	{ _trackers.push_back( tracker ); }

	Tracker const &getTracker(size_t index) const
	{ return *_trackers.at(index); }

	TrackerRefPtr getTrackerPtr(size_t index)
	{ return _trackers.at(index); }

	size_t getNTrackers(void) const
	{ return _trackers.size(); }

	vl::EventManagerPtr getEventManager(void)
	{ return _event_manager; }

protected :
	std::vector<TrackerRefPtr> _trackers;

	vl::EventManagerPtr _event_manager;
};

std::ostream &
operator<<(std::ostream &os, Tracker const &t);

std::ostream &
operator<<(std::ostream &os, Clients const &c);

}	// namespace vl

#endif
