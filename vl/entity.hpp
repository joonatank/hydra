/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-04
 *	@file entity.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_ENTITY_HPP
#define HYDRA_ENTITY_HPP

// Necessary for HYDRA_API
#include "defines.hpp"
// Base class
#include "movable_object.hpp"

#include <string>

#include "typedefs.hpp"
/// Necessary for the Loaded callback
#include "mesh_manager.hpp"

#include <OGRE/OgreEntity.h>

namespace vl
{

/// @brief Callback functor for non-blocking mesh loading
/// Called from MeshManager when mesh has been loaded.
/// We need to use non-blocking mesh loading, because loading a mesh
/// would otherwise distrubt the message system.
struct EntityMeshLoadedCallback : public vl::MeshLoadedCallback
{
	EntityMeshLoadedCallback(Entity *ent);

	virtual void meshLoaded(vl::MeshRefPtr mesh);

	Entity *owner;
};

/**	@class Entity
 *	@brief Entity that can be drawn to the scene, is distributed
 */
class HYDRA_API Entity : public MovableObject
{
public :
	/// Constructor
	/// Should not be called from user code, use SceneManager to create these
	Entity(std::string const &name, std::string const &mesh_name, 
		vl::SceneManagerPtr creator, bool dynamic, bool use_new_mesh_manager = false);

	/// @internal used by slave mapping
	Entity(vl::SceneManagerPtr creator);

	/// Destructor
	virtual ~Entity(void);

	std::string const &getMeshName(void) const
	{ return _mesh_name; }

	MeshRefPtr getMesh(void) const
	{ return _mesh; }

	void setCastShadows(bool shadows);

	bool getCastShadows(void) const
	{ return _cast_shadows; }

	void setMaterialName(std::string const &name);

	std::string const &getMaterialName(void) const
	{ return _material_name; }

	void setInstanced(bool enabled);

	bool isInstanced(void) const
	{ return _is_instanced; }

	virtual vl::MovableObjectPtr clone(std::string const &append_to_name) const;

	enum DirtyBits
	{
		DIRTY_MESH_NAME = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_CAST_SHADOWS = vl::MovableObject::DIRTY_CUSTOM << 1,
		DIRTY_MATERIAL = vl::MovableObject::DIRTY_CUSTOM << 2,
		DIRTY_INSTANCED = vl::MovableObject::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = vl::MovableObject::DIRTY_CUSTOM << 4,
	};

	/// Internal
	/// Callback for MeshManager when background loading is used
	void meshLoaded(vl::MeshRefPtr mesh);

	// @todo needs a size (bounding box size is fine) and a scale

/// Virtual overrides
	virtual Ogre::MovableObject *getNative(void) const
	{ return _ogre_object; }

	std::string getTypeName(void) const
	{ return "Entity"; }

private :
	virtual void doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	virtual bool _doCreateNative(void);

	bool _finishCreateNative(void);

	/// clears the structure to default values, called from constructors
	void _clear(void);

	Ogre::MovableObject *_createNativeEntity(std::string const &_name, std::string const &_mesh_name);

	std::string _mesh_name;

	/// For new MeshManager
	bool _use_new_mesh_manager;
	vl::MeshRefPtr _mesh;

	bool _cast_shadows;
	std::string _material_name;

	bool _is_instanced;

	Ogre::MovableObject *_ogre_object;

	// Save the loader pointer so it can be destroyed when not needed anymore
	EntityMeshLoadedCallback *_loader_cb;
};

std::ostream &operator<<(std::ostream &os, vl::Entity const &ent);

}	// namespace vl

#endif	// HYDRA_ENTITY_HPP
