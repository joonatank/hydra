/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11 major update to new event handling system
 *
 */
#include "vrpn_tracker.hpp"

#include "base/exceptions.hpp"

/// Necessary for log levels
#include "logger.hpp"

/// -------------- Globals -------------
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

void VRPN_CALLBACK vl::handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
	vl::vrpnTracker *tracker = (vl::vrpnTracker *)userdata;
	tracker->update(t);
}


/// ----------- vrpnTracker ------------

/// ----------- Public ----------------
vl::vrpnTracker::~vrpnTracker(void)
{}

void
vl::vrpnTracker::mainloop(void )
{
	if( !_tracker )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_tracker->mainloop();
}

/// -------- Protected -------------
vl::vrpnTracker::vrpnTracker(const std::string& trackerName )
	: Tracker(trackerName)
{
	_create(trackerName.c_str());
}

vl::vrpnTracker::vrpnTracker(const std::string& hostname,
							 const std::string& tracker,
							 uint16_t port)
	: Tracker(tracker)
{
	std::stringstream ss;
	ss << tracker << "@" << hostname;

	// Add port if it's not default value
	if( port != 0 )
	{ ss << ":" << port; }

	_create(ss.str().c_str());
}

void 
vl::vrpnTracker::_create(char const *tracker_name)
{
	std::cout << vl::TRACE << "Creating vrpn tracker : " << tracker_name << std::endl;

	_tracker.reset( new vrpn_Tracker_Remote(tracker_name) );
	_tracker->shutup = true;
	_tracker->register_change_handler(this, vl::handle_tracker);

}

void
vl::vrpnTracker::update( vrpn_TRACKERCB const t )
{
	// Only update sensors that the user has created and added
	if( _sensors.size() > t.sensor )
	{
		TrackerSensor &sensor = _sensors.at(t.sensor);

		vrpn_float64 quat[4];
		if( incorrect_quaternion )
		{
			quat[Q_W] = t.quat[Q_X];
			quat[Q_X] = t.quat[Q_Y];
			quat[Q_Y] = t.quat[Q_Z];
			quat[Q_Z] = t.quat[Q_W];
		}
		else
		{
			quat[Q_X] = t.quat[Q_X];
			quat[Q_Y] = t.quat[Q_Y];
			quat[Q_Z] = t.quat[Q_Z];
			quat[Q_W] = t.quat[Q_W];
		}

		vl::Transform trans = vl::createTransform( t.pos, quat);
		if(trans.isValid())
		{
			trans = trans*_transform;
			sensor.update( trans );
		}
	}
}
