/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-01
 *	@file eye_tracker.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
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
	/// @todo this will throw an exception if compiled
	/// with Eyes but without setting the settings
	/// we want it to only throw exception if user tries to use
	/// EyeTracking without configuring.
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

	/// @brief load a recording from file to memory
	/// Does not automatically show the recording, only one recording can be loaded at one time.
	/// If param filename is empty uses the recording from current run.
	void loadRecording(std::string const &finename);

	void saveRecording(std::string const &filename);

	/// @brief Show recording instead of real time data
	/// Automatically disables real time eye tracking.
	void showRecording(bool show);

	bool isRecordingShown(void) const;

	RayObject *getRay(void)
	{ return _drawable; }

	void setDebug(bool debug)
	{ _debug = debug; }

	bool isDebug(void) const
	{ return _debug; }

	void disableHead(bool disable_head)
	{ _head_disabled = disable_head; }

	bool isHeadDisabled(void) const
	{ return _head_disabled; }

	void printInit(void);

private :
	void _update_head(void);
#ifdef USE_EYES
	boost::scoped_ptr<eyes::EyeTracker> _eyes;
	eyes::Report _loaded_report;
#endif

	PlayerPtr _player;

	// What we own
	SceneNodePtr _node;
	RayObject *_drawable;

	SceneManager *_creator;

	bool _started;
	bool _debug;
	bool _head_disabled;

};

inline
std::ostream &operator<<(std::ostream &os, EyeTracker const &e)
{
	os << "EyeTracker : ";
	
	return os;
}

}	// namespace vl

#endif // HYDRA_EYE_TRACKER_HPP
