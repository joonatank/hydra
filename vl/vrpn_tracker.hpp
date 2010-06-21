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
	vrpnTracker( std::string const &hostname = std::string(), unsigned int port = 0);

	Ogre::Vector3 const &getPosition( size_t sensor );
	Ogre::Quaternion const &getOrientation( size_t sensor );

	// TODO we should only set either observer or camera
	void map( Ogre::SceneNode *node );

	void map( eq::Observer *observer );

protected :
	std::vector<SensorData> _data;

};	// class Tracker

}	// namespace vl

#endif