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

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
//#include <eq/client/observer.h>

#include <iostream>

namespace vl
{
	void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

struct SensorData
{
	SensorData( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO, 
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: position( pos ), quaternion( rot )
	{}

	SensorData( vrpn_float64 const *pos, vrpn_float64 const *quat )
		: position( pos[0], pos[1], -pos[2] ),
		  quaternion( quat[3], quat[0], quat[1], quat[2] )
	{}

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

std::ostream &operator<<( std::ostream &os, SensorData const &d );

class vrpnTracker : public vl::Tracker
{
public :
	// Construct a tracker from vrpn type of tracker name tracker@hostname:port
	vrpnTracker( std::string const &trackerName );

	~vrpnTracker( void );

	void init( void )
	{
		_tracker->register_change_handler(this, vl::handle_tracker);
	}

	size_t getNSensors( void )
	{ return _data.size(); }

	Ogre::Vector3 const &getPosition( size_t sensor ) const;
	Ogre::Quaternion const &getOrientation( size_t sensor ) const;

	// Map the SceneNode position and orientation to the tracker values
	// Will overwrite any existing values
	void map( Ogre::SceneNode *node );

	// Map the observer position and orientation to the tracker values
	// Will overwrite any existing values
	// To map this to Ogre::Camera just retrieve the head matrix
//	void map( eq::Observer *observer );

	// Called once in an iteration from main application
	void mainloop( void );

protected :
	// Callback function
	void update( vrpn_TRACKERCB const t );

	std::vector<SensorData> _data;
	
	vrpn_Tracker_Remote *_tracker;

private :
	// For access to udpate function
	friend void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

};	// class vrpnTracker


}	// namespace vl

#endif