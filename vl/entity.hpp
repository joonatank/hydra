/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file entity.hpp
 */

#ifndef VL_ENTITY_HPP
#define VL_ENTITY_HPP

// Base class
#include "distributed.hpp"

#include <string>

#include "typedefs.hpp"

#include <OGRE/OgreMovableObject.h>

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
	Entity(std::string const &name, PREFAB type, vl::SceneManager *creator);

	/// Destructor
	virtual ~Entity(void);

	std::string const &getName(void) const
	{ return _name; }

	bool isPrefab(void) const
	{ return _prefab != PF_NONE; }

	PREFAB getPrefab(void) const
	{ return _prefab; }

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		// Not used yet
		DIRTY_MESH = vl::Distributed::DIRTY_CUSTOM << 1,
		// Will be removed after mesh is used
		DIRTY_PREFAB = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 3,
	};

	Ogre::MovableObject *getNative(void) const
	{ return _ogre_object; }

	// @todo needs a size (bounding box size is fine) and a scale

/// Virtual overrides
protected :

	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	bool _createNative(void);

	/// Name and _prefab should not be changed after creation
	std::string _name;

	PREFAB _prefab;

	vl::SceneManagerPtr _creator;

	Ogre::MovableObject *_ogre_object;
};

}	// namespace vl

#endif	// VL_ENTITY_HPP
