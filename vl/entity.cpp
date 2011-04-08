/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file entity.cpp
 */

#include "entity.hpp"

#include "scene_manager.hpp"
#include "scene_node.hpp"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSceneManager.h>

vl::Entity::Entity(std::string const &name, vl::PREFAB type, vl::SceneManager *creator)
	: _name(name)
	, _prefab(type)
	, _mesh_name()
	, _cast_shadows(true)
	, _creator(creator)
	, _parent(0)
	, _ogre_object(0)
{}

vl::Entity::Entity(std::string const &name, std::string const &mesh_name, vl::SceneManager *creator)
	: _name(name)
	, _prefab(PF_NONE)
	, _mesh_name(mesh_name)
	, _cast_shadows(true)
	, _creator(creator)
	, _parent(0)
	, _ogre_object(0)
{}

vl::Entity::~Entity(void)
{}

void 
vl::Entity::setCastShadows(bool shadows)
{
	if( _cast_shadows != shadows )
	{
		setDirty(DIRTY_CAST_SHADOWS);
		_cast_shadows = shadows;
	}
}

void 
vl::Entity::setMaterialName(std::string const &name)
{
	if( _material_name != name )
	{
		setDirty(DIRTY_MATERIAL);
		_material_name = name;
	}
}

void 
vl::Entity::setParent(vl::SceneNodePtr parent)
{
	_parent = parent;
	if( _ogre_object && _parent->getNative() )
	{
		if( _ogre_object->isAttached() )
		{
			if( _ogre_object->getParentSceneNode() != _parent->getNative() )
			{
				_ogre_object->detachFromParent();
				_parent->getNative()->attachObject(_ogre_object);
			}
		}
		else
		{
			_parent->getNative()->attachObject(_ogre_object);
		}
	}
}

void 
vl::Entity::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_NAME & dirtyBits )
	{
		msg << _name;
	}

	if( DIRTY_PREFAB & dirtyBits )
	{
		msg << _prefab;
	}

	if( DIRTY_MESH & dirtyBits )
	{
		msg << _mesh_name;
	}

	if( DIRTY_CAST_SHADOWS & dirtyBits )
	{
		msg << _cast_shadows;
	}
	
	if( DIRTY_MATERIAL & dirtyBits )
	{
		msg << _material_name;
	}
}

void 
vl::Entity::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	bool recreate = false;

	if( DIRTY_NAME & dirtyBits )
	{
		msg >> _name;
		recreate = true;
	}

	if( DIRTY_PREFAB & dirtyBits )
	{
		msg >> _prefab;
	}

	if( DIRTY_MESH & dirtyBits )
	{
		msg >> _mesh_name;
	}

	if( DIRTY_CAST_SHADOWS & dirtyBits )
	{
		msg >> _cast_shadows;
		if( _ogre_object )
		{ _ogre_object->setCastShadows(_cast_shadows); }
	}

	if( DIRTY_MATERIAL & dirtyBits )
	{
		msg >> _material_name;
		if( _ogre_object )
		{
			std::cout << "Setting material name to Ogre object" << std::endl;
			_ogre_object->setMaterialName(_material_name); 
		}
	}

	if( recreate )
	{ _createNative(); }
}


bool
vl::Entity::_createNative(void)
{
	if( _ogre_object )
	{ return true; }

	assert( _creator );
	assert( _creator->getNative() );
	assert( !_name.empty() );

	if( _creator->getNative()->hasEntity( _name ) )
	{
		_ogre_object = _creator->getNative()->getEntity( _name );
	}
	else if( !_mesh_name.empty() )
	{
		_ogre_object = _creator->getNative()->createEntity(_name, _mesh_name);
	}
	else if( PF_NONE != _prefab )
	{
		// Lets create the entity
		Ogre::SceneManager::PrefabType t;
		if( PF_PLANE == _prefab )
		{ t = Ogre::SceneManager::PT_PLANE; }
		else if( PF_SPHERE == _prefab )
		{ t = Ogre::SceneManager::PT_SPHERE; }
		else if(PF_CUBE == _prefab)
		{ t = Ogre::SceneManager::PT_CUBE; }
		else
		{
			std::cerr << "Trying to create unknow prefab entity. Not supported." << std::endl;
			return false;
		}
		_ogre_object = _creator->getNative()->createEntity(_name, t);
	}
	else
	{
		std::cerr << "Trying to create an entity without a mesh file" 
			<< " or prefab. Not supported." << std::endl;
		return false;
	}

	assert(_ogre_object);
	_ogre_object->setCastShadows(_cast_shadows);
	if( !_material_name.empty() )
	{ _ogre_object->setMaterialName(_material_name); }

	if( _parent )
	{ setParent(_parent); }

	return true;
}
