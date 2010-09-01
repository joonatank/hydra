
#include "vrpn_tracker.hpp"

#include "base/exceptions.hpp"

std::ostream &operator<<( std::ostream &os, vrpn_TRACKERCB t )
{
	os << "Sensor = " << t.sensor << " : position = (";
	for( size_t i = 0; i < 3; ++i )
	{ os << t.pos[i] << ", "; }
	os << ")" << " : quaternion (";
	for( size_t i = 0; i < 4; ++i )
	{ os << t.quat[i] << ", "; }

	return os;
}

std::ostream & vl::operator<<( std::ostream &os, vl::SensorData const &d )
{
	os << "Position = " << d.position << " : Orientation = " << d.quaternion;

	return os;
}

void VRPN_CALLBACK vl::handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
	vl::vrpnTracker *tracker = (vl::vrpnTracker *)userdata;
	//std::cout << "Callback called : " << t << std::endl;
	tracker->update(t);
}

vl::vrpnTracker::vrpnTracker(const std::string& trackerName )
{
	_tracker = new vrpn_Tracker_Remote( trackerName.c_str() );
}

vl::vrpnTracker::~vrpnTracker( void )
{
	delete _tracker;
}

Ogre::Quaternion const &
vl::vrpnTracker::getOrientation(size_t sensor) const
{
	return _data.at(sensor).quaternion;
}

Ogre::Vector3 const &
vl::vrpnTracker::getPosition(size_t sensor) const
{
	return _data.at(sensor).position;
}

void 
vl::vrpnTracker::mainloop(void )
{
	if( !_tracker )
	{ throw vl::exception(); }

	_tracker->mainloop();
}

void 
vl::vrpnTracker::map(Ogre::SceneNode* node)
{
	// TODO
}

void 
vl::vrpnTracker::update( vrpn_TRACKERCB const t )
{
	if( _data.size() <= t.sensor )
	{
		_data.resize( t.sensor+1 ); 
	}

	_data.at(t.sensor) = vl::vrpnSensorData( t.pos, t.quat );
}
