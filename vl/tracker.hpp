
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

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "eq_cluster/event.hpp"
#include "eq_cluster/scene_node.hpp"

namespace vl
{

struct SensorData
{
	SensorData( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO,
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: position( pos ), quaternion( rot )
	{}

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

/// Callback Action class designed for Trackers
/// Could be expanded for use with anything that sets the object transformation
// For now the Tracker Triggers are the test case
class TrackerAction : public eqOgre::Operation
{
public :
	/// Callback function for TrackerTrigger
	/// Called when new data is received from the tracker
	virtual void execute( SensorData const &data ) = 0;

private :
	// Forbid execute as this action is used with callbacks which take parameters
	virtual void execute( void ) {}

};

typedef TrackerAction * TrackerActionPtr;


/// Move a scene node callback
// TODO should be moved to SceneNode events
class NodeTrackerAction : public vl::TrackerAction
{
public :
	void setSceneNode( eqOgre::SceneNode *node )
	{ _node = node; }

	eqOgre::SceneNode *getSceneNode( void )
	{ return _node; }

	/// Callback function for TrackerTrigger
	/// Called when new data is received from the tracker
	virtual void execute( SensorData const &data );

	virtual std::string const &getTypeName( void ) const;

private :
	// Forbid execute as this action is used with callbacks which take parameters
	virtual void execute( void ) {}

	eqOgre::SceneNode *_node;
};


/// TrackerAction Factory class for creating new Actions
class NodeTrackerActionFactory
{
public :
	virtual eqOgre::Operation *create( void )
	{ return new NodeTrackerAction; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


/// This is not an abstract class because for all the trackers we have the
/// Same kind of Trigger so they can use the same one.
class TrackerTrigger : public eqOgre::Trigger
{
public :

	/// --------- Overrides from eqOgre::Trigger ------------
	/// This class is not a specialization of anything always returns false
	virtual bool isSpecialisation( Trigger const *other ) const
	{ return false; }

	// NOTE problematic to say the least... we return 0 for now
	virtual double value( void ) const
	{ return 0; }

	/// Two triggers are equal if they have the same name
	/// Actions they execute don't matter.
	virtual bool isEqual( eqOgre::Trigger const &other ) const
	{
		TrackerTrigger const &a = (TrackerTrigger const &)other;
		return( a._name == _name );
	}

	virtual std::string const &getTypeName( void ) const;

	/// -------------- Custom Methods ----------------

	/// Name is used for creation from XML and mapping from python
	void setName( std::string const &name )
	{ _name = name; }

	std::string const &getName( void ) const
	{ return _name; }

	/// Action to execute when updated
	void setAction( TrackerActionPtr action );

	/// Callback function
	void update( SensorData const &data );

protected :
	std::string _name;

	TrackerActionPtr _action;

};

class TrackerTriggerFactory : public eqOgre::TriggerFactory
{
public :
	virtual eqOgre::Trigger *create( void )
	{ return new TrackerTrigger; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


class Sensor
{
public :
	Sensor( Ogre::Vector3 const &default_pos = Ogre::Vector3::ZERO,
			Ogre::Quaternion const &default_quat = Ogre::Quaternion::IDENTITY )
		: _default_value( default_pos, default_quat )
	{}

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
	virtual void update( SensorData const &data );

protected :
	vl::TrackerTrigger *_trigger;

	SensorData _default_value;

};

typedef boost::shared_ptr<Sensor> SensorRefPtr;


class Tracker
{
public :
	virtual void init( void ) = 0;

	virtual void mainloop( void ) = 0;

	/// Set a sensor created else where. Sensors should be created to the heap
	/// So they stay alive as long as the tracker.
	// TODO where are the sensors created?
	// Should they be created here or in the calling code?
	virtual void setSensor( size_t i, SensorRefPtr sensor );

	virtual SensorRefPtr getSensor( size_t i );

protected :
	std::vector<SensorRefPtr> _sensors;

};	// class Tracker


}	// namespace vl

#endif