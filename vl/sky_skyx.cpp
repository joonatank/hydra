/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-01
 *	@file sky_skyx.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "sky_skyx.hpp"

#include "scene_manager.hpp"

#include "camera.hpp"

vl::SkySkyX::SkySkyX(SceneManager *man)
{
	std::clog << "vl::SkySkyX::SkySkyX" << std::endl;

	assert(man && man->getNative());
	_skyX_controller = new SkyX::BasicController();
	_skyX = new SkyX::SkyX(man->getNative(), _skyX_controller);
	_skyX->create();

	// Distance geometry falling is a feature introduced in SkyX 0.2
	// When distance falling is enabled, the geometry linearly falls with the distance and the
	// amount of falling in world units is determinated by the distance between the cloud field "plane"
	// and the camera height multiplied by the falling factor.
	// For this demo, a falling factor of two is good enough for the point of view we're using. That means that if the camera
	// is at a distance of 100 world units from the cloud field, the fartest geometry will fall 2*100 = 200 world units.
	// This way the cloud field covers a big part of the sky even if the camera is in at a very low altitude.
	// The second parameter is the max amount of falling distance in world units. That's needed when for example, you've an 
	// ocean and you don't want to have the volumetric cloud field geometry falling into the water when the camera is underwater.
	// -1 means that there's not falling limit.
	//_skyX->getVCloudsManager()->getVClouds()->setDistanceFallingParams(Ogre::Vector2(2,-1));

	// Register SkyX listeners
	// @todo remove and use update instead
	Ogre::Root::getSingleton().addFrameListener(_skyX);
	// Window listener necessary?
	// if it's we can retrieve the first window and always attach to that...
	//mWindow->addListener(_skyX);

	// Assuming this is useful anyway
	_skyX->getVCloudsManager()->setAutoupdate(true);

	// Hard coded for now
	SkyX::AtmosphereManager::Options atmosphere_opt(9.77501f, 10.2963f, 0.01f, 0.0022f, 0.000675f, 30, Ogre::Vector3(0.57f, 0.52f, 0.44f), -0.991f, 3, 4);
	_skyX->getAtmosphereManager()->setOptions(atmosphere_opt);
}

vl::SkySkyX::~SkySkyX(void)
{
	delete _skyX_controller;
	delete _skyX;
}

void
vl::SkySkyX::setTimeMultiplier(vl::scalar mul)
{
	_skyX->setTimeMultiplier(mul);
}

void
vl::SkySkyX::setSunsetTime(vl::time const &t)
{
	Ogre::Vector3 tim = _skyX_controller->getTime();
	tim.z = (double)t;
	_skyX_controller->setTime(tim);
}

void
vl::SkySkyX::setSunriseTime(vl::time const &t)
{
	Ogre::Vector3 tim = _skyX_controller->getTime();
	tim.y = (double)t;
	_skyX_controller->setTime(tim);
}

void
vl::SkySkyX::setTime(vl::time const &t)
{
	Ogre::Vector3 tim = _skyX_controller->getTime();
	tim.x = (double)t;
	_skyX_controller->setTime(tim);
}

void
vl::SkySkyX::setMoonPhase(vl::scalar phase)
{
	_skyX_controller->setMoonPhase(phase);
}

void
vl::SkySkyX::setWindSpeed(vl::scalar speed)
{
	_skyX->getVCloudsManager()->setWindSpeed(speed);
}

void
vl::SkySkyX::setWindDirection(Ogre::Vector3 const &dir)
{
	SkyX::VClouds::VClouds* vclouds = _skyX->getVCloudsManager()->getVClouds();

	Ogre::Vector3 d(dir);
	d.y = 0;
	d.normalise();

	vclouds->setWindDirection(d.angleBetween(Ogre::Vector3::NEGATIVE_UNIT_Z));
}

void
vl::SkySkyX::setAmbientColor(Ogre::ColourValue const &col)
{
	SkyX::VClouds::VClouds* vclouds = _skyX->getVCloudsManager()->getVClouds();
	vclouds->setAmbientColor(Ogre::Vector3(col.r, col.g, col.b));
}

void
vl::SkySkyX::setLightResponse(Ogre::ColourValue const &col)
{
	SkyX::VClouds::VClouds* vclouds = _skyX->getVCloudsManager()->getVClouds();
	vclouds->setLightResponse(Ogre::Vector4(col.r, col.g, col.b, col.a));
}

void
vl::SkySkyX::setAmbientFactors(Ogre::ColourValue const &col)
{
	SkyX::VClouds::VClouds* vclouds = _skyX->getVCloudsManager()->getVClouds();
	vclouds->setAmbientFactors(Ogre::Vector4(col.r, col.g, col.b, col.a));
}

void
vl::SkySkyX::setWeather(Ogre::Vector2 const &weather)
{
	SkyX::VClouds::VClouds* vclouds = _skyX->getVCloudsManager()->getVClouds();
	vclouds->setWheater(weather.x, weather.y, false);
}


void
vl::SkySkyX::enableVolumetricClouds(bool clouds)
{
	if(clouds)
	{
		std::clog << "Craeting Volumetric clouds." << std::endl;
		// Create VClouds
		if (!_skyX->getVCloudsManager()->isCreated())
		{
			// SkyX::MeshManager::getSkydomeRadius(...) works for both finite and infinite(=0) camera far clip distances
			// @todo fix the camera stuff
			// @todo we can not easily access Ogre camera so lets just guess a value
			_skyX->getVCloudsManager()->create(2000); //_skyX->getMeshManager()->getSkydomeRadius(cam));
		}
	}
	else
	{
		// Remove VClouds
		if (_skyX->getVCloudsManager()->isCreated())
		{
			_skyX->getVCloudsManager()->remove();
		}
	}
}


void
vl::SkySkyX::update(vl::time const &delta_time)
{
	_skyX->update((double)delta_time);
}

void
vl::SkySkyX::notifyCameraRender(vl::Camera* c)
{
	assert(c && c->getNative());
	_skyX->notifyCameraRender((Ogre::Camera *)c->getNative());
}
