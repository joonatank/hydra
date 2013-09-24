/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-01
 *	@file eye_tracker.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "eye_tracker.hpp"

#include "ray_object.hpp"
// Necessary for creating the RayObject
#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "player.hpp"

#include "eyes_report.hpp"

#ifndef USE_EYES
/// Dummy implementation without Eyes
vl::EyeTracker::EyeTracker(SceneManager *creator)
	: _drawable(0)
	, _creator(creator)
	, _started(false)
{}
void
vl::EyeTracker::start(void)
{}

void
vl::EyeTracker::stop(void)
{}

void
vl::EyeTracker::progress(void)
{}

void
vl::EyeTracker::_update_head(void)
{}

#else
vl::EyeTracker::EyeTracker(PlayerPtr player, SceneManager *creator)
	: _player(player)
	, _drawable(0)
	, _creator(creator)
	, _started(false)
	, _debug(false)
	, _head_disabled(false)
{
	assert(_creator);

	// Create the drawable and hide it
	// @todo we need the name and material name as parameters
	// for now this will limit us to single eye tracker
	// and always using debug red colour
	// We could easily make it configurable by getRayObject member function
	// but will this expose too many details?
	// @todo should we move the sn or the drawable?
	// sn would be more natural in the sense Ogre understands it (no clipping errors)
	// @todo we need to attach the scene node to current camera
	// and we need to switch between active cameras.
	// might just be easier to copy the camera transformation from current camera to our sn
	_node = _creator->getRootSceneNode()->createChildSceneNode("EyeRay");
	_drawable = creator->createRayObject("EyeRay", "collision_sphere/red");
	_drawable->setDirection(Ogre::Vector3(0, 0, -1));
	_drawable->setSphereRadius(0.2);
	// Doesn't affect performance that much if the spheres are on or off
	_drawable->setDrawCollisionSphere(true);
	_drawable->setCollisionDetection(true);
	_node->attachObject(_drawable);
	_node->hide();

	// Create Eyes
	// Only supports ini file because we can not pass command line arguments here
	eyes::Options opt;
	opt.parse_ini();
	// Reset VRPN head tracker name from the file
	// we want to provide our own head tracker data for consistency
	opt.vrpn_head_tracker = "";
	std::clog << "Eyes Options : " << opt << std::endl;
	_eyes.reset(new eyes::EyeTracker(opt));
}

void
vl::EyeTracker::start(void)
{
	if(_started)
	{ return; }

	// Do start

	// Update head before calibrating
	_update_head();
	_eyes->calibrate();
	_node->show();

	_started = true;
}

void
vl::EyeTracker::stop(void)
{
	if(!_started)
	{ return; }

	// Do stop
	_node->hide();

	_started = false;
}

void
vl::EyeTracker::_update_head(void)
{
	if(_head_disabled)
	{ return; }

	vl::Transform const &t = _player->getHeadTransform();
	glm::vec3 v(t.position.x, t.position.y, t.position.z);
	glm::quat q(t.quaternion.w, t.quaternion.x, t.quaternion.y, t.quaternion.z);

	_eyes->updateHead(v, q);
}

void
vl::EyeTracker::progress(void)
{
	if(!_started)
	{ return; }

	assert(_eyes);
	assert(_node && _player && _drawable);

	_update_head();

	_eyes->mainloop();
	
	// Copy the transformation from Player::currentCamera to our Ray SceneNode
	// @fixme this causes very weird bugs in the eye position
	// even with the static position
	if(!_debug)
	{
		_node->setWorldTransform(_player->getCameraNode()->getWorldTransform());
	}

	// No reason to check this because we update the head transformation every frame
//	if(_eyes->hasChanged())
	{
		// Get new data
		glm::vec3 v = _eyes->getEyePosition();
		glm::vec3 dir = _eyes->getEyeDirection();

		// Update the drawable
		_drawable->setPosition(Ogre::Vector3(v.x, v.y, v.z));
		_drawable->setDirection(Ogre::Vector3(dir.x, dir.y, dir.z));;

//		_eyes->clearChanged();
	}
}

void
vl::EyeTracker::loadRecording(std::string const &filename)
{
	// @todo this needs to disable real time tracking

	// @todo this is easier to manage if we use separate report
	// for this
	//_eyes->getReport().read(filename);
	if(filename.empty())
	{
		_loaded_report = _eyes->getReport();
	}
	else
	{
		/// @todo needs to check that we actually got a report
		_loaded_report.read(filename);
	}

	showRecording(true);
}

void
vl::EyeTracker::saveRecording(std::string const &filename)
{
	_eyes->getReport().write(filename);
}

void
vl::EyeTracker::showRecording(bool show)
{
	assert(_drawable);

	RayObject::RecType recording;
	recording.reserve(_eyes->getReport().data.size());

	for(std::vector<eyes::Report::DataElem>::const_iterator iter = _loaded_report.data.begin();
		iter != _loaded_report.data.end(); ++iter)
	{
		Ogre::Vector3 pos(iter->position.x, iter->position.y, iter->position.z);
		Ogre::Vector3 dir(iter->direction.x, iter->direction.y, iter->direction.z);
		recording.push_back(std::make_pair(pos, dir));
	}

	_drawable->setRecording(recording);
	_drawable->showRecordedRays(true);
}

bool
vl::EyeTracker::isRecordingShown(void) const
{
	assert(_drawable);
	return _drawable->getShowRecordedRays();
}

void
vl::EyeTracker::printInit(void)
{
	std::cout << _eyes->getInit();
}

#endif
