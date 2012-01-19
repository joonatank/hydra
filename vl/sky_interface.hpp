/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-01
 *	@file sky_interface.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#ifndef HYDRA_SKY_INTERFACE_HPP
#define HYDRA_SKY_INTERFACE_HPP

#include "math/types.hpp"
#include "base/timer.hpp"
#include "typedefs.hpp"

#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreVector2.h>

namespace vl
{

// Not yet implemented
struct AtmosphericOptions
{
};

/** SkyX settings struct
@remarks These just are the most important SkyX parameters, not all SkyX parameters.
 */
struct SkySettings
{
	/** Constructor
	 */
	SkySettings(const Ogre::Vector3 t, const Ogre::Real& tm, const Ogre::Real& mp,
		const bool& lc, const bool& vc, const Ogre::Real& vcws, const bool& vcauto, const Ogre::Vector3& vcwd, 
		const Ogre::ColourValue& vcac, const Ogre::ColourValue& vclr,  const Ogre::ColourValue& vcaf, const Ogre::Vector2& vcw)
		: time(t), timeMultiplier(tm), moonPhase(mp), layeredClouds(lc), volumetricClouds(vc), vcWindSpeed(vcws)
		, vcAutoupdate(vcauto), vcWindDir(vcwd), vcAmbientColor(vcac), vcLightResponse(vclr), vcAmbientFactors(vcaf), vcWheater(vcw)
	{}

	/** Constructor
	 */
	SkySettings(const Ogre::Vector3 t, const Ogre::Real& tm, const Ogre::Real& mp,
		const bool& lc, const bool& vc)
		: time(t), timeMultiplier(tm), moonPhase(mp), layeredClouds(lc), volumetricClouds(vc)
	{}

	// Must be for std::map
	SkySettings(void)
	{}

	/// Time
	Ogre::Vector3 time;
	/// Time multiplier
	Ogre::Real timeMultiplier;
	/// Moon phase
	Ogre::Real moonPhase;
	/// Layered clouds?
	bool layeredClouds;
	/// Volumetric clouds?
	bool volumetricClouds;
	/// VClouds wind speed
	Ogre::Real vcWindSpeed;
	/// VClouds autoupdate
	bool vcAutoupdate;
	/// VClouds wind direction
	Ogre::Vector3 vcWindDir;
	/// VClouds ambient color
	Ogre::ColourValue vcAmbientColor;
	/// VClouds light response
	Ogre::ColourValue vcLightResponse;
	/// VClouds ambient factors
	Ogre::ColourValue vcAmbientFactors;
	/// VClouds wheater
	Ogre::Vector2 vcWheater;
};



class Sky
{
public :

	virtual ~Sky(void) {}

	virtual void setTimeMultiplier(vl::scalar mul) = 0;
	virtual void setSunsetTime(vl::time const &t) = 0;
	virtual void setSunriseTime(vl::time const &t) = 0;
	virtual void setTime(vl::time const &t) = 0;
	virtual void setMoonPhase(vl::scalar phase) = 0;
	virtual void setWindSpeed(vl::scalar speed) = 0;
	virtual void setWindDirection(Ogre::Vector3 const &dir) = 0;
	virtual void setAmbientColor(Ogre::ColourValue const &col) = 0;
	virtual void setLightResponse(Ogre::ColourValue const &col) = 0;
	virtual void setAmbientFactors(Ogre::ColourValue const &col) = 0;
	virtual void setWeather(Ogre::Vector2 const &weather) = 0;
	virtual void enableVolumetricClouds(bool clouds) = 0;

	virtual void update(vl::time const &delta_time) = 0;

	virtual void notifyCameraRender(vl::Camera* c) = 0;
};

}	// namespace vl

#endif	// HYDRA_SKY_INTERFACE_HPP
