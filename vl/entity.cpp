/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-04
 *	@file entity.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */


#include "entity.hpp"

#include "scene_manager.hpp"
#include "scene_node.hpp"
// Necessary for loading meshes with the new interface
#include "mesh_manager.hpp"
#include "mesh_ogre.hpp"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreInstancedEntity.h>
#include <OGRE/OgreInstanceBatch.h>

/// ------------------------------------- Global -----------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::Entity const &ent)
{
	// @todo either mesh name or prefab should be printed
	os << "Entity : " << ent.getName()
		<< " : material " << ent.getMaterialName()
		<< " : mesh " << ent.getMeshName();
	os << " : cast shadows " << ent.getCastShadows() << std::endl;

	return os;
}

/// -------------------- EntityMeshLoadedCallback ----------------------------
vl::EntityMeshLoadedCallback::EntityMeshLoadedCallback(Entity *ent)
	: owner(ent)
{
	assert(owner);
}

void 
vl::EntityMeshLoadedCallback::meshLoaded(vl::MeshRefPtr mesh)
{
	owner->meshLoaded(mesh);
}

/// -------------------------------------- Entity ----------------------------
/// -------------------------------------- Public ----------------------------
vl::Entity::Entity(std::string const &name, std::string const &mesh_name, vl::SceneManagerPtr creator, bool use_new_mesh_manager)
	: MovableObject(name, creator)
{
	_clear();
	_mesh_name = mesh_name;
	_use_new_mesh_manager = use_new_mesh_manager;
	if(_use_new_mesh_manager)
	{
		_mesh = creator->getMeshManager()->loadMesh(mesh_name);
	}
}

vl::Entity::Entity(vl::SceneManagerPtr creator)
	: MovableObject("", creator)
{
	_clear();
}

vl::Entity::~Entity(void)
{
	if(_ogre_object)
	{
		assert(_creator->getNative());
		_creator->getNative()->destroyMovableObject(_ogre_object);
	}
}

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
vl::Entity::setInstanced(bool enable)
{
	if(_is_instanced != enable)
	{
		setDirty(DIRTY_INSTANCED);
		_is_instanced = enable;
	}
}

void 
vl::Entity::meshLoaded(vl::MeshRefPtr mesh)
{
	_mesh = mesh;

	if(!Ogre::MeshManager::getSingleton().resourceExists(_mesh_name))
	{
		Ogre::MeshPtr og_mesh = vl::create_ogre_mesh(_mesh_name, mesh);
	}
	_ogre_object = _createNativeEntity(_name, _mesh_name);


	// Check materials
	if(!_is_instanced)
	{
		Ogre::Entity *ent = (Ogre::Entity *)_ogre_object;
		for(uint16_t i = 0; i < ent->getNumSubEntities(); ++i)
		{
			Ogre::SubEntity *og_se = ent->getSubEntity(i);
			if(og_se->getMaterialName() == "BaseWhite")
			{
				// @todo this of course should add listener to MaterialManager
				// and it should be mesh attribute, but as our architecture does not
				// support it...
				// Empty sub mesh materials are purposefully so, they are usually overriden
				// from entity and in any case would make it impossible to load a such material...
				if(!og_se->getSubMesh()->getMaterialName().empty())
				{ _creator->getMeshManager()->_addSubEntityWithInvalidMaterial(og_se); }
			}
		}
	}

	delete _loader_cb;
	_loader_cb = 0;

	_finishCreateNative();
}

vl::MovableObjectPtr
vl::Entity::clone(std::string const &append_to_name) const
{
	if(append_to_name.empty())
	{ return MovableObject::clone(); }

	EntityPtr ent = 0;
	if(_use_new_mesh_manager)
	{ ent = _creator->createEntity(_name + append_to_name, _mesh_name, _use_new_mesh_manager); }
	else
	{ ent = _creator->createEntity(_name + append_to_name, _mesh_name); }

	ent->setCastShadows(_cast_shadows);
	ent->setMaterialName(_material_name);
	ent->setVisible(_visible);
	ent->setInstanced(_is_instanced);

	return ent;
}

void 
vl::Entity::doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( DIRTY_MESH_NAME & dirtyBits )
	{
		msg << _mesh_name << _use_new_mesh_manager;
	}

	if( DIRTY_CAST_SHADOWS & dirtyBits )
	{
		msg << _cast_shadows;
	}
	
	if( DIRTY_MATERIAL & dirtyBits )
	{
		msg << _material_name;
	}

	if( DIRTY_INSTANCED & dirtyBits )
	{
		msg << _is_instanced;
	}
}

void 
vl::Entity::doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_MESH_NAME & dirtyBits )
	{
		msg >> _mesh_name >> _use_new_mesh_manager;
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
		if( _ogre_object && !_is_instanced )
		{
			Ogre::Entity *ent = (Ogre::Entity *)_ogre_object;
			ent->setMaterialName(_material_name);
		}
		else if(_ogre_object)
		{
			std::clog << "WARNING : " << " reseting material for instanced entities is not supported." << std::endl;
		}
	}

	if( DIRTY_INSTANCED & dirtyBits )
	{
		msg >> _is_instanced;
		// For now we don't support changing from not instanced to instanced entities
		assert(!_ogre_object);
	}
}

bool
vl::Entity::_doCreateNative(void)
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
		if( _use_new_mesh_manager )
		{
			_loader_cb =  new EntityMeshLoadedCallback(this);
			_creator->getMeshManager()->loadMesh(_mesh_name, _loader_cb);
		}
		else
		{
			_ogre_object = _createNativeEntity(_name, _mesh_name);
		}
	}
	else
	{
		std::clog << "Trying to create an entity without a mesh file" 
			<< " or prefab. Not supported." << std::endl;
		return false;
	}

	if(_ogre_object)
	{
		return _finishCreateNative();
	}

	return true;
}

bool 
vl::Entity::_finishCreateNative(void)
{
	assert(_ogre_object);

	/// Really quick and dirty way to avoid problems when there is
	/// no uv coordinates 
	try {
		// TODO this will not work for objects without UVs
		// Will it try to use it when there is no UVs?
		// TODO this might also be really slow, we should probably have materails
		// that need tangents and those request the rebuild.
		// Also a more long term solution would be to build the tangents
		// into the meshe files.
		// TODO this does not work for the Ogre test object for some reason
		// it still has the zero tangents even though it has UVs.
		if(!_is_instanced)
		{
			Ogre::Entity *ent = (Ogre::Entity *)_ogre_object;
			Ogre::MeshPtr mesh = ent->getMesh();
			unsigned short src, dest;
			if (!mesh->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
			{
				mesh->buildTangentVectors(Ogre::VES_TANGENT, src, dest);
			}
		}
	}
	catch( Ogre::Exception const &e)
	{
		std::cout << "Exception : " << e.what() << std::endl;
	}

	_ogre_object->setCastShadows(_cast_shadows);
	if( !_material_name.empty() )
	{
		if(!_is_instanced)
		{
			Ogre::Entity *ent = (Ogre::Entity *)_ogre_object;
			ent->setMaterialName(_material_name); 
		}
	}

	/// Reset parent for those that are background loaded
	if(_parent && !_ogre_object->isAttached())
	{
		_parent->getNative()->attachObject(_ogre_object);
	}

	return true;
}

void 
vl::Entity::_clear(void)
{
	_cast_shadows = true;
	_use_new_mesh_manager = false;
	_is_instanced = false;
	_ogre_object = 0;
	_loader_cb = 0;
}

Ogre::MovableObject *
vl::Entity::_createNativeEntity(std::string const &name, std::string const &mesh_name)
{
	assert(_creator);
	Ogre::SceneManager *og_sm = _creator->getNative();
	assert(og_sm);

	if(_is_instanced)
	{
		Ogre::InstanceManager *inst_mgr = 0;
		// Create instance manager if one does not already exist
		// managers are mesh specific
		if(!og_sm->hasInstanceManager(mesh_name))
		{
			std::clog << "Entity : Instanced : creating new manager : " << mesh_name << std::endl;
			// parameters
			uint32_t const inst_per_patch = 80;
			// Technique chosen on basis of beign really fast when there is no sketal animation
			Ogre::InstanceManager::InstancingTechnique technique = Ogre::InstanceManager::HWInstancingBasic; //HWInstancingBasic;
			// Other possibilities
			//	ShaderBased
			//	TextureVTF
			//	HWInstancingVTF

			inst_mgr = og_sm->createInstanceManager(mesh_name, mesh_name, 
				Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, technique, inst_per_patch);
		}
		else
		{
			std::clog << "Entity : Instanced : retrieving manager : " << mesh_name << std::endl;
			inst_mgr = og_sm->getInstanceManager(mesh_name);
		}
		
		if(_mesh)
		{
			// Instancing only works if the sub mesh has independ geometry
			assert(_mesh->getNumSubMeshes() > 0);
			assert(_mesh->getSubMesh(0)->vertexData);
		}
		assert(inst_mgr);
		assert(!_material_name.empty());
		// @todo does this work if the Mesh defines a material and we don't?
		// Material needs to be derived from specific hw instance template
		// but we can not check it here.
		Ogre::InstancedEntity *inst = inst_mgr->createInstancedEntity(_material_name);
		//inst->_getOwner()->getParentSceneNode()->showBoundingBox(true);
		return inst;
	}
	else
	{
		return og_sm->createEntity(name, mesh_name);
	}
}
