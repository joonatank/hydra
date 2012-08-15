/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file material_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_MATERIAL_MANAGER_HPP
#define HYDRA_MATERIAL_MANAGER_HPP

// Base class
#include "cluster/distributed.hpp"

// Necessary for registering the object
#include "cluster/session.hpp"

#include "typedefs.hpp"

namespace vl
{

class MaterialManager : public vl::Distributed
{
public :
	/// @brief Master constructor
	MaterialManager(vl::Session *session);

	/// @brief Slave constructor
	/// Automatically retrieves the Ogre Material Manager
	MaterialManager(vl::Session *session, uint64_t id);

	virtual ~MaterialManager(void);


	/// Material creation
	/// Master creator
	MaterialRefPtr createMaterial(std::string const &name);

	/// @internal Slave creator
	MaterialRefPtr _createMaterial(uint64_t id);

	MaterialRefPtr getMaterial(std::string const &name);

	bool hasMaterial(std::string const &name);

	Ogre::MaterialManager *getNative(void) const
	{ return _ogre_manager; }

	std::vector<MaterialRefPtr> const &getMaterialList(void) const
	{ return _materials; }

	enum DirtyBits
	{
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 0,
	};

	// Methods
private :
	// Non-copyable
	MaterialManager(MaterialManager const &);
	MaterialManager &operator=(MaterialManager const &);

	void serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	void deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	// Data
private :
	vl::Session *_session;

	std::vector<MaterialRefPtr> _materials;

	// Concrete implementation
	Ogre::MaterialManager *_ogre_manager;

};	// class MaterialManager

inline
std::ostream &operator<<(std::ostream &os, MaterialManager const &man)
{
	os << "Material Manager : with " << man.getMaterialList().size() << " materials." << std::endl;

	return os;
}

inline
std::ostream &operator<<(std::ostream &os, 	std::vector<MaterialRefPtr> const &material_list)
{
	os << "Material list with " << material_list.size() << " materials.";
	
	return os;
}

}	// namespace vl

#endif // HYDRA_MATERIAL_MANAGER_HPP
