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

// Interface
#include "stereo_camera.hpp"

/// -------------------------- StereoCamera ----------------------------------
vl::StereoCamera::StereoCamera(void)
	: _camera(0)
	, _ogre_camera(0)
{}

vl::StereoCamera::~StereoCamera(void)
{
}

void
vl::StereoCamera::setHead(vl::Transform const &head)
{
	_head = head;
}

void
vl::StereoCamera::setCamera(vl::CameraPtr cam)
{
	_camera = cam;
	if(cam)
	{
		assert(_camera->getNative());
		_ogre_camera = (Ogre::Camera *)_camera->getNative();
	}
	else
	{ _ogre_camera = 0; }
}


void
vl::StereoCamera::update(STEREO_EYE eye_cfg)
{
	assert(_camera);
	assert(_ogre_camera);

	_frustum.setClipping(_camera->getNearClipDistance(), _camera->getFarClipDistance());

	Ogre::Quaternion wallRot = orientation_to_wall(_frustum.getWall());

	Ogre::Vector3 cam_pos = _camera->getPosition();
	Ogre::Quaternion cam_quat = _camera->getOrientation();

	Ogre::Vector3 eye;
	if(eye_cfg == HS_LEFT)
	{ eye = Ogre::Vector3(-_ipd/2, 0, 0); }
	else if(eye_cfg == HS_RIGHT)
	{ eye = Ogre::Vector3(_ipd/2, 0, 0); }
	else
	{ eye = Ogre::Vector3::ZERO; }

	Ogre::Matrix4 projMat = _frustum.getProjectionMatrix(eye.x);
	_ogre_camera->setCustomProjectionMatrix(true, projMat);

	// Combine eye and camera positions
	// Needs to be rotated with head for correct stereo
	// Do not rotate with wall will cause incorrect view for the side walls
	//
	// cam*head*eye
	Ogre::Vector3 eye_d = cam_quat*(_head.quaternion*eye + _head.position) + cam_pos;

	// Combine camera and wall orientation to get the projection on correct wall
	// Seems like the wallRotation needs to be inverse for this one, otherwise
	// left and right wall are switched.
	Ogre::Quaternion eye_orientation = cam_quat*wallRot.Inverse();

	_ogre_camera->setPosition(eye_d);
	_ogre_camera->setOrientation(eye_orientation);
}
