
#include "vrpn_tracker.hpp"

vl::vrpnTracker::vrpnTracker(const std::string& trackerName, const std::string& hostname, unsigned int port)
{

}

Ogre::Quaternion const &
vl::vrpnTracker::getOrientation(size_t sensor) const
{
	return Ogre::Quaternion::IDENTITY;
}

Ogre::Vector3 const &
vl::vrpnTracker::getPosition(size_t sensor) const
{
	return Ogre::Vector3::ZERO;
}

void 
vl::vrpnTracker::mainloop(void )
{

}

void 
vl::vrpnTracker::map(Ogre::SceneNode* node)
{

}

/*
void 
vl::vrpnTracker::map(eq::Observer* observer)
{

}
*/

void 
vl::vrpnTracker::update(const vl::SensorData& data)
{

}
