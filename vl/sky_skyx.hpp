/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-01
 *	@file sky_skyx.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_SKY_SKYX_HPP
#define HYDRA_SKY_SKYX_HPP

#include "sky_interface.hpp"

#include <SkyX.h>

namespace vl
{

class SkySkyX : public Sky
{
public :
	SkySkyX(SceneManager *man);

	virtual ~SkySkyX(void);

	virtual void setTimeMultiplier(vl::scalar mul);
	virtual void setSunsetTime(vl::time const &t);
	virtual void setSunriseTime(vl::time const &t);
	virtual void setTime(vl::time const &t);
	virtual void setMoonPhase(vl::scalar phase);
	virtual void setWindSpeed(vl::scalar speed);
	virtual void setWindDirection(Ogre::Vector3 const &dir);
	virtual void setAmbientColor(Ogre::ColourValue const &col);
	virtual void setLightResponse(Ogre::ColourValue const &col);
	virtual void setAmbientFactors(Ogre::ColourValue const &col);
	virtual void setWeather(Ogre::Vector2 const &weather);

	virtual void enableVolumetricClouds(bool clouds);

	virtual void update(vl::time const &delta_time);

	virtual void notifyCameraRender(vl::Camera* c);

private :
	/// SkyX stuff
	SkyX::BasicController *_skyX_controller;
	SkyX::SkyX *_skyX;
};

}	// namespace vl

#endif	// HYDRA_SKY_SKYX_HPP
