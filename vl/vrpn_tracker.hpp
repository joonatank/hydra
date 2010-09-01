/*	Joonatan Kuosa
 *	2010-05
 *
 *	VRPN Tracker implementation.
 *	
 *	Has add callback method which user can use to add a callback functor for
 *	specific sensors.
 */
#ifndef VL_VRPN_TRACKER_HPP
#define VL_VRPN_TRACKER_HPP

#ifdef VL_WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#include "tracker.hpp"

#include <vrpn_Tracker.h>

#include <string>
#include <cstring>

#include <iostream>

namespace vl
{
	void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

struct vrpnSensorData : public SensorData
{
	vrpnSensorData( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO, 
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: SensorData( pos, rot )
	{}

	// TODO remove hard-coded permutation and flipping
	vrpnSensorData( vrpn_float64 const *pos, vrpn_float64 const *quat )
		: SensorData( Ogre::Vector3( pos[0], pos[1], -pos[2] ), 
					  Ogre::Quaternion( quat[3], quat[0], quat[1], quat[2] ) )
	{}
};

std::ostream &operator<<( std::ostream &os, SensorData const &d );

class vrpnTracker : public vl::Tracker
{
public :
	// Construct a tracker from vrpn type of tracker name tracker@hostname:port
	vrpnTracker( std::string const &trackerName );

	virtual ~vrpnTracker( void );

	virtual void init( void )
	{
		_tracker->register_change_handler(this, vl::handle_tracker);
	}

	virtual size_t getNSensors( void ) const
	{ return _data.size(); }

	virtual Ogre::Vector3 const &getPosition( size_t sensor ) const;
	virtual Ogre::Quaternion const &getOrientation( size_t sensor ) const;

	// Map the SceneNode position and orientation to the tracker values
	// Will overwrite any existing values
	void map( Ogre::SceneNode *node );

	// Map the observer position and orientation to the tracker values
	// Will overwrite any existing values
	// To map this to Ogre::Camera just retrieve the head matrix
//	void map( eq::Observer *observer );

	// Called once in an iteration from main application
	virtual void mainloop( void );

	/// Sensor values used when not connected to a tracker
	/// Uses different sensor data stack, so if connection is lost for long it can be
	/// defaulted to these values.
	/// For now supports default values if no vrpn connection is found
	/// TODO we need to check if we have lost a connection and reset the sensor data then
	virtual void setPosition( size_t sensor, Ogre::Vector3 const &pos );
	virtual void setOrientation( size_t sensor, Ogre::Quaternion const &quat );

protected :
	// Callback function
	void update( vrpn_TRACKERCB const t );

	std::vector<vrpnSensorData> _data;
	std::vector<vrpnSensorData> _default_values;
	
	vrpn_Tracker_Remote *_tracker;

private :
	// For access to udpate function
	friend void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

};	// class vrpnTracker


}	// namespace vl

#endif