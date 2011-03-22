
/**	Joonatan Kuosa
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
	virtual void setDefaultPosition( Ogre::Vector3 const &pos );
	virtual void setDefaultOrientation( Ogre::Quaternion const &quat );

	virtual void setTrigger( vl::TrackerTrigger *trigger );

	/// This will return the current trigger of a sensor
	virtual vl::TrackerTrigger *getTrigger( void );

	/// Callback function for updating the Sensor data
	virtual void update( vl::Transform const &data );

protected :
	vl::TrackerTrigger *_trigger;

	vl::Transform _default_value;

};

typedef boost::shared_ptr<Sensor> SensorRefPtr;


class Tracker
{
public :
	Tracker( void );

	virtual ~Tracker( void ) {}

	virtual void init( void ) = 0;

	virtual void mainloop( void ) = 0;

	/// Set a sensor created else where. Sensors should be created to the heap
	/// So they stay alive as long as the tracker.
	// TODO where are the sensors created?
	// Should they be created here or in the calling code?
	virtual void setSensor( size_t i, SensorRefPtr sensor );

	virtual SensorRefPtr getSensor( size_t i );

	size_t getNSensors( void ) const
	{ return _sensors.size(); }

	void setTransformation( Ogre::Matrix4 const &trans )
	{ _transform = trans; }

	Ogre::Matrix4 const &getTransformation( void )
	{ return _transform; }

protected :
	std::vector<SensorRefPtr> _sensors;

	Ogre::Matrix4 _transform;

};	// class Tracker


class Clients
{
public :
	Clients( vl::EventManagerPtr event_manager )
		: _event_manager(event_manager )
	{}

	void addTracker( TrackerRefPtr tracker )
	{ _trackers.push_back( tracker ); }

	TrackerRefPtr getTracker( size_t index )
	{ return _trackers.at(index); }

	size_t getNTrackers( void ) const
	{ return _trackers.size(); }

	vl::EventManagerPtr getEventManager( void )
	{ return _event_manager; }

protected :
	std::vector<TrackerRefPtr> _trackers;

	vl::EventManagerPtr _event_manager;
};

}	// namespace vl

#endif
