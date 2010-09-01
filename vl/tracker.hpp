
/*	Joonatan Kuosa
 *	2010-05 initial
 *	2010-06 some meat
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

class Tracker
{
public :
	virtual void init( void ) = 0;

	virtual size_t getNSensors( void ) const = 0;

	virtual Ogre::Vector3 const &getPosition( size_t sensor ) const = 0;

	virtual Ogre::Quaternion const &getOrientation( size_t sensor ) const = 0;

	virtual void mainloop( void ) = 0;

};	// class Tracker

class TrackerSerializer
{
	void read( char *xml_data );
};

}	// namespace vl

#endif