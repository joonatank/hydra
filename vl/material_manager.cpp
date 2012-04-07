/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file material_manager.hpp
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
#include "material_manager.hpp"

#include "material.hpp"

vl::MaterialManager::MaterialManager(vl::Session *session)
	: _session(session)
	, _ogre_manager(0)
{
	assert(_session);
	_session->registerObject(this, OBJ_MATERIAL_MANAGER, vl::ID_UNDEFINED);
}

vl::MaterialManager::MaterialManager(vl::Session *session, uint64_t id)
	: _session(session)
	, _ogre_manager(0)
{
	assert(_session);
	_session->registerObject(this, OBJ_MATERIAL_MANAGER, id);
}

vl::MaterialManager::~MaterialManager(void)
{
}

vl::MaterialRefPtr
vl::MaterialManager::createMaterial(std::string const &name)
{
	/// @todo check the uniqueness of the material name to avoid collisions
	/// when creating Ogre material.
	MaterialRefPtr mat(new Material(name));
	
	// Register
	assert(_session);
	_session->registerObject(mat.get(), vl::OBJ_MATERIAL, vl::ID_UNDEFINED);
	assert(mat->getID() != vl::ID_UNDEFINED);

	_materials.push_back(mat);

	return mat;
}


vl::MaterialRefPtr
vl::MaterialManager::_createMaterial(uint64_t id)
{
	MaterialRefPtr mat(new Material);
	
	// Register
	assert(_session);
	_session->registerObject(mat.get(), vl::OBJ_MATERIAL, id);
	assert(mat->getID() != vl::ID_UNDEFINED);
	
	_materials.push_back(mat);

	return mat;
}

vl::MaterialRefPtr
vl::MaterialManager::getMaterial(std::string const &name)
{
	for(std::vector<MaterialRefPtr>::iterator iter = _materials.begin();
		iter != _materials.end(); ++iter)
	{
		if((*iter)->getName() == name)
		{
			return *iter;
		}
	}

	return vl::MaterialRefPtr();
}

bool
vl::MaterialManager::hasMaterial(std::string const &name)
{
	return(getMaterial(name) != 0);
}



// --------------------------------- Public ----------------------------------

// --------------------------------- Private ---------------------------------
void
vl::MaterialManager::serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
}

void
vl::MaterialManager::deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
}
