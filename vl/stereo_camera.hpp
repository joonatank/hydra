/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-04
 *	@file stereo_camera.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
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

	void setCamera(vl::CameraPtr cam);

	vl::CameraPtr getCamera(void) const
	{ return _camera; }

	/// @brief updates the Ogre::Camera's projection and view matrices
	/// @param eye_x the eye x direction used for this rendering
	/// For stereo rendering this needs to be called twice with different
	/// eye_x values. Also you need to call Viewport::update also.
	void update(STEREO_EYE eye);

	void setIPD(vl::scalar ipd)
	{ _ipd = ipd; }

	vl::scalar getIPD(void) const
	{ return _ipd; }

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

};	// class StereoCamera

}

#endif	// HYDRA_STEREO_CAMERA_HPP
