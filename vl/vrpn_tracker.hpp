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

#include "tracker.hpp"

#include <vrpn_Tracker.h>

#include <string>
#include <cstring>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <eq/client/observer.h>

namespace vl
{

void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

struct SensorData
{
	SensorData( Ogre::Vector3 const &pos, Ogre::Quaternion const &rot )
		: position( pos ), quaternion( rot )
	{}

	SensorData( vrpn_float64 *pos, vrpn_float64 *quat )
		: position( pos[0], pos[1], pos[2] ),
		  quaternion( quat[3], quat[0], quat[1], quat[2] )
	{}

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

class vrpnTracker : public vl::Tracker
{
public :
	vrpnTracker( std::string const &trackerName, std::string const &hostname, unsigned int port = 0);

	Ogre::Vector3 const &getPosition( size_t sensor ) const;
	Ogre::Quaternion const &getOrientation( size_t sensor ) const;

	// Map the SceneNode position and orientation to the tracker values
	// Will overwrite any existing values
	void map( Ogre::SceneNode *node );

	// Map the observer position and orientation to the tracker values
	// Will overwrite any existing values
	// To map this to Ogre::Camera just retrieve the head matrix
	void map( eq::Observer *observer );

	// Called once in an iteration from main application
	void mainloop( void );

	// Callback function
	void update( SensorData const &data );

protected :
	std::vector<SensorData> _data;
	
	vrpnTracker *_tracker;

};	// class vrpnTracker

}	// namespace vl

#endif