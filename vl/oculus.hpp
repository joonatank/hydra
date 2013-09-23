/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file oculus.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_OCULUS_HPP
#define HYDRA_OCULUS_HPP

#include <OVR.h>

#include "base/rect.hpp"
#include "math/types.hpp"

#include <OGRE/OgreVector2.h>

#include "base/config.hpp"

// For testing
#include <Util/Util_Render_Stereo.h>

namespace vl
{

inline
Ogre::Matrix4 convert_matrix(OVR::Matrix4f const &m)
{
	Ogre::Matrix4 m1;
	for(size_t i = 0; i < 4; ++i)
	{
		for(size_t j = 0; j < 4; ++j)
		{
			m1[i][j] = m.M[i][j];
		}
	}

	return m1;
}

// @todo we should move this to another file so we don't need to
// include the whole Occulus for env config.
// we need to send this to Renderer anyway (Slaves)
struct HMDInfo
{
	std::string device_name;
	float ipd;
	// For creating the window
	Rect<int> resolution;
	// For calculating the frustum
	float EyeToScreenDistance;
	Ogre::Vector2 screen_size;
	Ogre::Vector2 screen_center;
	// something?
	float LensSeparationDistance;
	// For pixel shader
	float distortionK[4];
};

std::ostream &operator<<(std::ostream &os, HMDInfo const &info);

// distortion info should not need special overloads for serialization
// if it's changed to include types that use pointers we need to implement
// serialization methods.
struct DistortionInfo
{
	// empty constructor for now
	// should not matter that we don't have default values since it should only
	// be used if it's initialised from Oculus
	DistortionInfo(void) : enabled(true) {}

	// Distortion parameters, based on the lense used
	Ogre::Vector4 K;
	// no idea
	Ogre::Vector4 chromatic_aberration;
	// lens offset
	vl::scalar x_center_offset;
	vl::scalar y_center_offset;
	// how much the image needs to be scaled because the distortion filter
	// makes the image smaller than the rendering viewport.
	vl::scalar scale;
	// Is distortion enabled or not, defaults to on
	bool enabled;
};

std::ostream &operator<<(std::ostream &os, DistortionInfo const &info);

class Oculus : public OVR::MessageHandler
{
public :
	Oculus(void);

	~Oculus(void);

	
	Ogre::Quaternion getOrientation(void) const;
	Ogre::Vector3 getAcceleration(void) const;
	// @todo position is not implemented (always returns zero)
	// @todo we need the time difference for implementing position
	Ogre::Vector3 getPositon(void) const;

	HMDInfo const &getInfo(void) const
	{ return _info; }

	DistortionInfo const &getDistortionInfo(void) const
	{ return _distortion; }

	// For testing
	OVR::Util::Render::StereoConfig const &getStereoConfig(void) const
	{ return _sconfig; }

	void copyConfig(vl::config::Channel &channel);

private :
	void _initialise_sconfig(OVR::HMDInfo const &info);

	HMDInfo _info;
	DistortionInfo _distortion;

	OVR::Ptr<OVR::DeviceManager> _manager;
	OVR::Ptr<OVR::SensorDevice> _sensor;
	OVR::Ptr<OVR::HMDDevice> _hmd;
	OVR::SensorFusion sfusion;
	OVR::Util::Render::StereoConfig _sconfig;

};	// class Oculus

}	// namespace vl

#endif // HYDRA_OCULUS_HPP
