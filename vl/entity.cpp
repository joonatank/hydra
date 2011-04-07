/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file entity.cpp
 */

#include "entity.hpp"

#include "scene_manager.hpp"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSceneManager.h>

vl::Entity::Entity(std::string const &name, vl::PREFAB type, vl::SceneManager *creator)
	: _name(name), _prefab(type), _creator(creator), _ogre_object(0)
{
}

vl::Entity::~Entity(void)
{

}

/*
void 
vl::Entity::setPrefab(vl::PREFAB type)
{
	if( _prefab != type )
	{
		setDirty(DIRTY_PREFAB);
		_prefab = type;
	}
}
*/

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

	if( recreate )
	{ _findEntity(); }
}


bool
vl::Entity::_findEntity(void)
{
	if( _ogre_object )
	{ return true; }

	assert( _creator );
	assert( _creator->getNative() );
	assert( !_name.empty() );

	if( _creator->getNative()->hasEntity( _name ) )
	{
		_ogre_object = _creator->getNative()->getEntity( _name );

		return true;
	}
	else
	{
		if( PF_NONE == _prefab )
		{
			std::cerr << "Trying to create NON prefab entity. Not supported." << std::endl;
			return false;
		}

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

		return true;
	}
}
