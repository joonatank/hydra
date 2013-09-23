/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file oculus.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "oculus.hpp"

#include "base/exceptions.hpp"

/// -------------------------------- Global ----------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::HMDInfo const &info)
{
	os << "HMD device : " << info.device_name
		<< " resolution = " << info.resolution
		<< " Screen size = " << info.screen_size
		<< " Screen center = " << info.screen_center
		<< " IPD = " << info.ipd
		<< " Eye to Screen= " << info.EyeToScreenDistance
		<< " Lens separtion = " << info.LensSeparationDistance
		<< " Distortion : [" << info.distortionK[0] << ", " << info.distortionK[1] << ", " 
		<< info.distortionK[2] << ", " << info.distortionK[3] << "]";

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, DistortionInfo const &info)
{
	os << "K = " << info.K << std::endl
		<< "chromatic_aberration = " << info.chromatic_aberration << std::endl
		<< "X center offset = " << info.x_center_offset 
		<< " : Y center offset = " << info.y_center_offset
		<< " : scale = " << info.scale << std::endl;
	return os;
}


/// -------------------------------- Oculus ----------------------------------
vl::Oculus::Oculus(void)
	: _manager(0)
	, _hmd(0)
{
	std::clog << "vl::Oculus::Oculus" << std::endl;
	OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

	// @todo save the device pointer as a member
	// @todo check that we actually have a device
	// if we don't have one connected we should modify the env config
	// @todo do we need a debug option is enabled in env config 
	// that allows us to test the Oculus without having one connected?
	// or should we just have one connected and be able to redirect the
	// rendering result to normal window or duplicate it?
	// @todo we should modify env config anyway here with the values
	// gotten from the device info. This is pretty standard feature and
	// there is no point in not implementing it.
	// For Oculus it's straigth forward since there is no point in
	// using anyother values than the native.
	// For other devices it would provide good default values
	// instead of hard coded (using primary device and max resolution)
	_manager = *OVR::DeviceManager::Create();
	assert(_manager);
	// returns zero if hmd is not connected
	// @todo do we throw here?
	_hmd = *_manager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
	if(!_hmd)
	{
		// @todo add proper exception
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Failed to retrieve Oculus device."));
	}

	// Copy HMD info
	// @todo make this a free function
	OVR::HMDInfo hmd_info;
	if (_hmd->GetDeviceInfo(&hmd_info))
	{
		_info.device_name = hmd_info.DisplayDeviceName;
		_info.ipd = hmd_info.InterpupillaryDistance;
		// For creating the window
		// w = width = horizontal
		// h = height = vertical
		_info.resolution.w = hmd_info.HResolution;
		_info.resolution.h = hmd_info.VResolution;
		_info.resolution.x = hmd_info.DesktopX;
		_info.resolution.y = hmd_info.DesktopY;
		// For calculating the frustum
		_info.EyeToScreenDistance = hmd_info.EyeToScreenDistance;
		_info.screen_size.x = hmd_info.HScreenSize;
		_info.screen_size.y = hmd_info.VScreenSize;
		_info.screen_center.x = 0;
		_info.screen_center.y = hmd_info.VScreenCenter;
		// something?
		_info.LensSeparationDistance = hmd_info.LensSeparationDistance;
		// For pixel shader
		_info.distortionK[0] = hmd_info.DistortionK[0];
		_info.distortionK[1] = hmd_info.DistortionK[1];
		_info.distortionK[2] = hmd_info.DistortionK[2];
		_info.distortionK[3] = hmd_info.DistortionK[3];
	}

	if(_hmd)
	{ _sensor = *_hmd->GetSensor(); }

	assert(_sensor);
	
	sfusion.AttachToSensor(_sensor);
	// These are used in the examples do we need them and why?
	// also the class needs to be inherited from OVR::MessageHandler
	sfusion.SetDelegateMessageHandler(this);
	sfusion.SetPredictionEnabled(true);

	// Use yaw correction since to get as accurate tracking as possible.
	// We need to calibrate the magnetometer for this though
	// need to check how often it needs to be done etc.
	// probably adding a python function for it after documenting it.
	//MagCal.BeginAutoCalibration(SFusion);
	//SetAdjustMessage("Starting Auto Mag Calibration");
	//sfusion.SetYawCorrectionEnabled(true);

	// Create stereo config for testing
	_initialise_sconfig(hmd_info);
}

vl::Oculus::~Oculus(void)
{
	// Call the inherited cleanup function
	RemoveHandlerFromDevices();
	// Remove OVR devices
	_sensor.Clear();
	_hmd.Clear();
	// do we need to clear manager?
	_manager.Clear();
}


Ogre::Quaternion
vl::Oculus::getOrientation(void) const
{
	// If sfusion is not defined or the sensor it's attached to is destroyed
	// it returns identity quaternion.
	OVR::Quatf q = sfusion.GetOrientation();
	return Ogre::Quaternion(q.w, q.x, q.y, q.z);
}

Ogre::Vector3
vl::Oculus::getAcceleration(void) const
{
	// returns zero if sfusion is not connected
	OVR::Vector3f const &acc = sfusion.GetAcceleration();
	return Ogre::Vector3(acc.x, acc.y, acc.z);
}

Ogre::Vector3
vl::Oculus::getPositon(void) const
{
	return Ogre::Vector3::ZERO;
}

void
vl::Oculus::copyConfig(vl::config::Channel &channel)
{
	// @todo this stuff is cool but it should not be here
	// These are interesting for Window creation, more of convenience
	// functions than anything we need now.
	//RenderParams.MonitorName = HMDInfo.DisplayDeviceName;
	//RenderParams.DisplayId = HMDInfo.DisplayId;

	// @todo most of this should be in Oculus and we should just copy the
	// matrices here
	//
	// Set the custom oculus projection matrix
	// because we get wrong results with our own
	// Oculus projection matrices seem to work
	// well at least they are usable unlike ours.
	//OVR::Util::Render::StereoConfig sconfig = _game_manager->getOculus()->getStereoConfig();
	// We need to use the Rift SDK for the Projection here

	// from here we get the projection matrix
	// Convert Oculus matrices to Ogre
	OVR::Util::Render::StereoEyeParams p = _sconfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);			
	channel.user_projection_left = convert_matrix(p.Projection);
	p = _sconfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
	channel.user_projection_right = convert_matrix(p.Projection);
}

/// --------------------------- Private --------------------------------------
void
vl::Oculus::_initialise_sconfig(OVR::HMDInfo const &info)
{
	// @todo
	// should this stuff be in the Window and completely bypass
	// the regular rendering
	// at least for testing.
	_sconfig.SetHMDInfo(info);
	vl::scalar width = info.HResolution;
	vl::scalar height = info.VResolution;
	_sconfig.SetFullViewport(OVR::Util::Render::Viewport(0,0, width, height));
	_sconfig.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
	// Get the stereo config

	// Configure proper Distortion Fit.
	// For 7" screen, fit to touch left side of the view, leaving a bit of invisible
	// screen on the top (saves on rendering cost).
	// For smaller screens (5.5"), fit to the top.
	if (info.HScreenSize > 0.0f)
	{
		if (info.HScreenSize > 0.140f) // 7"
			_sconfig.SetDistortionFitPointVP(-1.0f, 0.0f);
		else
			_sconfig.SetDistortionFitPointVP(0.0f, 1.0f);
	}

	// @todo this is in TinyRoom sample
	// it doesn't seem to have any affect though, dunno what it should do.
	_sconfig.Set2DAreaFov(Ogre::Degree(85).valueRadians());

	OVR::Util::Render::DistortionConfig const &d_config = _sconfig.GetDistortionConfig();

	// copy distortion info
	_distortion.K = Ogre::Vector4( d_config.K[0], d_config.K[1], d_config.K[2], d_config.K[3] );
	_distortion.chromatic_aberration = Ogre::Vector4( d_config.ChromaticAberration[0], 
		d_config.ChromaticAberration[1], d_config.ChromaticAberration[2], d_config.ChromaticAberration[3] );
	_distortion.x_center_offset = d_config.XCenterOffset;
	_distortion.y_center_offset = d_config.YCenterOffset;
	_distortion.scale = d_config.Scale;

	OVR::Util::Render::StereoEyeParams leftEye = _sconfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
	OVR::Util::Render::StereoEyeParams rightEye = _sconfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
	// Left eye rendering parameters
	// Custom projection matrix for left eye
	OVR::Matrix4f leftProjection = leftEye.Projection;
	// Matrix that should multiply the view matrix (not necessary if we use ipd properly)
	OVR::Matrix4f leftViewAdjust = leftEye.ViewAdjust;
}
