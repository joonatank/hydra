/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-04
 *	@file light.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_LIGHT_HPP
#define HYDRA_LIGHT_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

// Base class
#include "movable_object.hpp"

#include <OGRE/OgreLight.h>

namespace vl
{

struct HYDRA_API LightAttenuation
{
	LightAttenuation(Ogre::Real r, Ogre::Real c, Ogre::Real l, Ogre::Real q)
		: range(r), constant(c), linear(l), quadratic(q)
	{}

	LightAttenuation(void)
		: range(100), constant(1), linear(0), quadratic(0)
	{}

	/// @brief normalises the attenuation parameters (except range)
	/// range is left as it is, but the square sum of others is 1
	void normalise(void)
	{
		Ogre::Vector3 v(constant, linear, quadratic);
		v.normalise();
		constant = v.x;
		linear = v.y;
		quadratic = v.z;
	}

	Ogre::Real range;
	Ogre::Real constant;
	Ogre::Real linear;
	Ogre::Real quadratic;
};

inline std::ostream &
operator<<(std::ostream &os, LightAttenuation const &att)
{
	os << " Attenuation : range " << att.range 
		<< " : constant " << att.constant 
		<< " : linear " << att.linear 
		<< " : quadratic " << att.quadratic;

	return os;
}

inline bool
operator==(LightAttenuation const &att1, LightAttenuation const &att2)
{
	return( att1.range == att2.range
		&& att1.constant == att2.constant
		&& att1.linear == att2.linear
		&& att1.quadratic == att2.quadratic );
}

inline bool
operator!=(LightAttenuation const &att1, LightAttenuation const &att2)
{
	return !(att1 == att2);
}

class HYDRA_API Light : public MovableObject
{
public :
	enum LightType
	{
		LT_POINT,
		LT_SPOT,
		LT_DIRECTIONAL,
	};

	/// Constructor
	/// Should not be called from user code, use SceneManager to create these
	Light(std::string const &name, vl::SceneManagerPtr creator, bool dynamic);

	/// Internal used by slave mapping
	Light(vl::SceneManagerPtr creator);

	virtual ~Light(void);

	LightType getType(void) const
	{ return _type; }

	/// @brief get light type as a string, mostly for python
	std::string getLightTypeName(void) const;

	void setType(LightType type);

	/// @brief Set type with a typename, mostly for python
	/// Case insensitive
	/// valid values "spot" or "spotlight", "point", "directional" or "dir"
	void setType(std::string const &type);

	Ogre::ColourValue const &getDiffuseColour(void) const
	{ return _diffuse_colour; }

	void setDiffuseColour(Ogre::ColourValue const &col);

	Ogre::ColourValue const &getSpecularColour(void) const
	{ return _specular_colour; }

	void setSpecularColour(Ogre::ColourValue const &col);

	Ogre::Vector3 const &getDirection(void) const
	{ return _direction; }

	void setDirection(Ogre::Vector3 const &q);
	
	Ogre::Vector3 const &getPosition(void) const
	{ return _position; }

	void setPosition(Ogre::Vector3 const &pos);

	bool getCastShadows(void) const
	{ return _cast_shadows; }

	void setCastShadows(bool v);

	/// @todo add getters
	void setSpotlightRange(Ogre::Radian const &inner, Ogre::Radian const &outer, Ogre::Real falloff = 1.0);

//	void setAttenuation(Ogre::Real range, Ogre::Real constant, Ogre::Real linear, Ogre::Real quadratic);

	void setAttenuation(LightAttenuation const &att);

	LightAttenuation const &getAttenuation(void) const
	{ return _attenuation; }

	virtual MovableObjectPtr clone(std::string const &append_to_name) const;

	enum DirtyBits
	{
		DIRTY_TYPE = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_COLOUR = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_TRANSFORM = vl::Distributed::DIRTY_CUSTOM << 2,
		/// General parameters, anything that does not fit with the others
		DIRTY_GEN_PARAMS = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_ATTENUATION = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_SPOT_PARAMS = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 6,
	};

/// Virtual overrides
	virtual Ogre::MovableObject *getNative(void) const;

	std::string getTypeName(void) const
	{ return "Light"; }

	friend std::ostream &operator<<(std::ostream &os, vl::Light const &light);

/// Private virtual overrides
private :
	
	virtual void doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	virtual bool _doCreateNative(void);

	/// clears the structure to default values, called from constructors
	void _clear(void);
	
/// Private data
private :
	LightType _type;
	
	Ogre::ColourValue _diffuse_colour;
	Ogre::ColourValue _specular_colour;

	Ogre::Vector3 _position;
	Ogre::Vector3 _direction;

	bool _cast_shadows;
	
	Ogre::Radian _inner_cone; 
	Ogre::Radian _outer_cone;
	Ogre::Real _spot_falloff;

	LightAttenuation _attenuation;

	Ogre::Light *_ogre_light;

};	// class Light

inline std::string
getLightTypeAsString(Light::LightType t)
{
	switch(t)
	{
	case Light::LT_POINT :
		return "POINT";
	case Light::LT_SPOT :
		return "SPOT";
	case Light::LT_DIRECTIONAL :
		return "DIRECTIONAL";
	default:
		return "UNKNOWN";
	}
}

std::ostream &operator<<(std::ostream &os, vl::Light const &light);

}	// namespace vl

#endif	// HYDRA_LIGHT_HPP