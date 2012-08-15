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

#ifndef HYDRA_SKY_CAELUM_HPP
#define HYDRA_SKY_CAELUM_HPP

#include "sky_interface.hpp"

#include <memory>

#include <Caelum.h>

namespace vl
{

class SkyCaelum : public Sky
{
public :
	SkyCaelum(SceneManager *man);

	/// @fixme the destructor crashes the program
	virtual ~SkyCaelum(void);

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
	 std::auto_ptr<Caelum::CaelumSystem> _caelum_system;

};	// class SkyCaelum

}	// namespace vl

#endif	// HYDRA_SKY_CAELUM_HPP
