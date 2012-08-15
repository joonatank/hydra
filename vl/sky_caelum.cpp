/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-01
 *	@file sky_caelum.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "sky_caelum.hpp"

#include "scene_manager.hpp"
#include "camera.hpp"

void logUnsupportedException(Caelum::UnsupportedException const &ex)
{
	std::clog << "Caelum::UnsupportedException : " << ex.what() << std::endl;
}

vl::SkyCaelum::SkyCaelum(vl::SceneManager *man)
{
	assert(man && man->getNative());
	// Initialise Caelum
	_caelum_system.reset(new Caelum::CaelumSystem (
			Ogre::Root::getSingletonPtr(),
			man->getNative(),
			Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE));

	// Create subcomponents (and allow individual subcomponents to fail).
	try {
		_caelum_system->setSkyDome (new Caelum::SkyDome ( man->getNative(), _caelum_system->getCaelumCameraNode ()));
	} catch (Caelum::UnsupportedException const &ex) {
		logUnsupportedException (ex);
	}
	try {
		_caelum_system->setSun (new Caelum::SphereSun( man->getNative(), _caelum_system->getCaelumCameraNode ()));
	} catch (Caelum::UnsupportedException& ex) {
		logUnsupportedException (ex);
	}
	try {
		_caelum_system->setMoon (new Caelum::Moon( man->getNative(), _caelum_system->getCaelumCameraNode ()));
	} catch (Caelum::UnsupportedException& ex) {
		logUnsupportedException (ex);
	}
	try {
		_caelum_system->setCloudSystem (new Caelum::CloudSystem ( man->getNative(), _caelum_system->getCaelumGroundNode ()));
	} catch (Caelum::UnsupportedException& ex) {
		logUnsupportedException (ex);
	}
	try {
		_caelum_system->setPointStarfield (new Caelum::PointStarfield ( man->getNative(), _caelum_system->getCaelumCameraNode ()));
	} catch (Caelum::UnsupportedException& ex) {
		logUnsupportedException (ex);
	}

	// Register caelum.
	// Don't make it a frame listener; update it by hand.
	//Root::getSingletonPtr ()->addFrameListener (mCaelumSystem.get ());
	// @todo some functionality is lost if viewport is not attached
	//_caelum_system->attachViewport (getViewport ());

	try {
		_caelum_system->setPrecipitationController (new Caelum::PrecipitationController ( man->getNative()));
	} catch (Caelum::UnsupportedException& ex) {
		logUnsupportedException (ex);
	}

	_caelum_system->setSceneFogDensityMultiplier (0.0015);
	_caelum_system->setManageAmbientLight (true);
	_caelum_system->setMinimumAmbientLight (Ogre::ColourValue (0.1, 0.1, 0.1));

	// Test spinning the caelum root node. Looks wrong in the demo;
	// but at least the sky components are aligned with each other.
	if (false) {
		_caelum_system->getCaelumCameraNode ()->setOrientation(
				Ogre::Quaternion (Ogre::Radian (Ogre::Math::PI), Ogre::Vector3::UNIT_Z) *
				Ogre::Quaternion (Ogre::Radian (Ogre::Math::PI / 2), Ogre::Vector3::UNIT_X));
		_caelum_system->getCaelumCameraNode ()->_update (true, true);
	}

	// Setup sun options
	if (_caelum_system->getSun())
	{
		// Make the sun very obvious:
		//mCaelumSystem->getSun ()->setDiffuseMultiplier (Ogre::ColourValue (1, 10, 1));

		_caelum_system->getSun()->setAutoDisableThreshold(0.05);
		_caelum_system->getSun()->setAutoDisable(false);
	}

    if (_caelum_system->getMoon())
	{
		// Make the moon very obvious:
		//mCaelumSystem->getMoon ()->setDiffuseMultiplier (Ogre::ColourValue (1, 1, 10));

		_caelum_system->getMoon()->setAutoDisableThreshold(0.05);
		_caelum_system->getMoon()->setAutoDisable(false);
    }

    if (_caelum_system->getCloudSystem())
	{
        try
		{
			_caelum_system->getCloudSystem ()->createLayerAtHeight(2000);
			_caelum_system->getCloudSystem ()->createLayerAtHeight(5000);
			_caelum_system->getCloudSystem ()->getLayer(0)->setCloudSpeed(Ogre::Vector2(0.000005, -0.000009));
			_caelum_system->getCloudSystem ()->getLayer(1)->setCloudSpeed(Ogre::Vector2(0.0000045, -0.0000085));
        } catch (Caelum::UnsupportedException& ex)
		{
			logUnsupportedException (ex);
        }
    }

    if (_caelum_system->getPrecipitationController())
	{
        _caelum_system->getPrecipitationController()->setIntensity (0);
    }

    // Set time acceleration.
    //_caelum_system->getUniversalClock ()->setTimeScale (0);

    // Sunrise with visible moon.
    _caelum_system->getUniversalClock()->setGregorianDateTime (2007, 4, 9, 9, 33, 0);
}

vl::SkyCaelum::~SkyCaelum(void)
{
	//_caelum_system.reset();
}

void
vl::SkyCaelum::setTimeMultiplier(vl::scalar mul)
{
}

void
vl::SkyCaelum::setSunsetTime(vl::time const &t)
{
}

void
vl::SkyCaelum::setSunriseTime(vl::time const &t)
{
}

void
vl::SkyCaelum::setTime(vl::time const &t)
{
}

void
vl::SkyCaelum::setMoonPhase(vl::scalar phase)
{
}

void
vl::SkyCaelum::setWindSpeed(vl::scalar speed)
{
}

void
vl::SkyCaelum::setWindDirection(Ogre::Vector3 const &dir)
{
}

void
vl::SkyCaelum::setAmbientColor(Ogre::ColourValue const &col)
{
}

void
vl::SkyCaelum::setLightResponse(Ogre::ColourValue const &col)
{
}

void
vl::SkyCaelum::setAmbientFactors(Ogre::ColourValue const &col)
{
}

void
vl::SkyCaelum::setWeather(Ogre::Vector2 const &weather)
{
}

void
vl::SkyCaelum::enableVolumetricClouds(bool clouds)
{
}

void
vl::SkyCaelum::update(vl::time const &delta_time)
{
	_caelum_system->updateSubcomponents((double)delta_time);
}

void
vl::SkyCaelum::notifyCameraRender(vl::Camera* c)
{
	assert(c && c->getNative());
	_caelum_system->notifyCameraChanged((Ogre::Camera *)c->getNative());
}
