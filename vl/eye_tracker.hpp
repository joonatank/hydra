/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-01
 *	@file eye_tracker.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_EYE_TRACKER_HPP
#define HYDRA_EYE_TRACKER_HPP

#ifdef USE_EYES
#include "eyes.hpp"
#endif

#include "typedefs.hpp"

namespace vl
{

/** @class EyeTracker
 *	Manage the Eye tracking plugin
 *	Request new data, start and stop, recalibrate etc.
 *	
 *	Manage the drawable object
 *	the drawable is RayObject
 *	- Create it when we are created
 *	- Destroy it when we are destroyed
 *	- Move it when new data is received
 *	- Hide it when we are stopped
 *	
 *	Later manage the recordings
 *
 *	Needs to be created after Scene Manager
 *	Needs to be only created if the Eyes plugin can be linked to
 *	i.e. we need a compile time definion to exclude this object.
 *
 */
class EyeTracker
{
public :
	/// @brief constructor
	/// Start state is always stop and no drawable will be shown
	/// @todo we need to pass player here so we can retrieve current
	/// camera and current head transformation
	EyeTracker(PlayerPtr player, SceneManager *creator);

	/// @brief Destructor
	/// @todo add deregistering and destroying of the drawable
	~EyeTracker(void) {}

	// Start progressing the tracking
	void start(void);

	// Stop progressing the EyeTracker and hide all drawables
	void stop(void);

	bool isStarted(void) const
	{ return _started; }

	// Necessary for progressing EyeTracker
	// Needs to be called at each frame
	void progress(void);

private :
#ifdef USE_EYES
	boost::scoped_ptr<eyes::EyeTracker> _eyes;
#endif

	PlayerPtr _player;

	// What we own
	SceneNodePtr _node;
	RayObject *_drawable;

	SceneManager *_creator;

	bool _started;

};

inline
std::ostream &operator<<(std::ostream &os, EyeTracker const &e)
{
	os << "EyeTracker : ";
	
	return os;
}

}	// namespace vl

#endif // HYDRA_EYE_TRACKER_HPP
