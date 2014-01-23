/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-05
 *	@file vrpn_tracker.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**
 *	VRPN Tracker implementation.
 *
 *	Has add callback method which user can use to add a callback functor for
 *	specific sensors.
 */

#ifndef HYDRA_VRPN_TRACKER_HPP
#define HYDRA_VRPN_TRACKER_HPP

#ifdef _WIN32
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

#include <boost/scoped_ptr.hpp>

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
	/// Destructor
	virtual ~vrpnTracker( void );

	/// Called once in an iteration from main application
	virtual void mainloop( void );

	static TrackerRefPtr create(std::string const &trackerName)
	{ return TrackerRefPtr(new vrpnTracker(trackerName)); }

	static TrackerRefPtr create(std::string const &hostname, std::string const &tracker, uint16_t port = 0)
	{ return TrackerRefPtr(new vrpnTracker(hostname, tracker, port)); }

protected :
	/// Protected constructors use create instead
	/// Construct a tracker from vrpn type of tracker name tracker@hostname:port
	vrpnTracker(std::string const &trackerName);

	/// Construct a tracker from tracker, hostname, port tuple
	vrpnTracker(std::string const &hostname, std::string const &tracker, uint16_t port);

	void _create(char const *tracker_name);

	// Callback function
	/// Updates only sensors that are in use
	void update( vrpn_TRACKERCB const t );

	boost::scoped_ptr<vrpn_Tracker_Remote> _tracker;

private :
	// For access to udpate function
	friend void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

};	// class vrpnTracker


}	// namespace vl

#endif	// HYDRA_VRPN_TRACKER_HPP
