#ifndef VL_FAKE_TRACKER_HPP
#define VL_FAKE_TRACKER_HPP

#include "tracker.hpp"

#include <vector>

namespace vl
{

class FakeTracker : public Tracker
{
public :
	FakeTracker( void );

	virtual ~FakeTracker( void );

	virtual void init( void );

	virtual size_t getNSensors( void ) const;

	virtual Ogre::Vector3 const &getPosition( size_t sensor ) const;

	virtual Ogre::Quaternion const &getOrientation( size_t sensor ) const;

	// Empty function
	virtual void mainloop( void );

	// Set some constant values for sensors
	// Will expand the data array to contain the sensor number passed
	void setPosition( size_t sensor, Ogre::Vector3 const &pos );
	void setOrientation( size_t sensor, Ogre::Quaternion const &quat );

protected :
	std::vector<SensorData> _data;
};

}

#endif