/**	Joonatan Kuosa
 *	2010-11 major update to new event handling system
 *
 */
#include "vrpn_tracker.hpp"

#include "base/exceptions.hpp"

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
vl::vrpnTracker::vrpnTracker(const std::string& trackerName )
	: _tracker(0)
{
	// TODO should use Ogre LogManager
	// Needs to be created in Config
	std::cout << "Creating vrpn tracker : " << trackerName << std::endl;
	_tracker = new vrpn_Tracker_Remote( trackerName.c_str() );
	_tracker->shutup = true;
}

vl::vrpnTracker::vrpnTracker(const std::string& hostname,
							 const std::string& tracker,
							 uint16_t port)
	: _tracker(0)
{
	std::stringstream ss;
	ss << tracker << "@" << hostname;

	// Add port if it's not default value
	if( port != 0 )
	{ ss << ":" << port; }

	// TODO should use Ogre LogManager
	// Needs to be created in Config
	std::cout << "Creating vrpn tracker : " << ss.str() << std::endl;

	_tracker = new vrpn_Tracker_Remote( ss.str().c_str() );
	_tracker->shutup = true;
}

vl::vrpnTracker::~vrpnTracker( void )
{
	delete _tracker;
}

void vl::vrpnTracker::init(void )
{
	_tracker->register_change_handler(this, vl::handle_tracker);
}

void
vl::vrpnTracker::mainloop(void )
{
	if( !_tracker )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_tracker->mainloop();
}

/// -------- Protected -------------
void
vl::vrpnTracker::update( vrpn_TRACKERCB const t )
{
	// Only update sensors that the user has created and added
	if( _sensors.size() > t.sensor )
	{
		SensorRefPtr sensor = _sensors.at(t.sensor);
		// Check that we have a sensor object
		// Only sensors that are in use have an object
		if( sensor )
		{
			vl::Transform trans = vl::createTransform( t.pos, t.quat );
			trans = trans * _transform;
			std::cerr << "updating sensor : " << sensor->getTrigger()->getName() 
				<< " transform = " << trans << std::endl << std::endl;
			sensor->update( trans );
		}
	}
}
