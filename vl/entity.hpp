/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file entity.hpp
 *
 *	@todo dynamically created Entities with meshes don't work
 */

#ifndef VL_ENTITY_HPP
#define VL_ENTITY_HPP

// Base class
#include "distributed.hpp"

#include <string>

#include "typedefs.hpp"

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

/**	@class Entity
 *	@brief Entity that can be drawn to the scene, is distributed
 *	Currently only supports prefabs
 */
class Entity : public vl::Distributed
{
public :
	/// Constructor
	/// Should not be called from user code, use SceneManager to create these
	Entity(std::string const &name, PREFAB type, vl::SceneManagerPtr creator);

	Entity(std::string const &name, std::string const &mesh_name, vl::SceneManagerPtr creator);

	/// Destructor
	virtual ~Entity(void);

	std::string const &getName(void) const
	{ return _name; }

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
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_MESH = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_PREFAB = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_CAST_SHADOWS = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_MATERIAL = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 5,
	};

	Ogre::MovableObject *getNative(void) const
	{ return _ogre_object; }

	void setParent(vl::SceneNodePtr parent);

	// @todo needs a size (bounding box size is fine) and a scale

/// Virtual overrides
protected :

	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	bool _createNative(void);

	/// Name, _prefab and _mesh_name should not be changed after creation
	std::string _name;
	
	PREFAB _prefab;

	std::string _mesh_name;

	bool _cast_shadows;
	std::string _material_name;

	vl::SceneManagerPtr _creator;

	vl::SceneNodePtr _parent;

	Ogre::Entity *_ogre_object;
};

}	// namespace vl

#endif	// VL_ENTITY_HPP
