/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file entity.hpp
 */

#ifndef VL_ENTITY_HPP
#define VL_ENTITY_HPP

// Base class
#include "movable_object.hpp"

#include <string>

#include "typedefs.hpp"
/// Necessary for the Loaded callback
#include "mesh_manager.hpp"

#include <OGRE/OgreEntity.h>

namespace vl
{

enum PREFAB
{
	PF_NONE,
	PF_PLANE,
	PF_SPHERE,
	PF_CUBE,
};

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
 *	Currently only supports prefabs
 */
class Entity : public MovableObject
{
public :
	/// Constructor
	/// Should not be called from user code, use SceneManager to create these
	Entity(std::string const &name, PREFAB type, vl::SceneManagerPtr creator);

	/// Should not be called from user code, use SceneManager to create these
	Entity(std::string const &name, std::string const &mesh_name, vl::SceneManagerPtr creator, bool use_new_mesh_manager = false);

	/// Internal used by slave mapping
	Entity(vl::SceneManagerPtr creator);

	/// Destructor
	virtual ~Entity(void);

	std::string const &getMeshName(void) const
	{ return _mesh_name; }

	bool isPrefab(void) const
	{ return _prefab != PF_NONE; }

	PREFAB getPrefab(void) const
	{ return _prefab; }

	void setCastShadows(bool shadows);

	bool getCastShadows(void) const
	{ return _cast_shadows; }

	void setMaterialName(std::string const &name);

	std::string const &getMaterialName(void) const
	{ return _material_name; }

	enum DirtyBits
	{
		DIRTY_MESH_NAME = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_PREFAB = vl::MovableObject::DIRTY_CUSTOM << 1,
		DIRTY_CAST_SHADOWS = vl::MovableObject::DIRTY_CUSTOM << 2,
		DIRTY_MATERIAL = vl::MovableObject::DIRTY_CUSTOM << 3,
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

	/// Name, _prefab and _mesh_name should not be changed after creation
	PREFAB _prefab;

	std::string _mesh_name;

	/// For new MeshManager
	bool _use_new_mesh_manager;
	vl::MeshRefPtr _mesh;

	bool _cast_shadows;
	std::string _material_name;

	Ogre::Entity *_ogre_object;

	// Save the loader pointer so it can be destroyed when not needed anymore
	EntityMeshLoadedCallback *_loader_cb;
};

inline std::ostream &
operator<<(std::ostream &os, vl::Entity const &ent)
{
	// @todo either mesh name or prefab should be printed
	os << "Entity : " << ent.getName() << " : mesh name " << ent.getMeshName() 
		<< " : prefab " << ent.getPrefab() 
		<< " : cast shadows " << ent.getCastShadows()
		<< std::endl;

	return os;
}

}	// namespace vl

#endif	// VL_ENTITY_HPP
