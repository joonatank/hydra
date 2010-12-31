/**	Joonatan Kuosa
 *	2010-05 initial implementation
 *	2010-11 removed mapping and added Trigger support
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

#include "math/conversion.hpp"

#include <vrpn_Tracker.h>

#include <string>
#include <cstring>

#include <iostream>
#include <stdint.h>

namespace vl
{

void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

/// Creates an Transformation from vrpn data
/// transformation is stored in same maner as in Ogre as oposed to vrpn quaternion
/// starts with w.
inline vl::Transform
createTransform( vrpn_float64 const *pos, vrpn_float64 const *quat )
{
	return Transform( vl::math::convert_vec(pos), vl::math::convert_quat(quat) );
}


class vrpnTracker : public vl::Tracker
{
public :
	/// Construct a tracker from vrpn type of tracker name tracker@hostname:port
	vrpnTracker( std::string const &trackerName );

	/// Construct a tracker from tracker, hostname, port tuple
	vrpnTracker( std::string const &hostname, std::string const &tracker, uint16_t port = 0 );

	/// Destructor
	virtual ~vrpnTracker( void );

	virtual void init( void );

	/// Called once in an iteration from main application
	virtual void mainloop( void );

protected :
	// Callback function
	/// Updates only sensors that are in use
	void update( vrpn_TRACKERCB const t );

	vrpn_Tracker_Remote *_tracker;

private :
	// For access to udpate function
	friend void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

};	// class vrpnTracker


}	// namespace vl

#endif