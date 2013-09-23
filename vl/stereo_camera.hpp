/**
 *	Copyright (c) 2012 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-04
 *	@file stereo_camera.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 */

#ifndef HYDRA_STEREO_CAMERA_HPP
#define HYDRA_STEREO_CAMERA_HPP

// Necessary for camera type
#include "camera.hpp"
// Necessary for Transform and scalar
#include "math/types.hpp"
// Necessary for frustum type used in projection calculation
#include "math/frustum.hpp"

namespace vl
{

enum STEREO_EYE
{
	HS_UNDEFINED,
	HS_MONO,
	HS_LEFT,
	HS_RIGHT,
};

inline
std::string stereo_eye_to_string(STEREO_EYE cfg)
{
	switch(cfg)
	{
	case HS_UNDEFINED:
		return "undefined";
	case HS_MONO:
		return "mono";
	case HS_LEFT:
		return "left";
	case HS_RIGHT:
		return "right";
	default :
		return "";
	}
}

/// @todo this should be implemented as an Ogre Movable object
/// a replacement for the Ogre::Camera class
/// Well I think we could be faster with it if we just replaced
/// our camera with it.
/// - We could easily distribute head and ipd (update them in master)
/// - Remove Player from distribution stack
///		(use Pipe/Window for Screenshotting)
/// Then later if we need we can refactor this to distribution and rendering
/// i.e. Hydra and Ogre implementations.
/// 
/// before rendering it should be updated with 
///		head matrix
///		cyclop transformation
///		ipd
class StereoCamera
{
public :
	StereoCamera(void);

	~StereoCamera(void);

	void setHead(vl::Transform const &head);

	/// Camera contains the position without head tracking
	/// i.e it's actually the users feet.
	void setCamera(vl::CameraPtr cam);

	vl::CameraPtr getCamera(void) const
	{ return _camera; }

	/// Return the view transform (view/eye matrix) for this frame
	/// returns a view for a random eye but with correct head and wall transforms
	vl::Transform getViewTransform(void) const;

	/// @brief updates the Ogre::Camera's projection and view matrices
	/// @param eye_x the eye x direction used for this rendering
	/// For stereo rendering this needs to be called twice with different
	/// eye_x values. Also you need to call Viewport::update also.
	///
	/// @todo this STEREO_EYE is silly
	/// replace with update_mono, update_left, update_right
	/// or update(vl::scalar eye)
	/// removes an unnecessary comparison from the rendering
	/// and cleans the update function.
	void update(STEREO_EYE eye);

	void setIPD(vl::scalar ipd)
	{ _ipd = ipd; }

	vl::scalar getIPD(void) const
	{ return _ipd; }

	/// @brief Is the frustum for HMD or not.
	/// Used for testing what rendering function to use.
	void enableHMD(bool enable)
	{ _is_hmd = enable; }

	bool isHMD(void) const
	{ return _is_hmd; }

	vl::Frustum &getFrustum(void)
	{ return _frustum; }

	vl::Frustum const &getFrustum(void) const
	{ return _frustum; }

	void setFrustum(vl::Frustum const &f)
	{ _frustum = f; }

private :
	vl::CameraPtr _camera;

	Ogre::Camera *_ogre_camera;

	vl::Transform _head;

	vl::Frustum _frustum;

	vl::scalar _ipd;

	// flag used to check if we are calculating projection for Oculus
	bool _is_hmd;

};	// class StereoCamera

}

#endif	// HYDRA_STEREO_CAMERA_HPP
