/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file material.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_MATERIAL_HPP
#define HYDRA_MATERIAL_HPP

// Base class
#include "cluster/distributed.hpp"

#include "typedefs.hpp"

#include "math/types.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"

// Concrete implementation
#include <OGRE/OgreMaterial.h>

namespace vl
{

class HYDRA_API Material : public vl::Distributed
{
public :
	/// @brief Master constructor
	/// Internal do not call directly (use MaterialManager::createMaterial)
	Material(std::string const &name);

	/// @brief Slave constructor
	/// Internal do not call directly
	Material(void);
	
	virtual ~Material(void);

	std::string const &getName(void) const
	{ return _name; }

	/// @brief reset the name, do not call this unless you know what you are doing
	/// not guarantied to work after the object has been distributed to slaves
	void setName(std::string const &);

	// Single texture supported for the moment
	// and only texture by name
	std::string const &getTexture(void) const
	{ return _texture; }

	void setTexture(std::string const &tex);

	std::string const &getShader(void) const
	{ return _shader; }

	void setShader(std::string const &shader);

	Ogre::ColourValue const &getDiffuse(void) const
	{ return _diffuse; }

	void setDiffuse(Ogre::ColourValue const &diff);

	Ogre::ColourValue const &getSpecular(void) const
	{ return _specular; }

	void setSpecular(Ogre::ColourValue const &specular);

	Ogre::ColourValue const &getEmissive(void) const
	{ return _emissive; }

	void setEmissive(Ogre::ColourValue const &emissive);

	Ogre::ColourValue const &getAmbient(void) const
	{ return _ambient; }

	void setAmbient(Ogre::ColourValue const &ambient);

	/// @todo add shininess
	vl::scalar getShininess(void) const
	{ return _shininess; }

	void setShininess(vl::scalar shininess);

	Ogre::MaterialPtr getNative(void) const
	{ return _ogre_material; }

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_SHADER = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_COLOUR = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_TEXTURE = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 4,
	};

	// Methods
private :
	// Non-copyable
	Material(Material const &);
	Material &operator=(Material const &);

	void serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	void deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	// Data
private :
	std::string _name;

	std::string _shader;

	Ogre::ColourValue _diffuse;
	Ogre::ColourValue _specular;
	Ogre::ColourValue _emissive;
	Ogre::ColourValue _ambient;
	vl::scalar _shininess;

	std::string _texture;

	Ogre::MaterialPtr _ogre_material;

};	// class Material

inline
std::ostream &operator<<(std::ostream &os, Material const &mat)
{
	os << "Material " << mat.getName() << "." << std::endl;
	
	return os;
}

}	// namespace vl

#endif // HYDRA_MATERIAL_HPP
