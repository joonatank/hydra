/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file light.cpp
 */

#include "light.hpp"

#include "scene_manager.hpp"

#include <OGRE/OgreSceneManager.h>

namespace
{

Ogre::Light::LightTypes 
getOgreLightType(vl::Light::LightType type)
{
	Ogre::Light::LightTypes t = Ogre::Light::LT_POINT;
	if( type == vl::Light::LT_POINT )
	{ t = Ogre::Light::LT_POINT; }
	else if( type == vl::Light::LT_SPOT )
	{ t = Ogre::Light::LT_SPOTLIGHT; }
	else if( type == vl::Light::LT_DIRECTIONAL )
	{ t = Ogre::Light::LT_DIRECTIONAL; }
	else
	{ std::cout << "Problem with unknown light type." << std::endl; }

	return t;
}

}

/// ------------------------------ Public ------------------------------------
vl::Light::Light(std::string const &name, vl::SceneManagerPtr creator)
	: MovableObject(name, creator)
{
	_clear();
}

vl::Light::Light(vl::SceneManagerPtr creator)
	: MovableObject("", creator)
{
	_clear();
}

void 
vl::Light::setType(vl::Light::LightType type)
{
	if( _type != type )
	{
		setDirty(DIRTY_TYPE);
		_type = type;
	}
}

void 
vl::Light::setDiffuseColour(Ogre::ColourValue const &col)
{
	if( _diffuse_colour != col )
	{
		setDirty(DIRTY_COLOUR);
		_diffuse_colour = col;
	}
}

void 
vl::Light::setSpecularColour(Ogre::ColourValue const &col)
{
	if( _specular_colour != col )
	{
		setDirty(DIRTY_COLOUR);
		_specular_colour = col;
	}
}

void 
vl::Light::setDirection(Ogre::Vector3 const &dir)
{
	if( _direction != dir )
	{
		setDirty(DIRTY_TRANSFORM);
		_direction = dir;
	}
}
	
void 
vl::Light::setPosition(Ogre::Vector3 const &pos)
{
	if( _position != pos )
	{
		setDirty(DIRTY_TRANSFORM);
		_position = pos;
	}
}

void 
vl::Light::setVisible(bool v)
{
	if( _visible != v )
	{
		setDirty(DIRTY_GEN_PARAMS);
		_visible = v;
	}
}

void 
vl::Light::setCastShadows(bool v)
{
	if( _cast_shadows != v )
	{
		setDirty(DIRTY_GEN_PARAMS );
		_cast_shadows = v;
	}
}

void 
vl::Light::setSpotlightRange(Ogre::Radian const &inner, 
	Ogre::Radian const &outer, Ogre::Real falloff)
{
	if( _inner_cone != inner || _outer_cone != outer || _spot_falloff != falloff)
	{
		setDirty(DIRTY_SPOT_PARAMS);
		_inner_cone = inner;
		_outer_cone = outer;
		_spot_falloff = falloff;
	}
}

void 
vl::Light::setAttenuation(Ogre::Real range, Ogre::Real constant, 
	Ogre::Real linear, Ogre::Real quadratic)
{
	LightAttenuation att(range, constant, linear, quadratic);
	if( att != _attenuation )
	{
		setDirty(DIRTY_ATTENUATION);
		_attenuation = att;
	}
}



Ogre::MovableObject *
vl::Light::getNative(void) const
{ return _ogre_light; }

/// ---------------------------- Protected -----------------------------------
void 
vl::Light::doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_TYPE & dirtyBits )
	{
		msg << _type;
	}

	if( DIRTY_COLOUR & dirtyBits )
	{
		msg << _diffuse_colour << _specular_colour;
	}

	if( DIRTY_TRANSFORM & dirtyBits )
	{
		msg << _position << _direction;
	}
	
	if( DIRTY_GEN_PARAMS & dirtyBits )
	{
		msg << _visible << _cast_shadows;
	}

	if( DIRTY_ATTENUATION & dirtyBits )
	{
		msg << _attenuation;
	}

	if( DIRTY_SPOT_PARAMS & dirtyBits )
	{
		msg << _inner_cone << _outer_cone << _spot_falloff;
	}
}

void
vl::Light::doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_TYPE & dirtyBits )
	{
		msg >> _type;
		// @todo when type has been changed the attenuation and spotlight parameters
		// need to be set if appropriate
		if( _ogre_light )
		{ _ogre_light->setType(getOgreLightType(_type)); }
	}

	if( DIRTY_COLOUR & dirtyBits )
	{
		msg >> _diffuse_colour >> _specular_colour;
		if( _ogre_light )
		{ 
			_ogre_light->setDiffuseColour(_diffuse_colour);
			_ogre_light->setSpecularColour(_specular_colour);
		}
	}

	if( DIRTY_TRANSFORM & dirtyBits )
	{
		msg >> _position >> _direction;
		if( _ogre_light )
		{ 
			_ogre_light->setPosition(_position);
			_ogre_light->setDirection(_direction);
		}
	}
	
	if( DIRTY_GEN_PARAMS & dirtyBits )
	{
		msg >> _visible >> _cast_shadows;
		if( _ogre_light )
		{ 
			_ogre_light->setVisible(_visible);
			_ogre_light->setCastShadows(_cast_shadows); 
		}
	}

	if( DIRTY_ATTENUATION & dirtyBits )
	{
		msg >> _attenuation;
		if( _ogre_light )
		{
			_ogre_light->setAttenuation(_attenuation.range, 
				_attenuation.constant, _attenuation.linear, _attenuation.quadratic); 
		}
	}

	if( DIRTY_SPOT_PARAMS & dirtyBits )
	{
		msg >> _inner_cone >> _outer_cone >> _spot_falloff;

		if( _ogre_light && _ogre_light->getType() == Ogre::Light::LT_SPOTLIGHT )
		{ _ogre_light->setSpotlightRange(_inner_cone, _outer_cone, _spot_falloff); }
	}
}

bool 
vl::Light::_doCreateNative(void)
{
	assert(_creator);
	assert(!_name.empty());

	if( _ogre_light )
	{ return true; }

	_ogre_light = _creator->getNative()->createLight(_name);

	_ogre_light->setType(getOgreLightType(_type));
	_ogre_light->setDiffuseColour(_diffuse_colour);
	_ogre_light->setSpecularColour(_specular_colour);
	_ogre_light->setPosition(_position);
	_ogre_light->setDirection(_direction);
	_ogre_light->setVisible(_visible);
	_ogre_light->setCastShadows(_cast_shadows);
	if( _ogre_light->getType() == Ogre::Light::LT_SPOTLIGHT )
	{ _ogre_light->setSpotlightRange(_inner_cone, _outer_cone, _spot_falloff); }
	_ogre_light->setAttenuation(_attenuation.range, 
			_attenuation.constant, _attenuation.linear, _attenuation.quadratic);

	return true;
}

void 
vl::Light::_clear(void)
{
	_type = LT_POINT;
	_diffuse_colour = Ogre::ColourValue(1,1,1,1);
	_specular_colour =  Ogre::ColourValue(1,1,1,1);
	_position = Ogre::Vector3::ZERO;
	_direction = -Ogre::Vector3::UNIT_Z;
	_visible = true;
	_cast_shadows = true;
	_ogre_light = 0;
}