/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file material.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Interface
#include "material.hpp"

#include "material_manager.hpp"

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreMaterialManager.h>

// --------------------------------- Public ----------------------------------
vl::Material::Material(std::string const &name)
	: _name(name)
	, _ogre_material(0)
{}

vl::Material::Material(void)
	: _name()
	, _ogre_material(0)
{}

vl::Material::~Material(void)
{
}

/// Attributes

void
vl::Material::setName(std::string const &name)
{
	if(name != _name)
	{
		setDirty(DIRTY_NAME);
		_name = name;
	}
}

void
vl::Material::setTexture(std::string const &tex)
{
	if(tex != _texture)
	{
		setDirty(DIRTY_TEXTURE);
		_texture = tex;
	}
}

void
vl::Material::setShader(std::string const &shader)
{
	if(shader != _shader)
	{
		setDirty(DIRTY_SHADER);
		_shader = shader;
	}
}

void
vl::Material::setDiffuse(Ogre::ColourValue const &diff)
{
	if(diff != _diffuse)
	{
		setDirty(DIRTY_COLOUR);
		_diffuse = diff;
	}
}

void
vl::Material::setSpecular(Ogre::ColourValue const &specular)
{
	if(specular != _specular)
	{
		setDirty(DIRTY_COLOUR);
		_specular = specular;
	}
}

void
vl::Material::setEmissive(Ogre::ColourValue const &emissive)
{
	if(emissive != _emissive)
	{
		setDirty(DIRTY_COLOUR);
		_emissive = emissive;
	}
}

void
vl::Material::setAmbient(Ogre::ColourValue const &ambient)
{
	if(ambient != _ambient)
	{
		setDirty(DIRTY_COLOUR);
		_ambient = ambient;
	}
}

// --------------------------------- Private ---------------------------------
void
vl::Material::serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
	if(DIRTY_NAME & dirtyBits)
	{
		msg << _name;
	}

	if(DIRTY_SHADER & dirtyBits)
	{
		msg << _shader;
	}

	if(DIRTY_COLOUR & dirtyBits)
	{
		msg << _diffuse << _specular << _emissive << _ambient;
	}

	if(DIRTY_TEXTURE & dirtyBits)
	{
		msg << _texture;
	}
}

void
vl::Material::deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	bool create = false;
	if(DIRTY_NAME & dirtyBits)
	{
		msg >> _name;
		create = true;
	}

	if(DIRTY_SHADER & dirtyBits)
	{
		msg >> _shader;
	}

	if(DIRTY_COLOUR & dirtyBits)
	{
		msg >> _diffuse >> _specular >> _emissive >> _ambient;
		if(_ogre_material)
		{
			_ogre_material->setDiffuse(_diffuse);
			_ogre_material->setSpecular(_specular);
			_ogre_material->setSelfIllumination(_emissive);
			_ogre_material->setAmbient(_ambient);
		}
	}

	if(DIRTY_TEXTURE & dirtyBits)
	{
		msg >> _texture;
	}

	if(create)
	{
		if(_ogre_material)
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Recreating Ogre material not supported.")); }

		// @todo Create ogre object
		// we can just use the singleton
		std::clog << "Creating Ogre material " << _name << std::endl;
		// @todo should provide the ManualResourceLoader (which we don't have...)
		Ogre::ResourcePtr res = Ogre::MaterialManager::getSingleton()
			.create(_name, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, true);
		
		assert(dynamic_cast<Ogre::Material *>(res.get()));
		_ogre_material = static_cast<Ogre::Material *>(res.get());
		_ogre_material->setDiffuse(_diffuse);
		_ogre_material->setSpecular(_specular);
		_ogre_material->setSelfIllumination(_emissive);
		_ogre_material->setAmbient(_ambient);
	}
}
