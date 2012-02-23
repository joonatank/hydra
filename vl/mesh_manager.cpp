/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/// Interface
#include "mesh_manager.hpp"

/// Necessary for writing and reading mesh files
#include "mesh_serializer.hpp"

/// Necessary for not implemented exception
#include "base/exceptions.hpp"
/// Necessary for M_PI
#include "math/math.hpp"

#include "resource_manager.hpp"

// Necessary for comparing sub mesh materials
#include "material.hpp"

#include <Procedural.h>

namespace {

template <typename T>
vl::MeshRefPtr make_to_mesh(std::string const &name, Procedural::MeshGenerator<T> const &generator)
{
	vl::MeshRefPtr mesh(new vl::Mesh(name));
	
	Procedural::TriangleBuffer tbuffer;
	generator.addToTriangleBuffer(tbuffer);	

	vl::SubMesh *sub = mesh->createSubMesh();
	sub->setMaterial("BaseWhiteNoLighting");

	// Using independent vertex data because of instancing
	// also shared vertex data provides very little benefits

	// copy vertex data
	sub->vertexData = new vl::VertexData;
	sub->useSharedGeometry = false;
	sub->vertexData->vertexDeclaration.addSemantic(Ogre::VES_POSITION, Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addSemantic(Ogre::VES_NORMAL, Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addSemantic(Ogre::VES_TEXTURE_COORDINATES, Ogre::VET_FLOAT2);

	for(std::vector<Procedural::TriangleBuffer::Vertex>::const_iterator it 
			= tbuffer.getVertices().begin(); it != tbuffer.getVertices().end();it++)
	{
		vl::Vertex vert;
		vert.position = it->mPosition;
		vert.normal = it->mNormal;
		vert.uv = it->mUV;
		sub->vertexData->addVertex(vert);
	}

	// copy index data

	// @todo should check if we need to use 32-bit buffer
	// forcing the use of 32 bit index buffer
	//sub->indexData.setIndexSize(vl::IT_32BIT);
	sub->indexData.setIndexCount(tbuffer.getIndices().size());
	for(size_t i = 0; i < tbuffer.getIndices().size(); ++i)
	{
		sub->indexData.set(i, (uint32_t)tbuffer.getIndices().at(i));
	}

	/* tangent vector calculation is not yet implemented */

	mesh->calculateBounds();

	return mesh;
}

}	// unamed namespace


/// Callbacks
void 
vl::ManagerMeshLoadedCallback::meshLoaded(vl::MeshRefPtr mesh)
{
	assert(owner);
	owner->meshLoaded(name, mesh);
}

vl::MasterMeshLoaderCallback::MasterMeshLoaderCallback(vl::ResourceManagerRefPtr res_man)
	: manager(res_man)
{}

/// Blocks till the mesh is loaded and passes a valid mesh to callback
void 
vl::MasterMeshLoaderCallback::loadMesh(std::string const &fileName, vl::MeshLoadedCallback *cb)
{
	std::clog << "vl::MasterMeshLoaderCallback::loadMesh : " << fileName << std::endl;
	if(!manager)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer());
	}
	assert(cb);

	Resource data;
	manager->loadMeshResource(fileName, data);

	/// @todo this needs to convert the Resource to a Mesh
	MeshSerializer ser;
	MeshRefPtr mesh(new Mesh(fileName));
	ser.readMesh(mesh, data);

	std::clog << "Read mesh " << mesh->getName() << " from file resource." << std::endl;
	
	cb->meshLoaded(mesh);
}

/// ------------------------------- MeshManager ------------------------------
vl::MeshRefPtr
vl::MeshManager::loadMesh(std::string const &file_name)
{
	vl::MeshRefPtr mesh;
	/// Mesh has already been created
	if(hasMesh(file_name))
	{
		mesh = getMesh(file_name);
	}
	else
	{
		if(!_load_callback)
		{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

		MeshLoadedCallback *cb = new ManagerMeshLoadedCallback(file_name, this);
		/// Blocking callback
		_load_callback->loadMesh(file_name, cb);
		delete cb;	// can delete because loader was blocking
		
		// mesh is now loaded
		assert(hasMesh(file_name));
		mesh = getMesh(file_name);
	}

	return mesh;
}

void
vl::MeshManager::loadMesh(std::string const &file_name,  MeshLoadedCallback *cb)
{
	assert(cb);

	if(hasMesh(file_name))
	{
		cb->meshLoaded(getMesh(file_name));
	}
	else
	{
		ListenerMap::iterator iter = _waiting_for_loading.find(file_name);
		if(iter != _waiting_for_loading.end())
		{
			/// assume that this is not called twice with same cb
			iter->second.push_back(cb);
			// No need to call load it's already called
		}
		else
		{
			_waiting_for_loading[file_name].push_back(cb);
			_load_callback->loadMesh(file_name, new ManagerMeshLoadedCallback(file_name, this));
		}
	}
}

void
vl::MeshManager::writeMesh(vl::MeshRefPtr, std::string const &file_name)
{
	std::clog << "vl::MeshManager::writeMesh" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

vl::MeshRefPtr
vl::MeshManager::createPlane(std::string const &name, Ogre::Real size_x, Ogre::Real size_y, 
	Ogre::Vector3 normal, uint16_t tesselation_x, uint16_t tesselation_y)
{
	if(normal.isZeroLength() || tesselation_x == 0 || tesselation_y == 0 || size_x <= 0 || size_y <= 0)
	{
		// @todo replace with real exception
		BOOST_THROW_EXCEPTION(vl::exception());
	}
	if(hasMesh(name))
	{
		BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name));
	}

	/// Mesh is created with the assumption that normal is +y
	/// so we rotate every vertex to the normal
	Ogre::Quaternion vert_rot = Ogre::Vector3(0, 1, 0).getRotationTo(normal);

	MeshRefPtr mesh(new Mesh(name));
	SubMesh *sub = mesh->createSubMesh();

	sub->vertexData = new VertexData;
	sub->useSharedGeometry = false;
	sub->vertexData->vertexDeclaration.addSemantic(Ogre::VES_POSITION, Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addSemantic(Ogre::VES_NORMAL, Ogre::VET_FLOAT3);
	sub->vertexData->vertexDeclaration.addSemantic(Ogre::VES_TEXTURE_COORDINATES, Ogre::VET_FLOAT2);
	/// @todo add tangents

	uint16_t M = tesselation_x;
	uint16_t N = tesselation_y;
	for(uint16_t m = 0; m < M+1; ++m)
	{
		for(uint16_t n = 0; n < N+1; ++n)
		{
			Vertex vert;
			Ogre::Vector3 pos(size_x*m/M - size_x/2, 0, size_y*n/N - size_y/2);
			vert.position = vert_rot*pos;
			vert.normal = normal;
			vert.uv = Ogre::Vector2(((double)m)/M, ((double)n)/N);
			sub->vertexData->addVertex(vert);
		}
	}

	mesh->calculateBounds();

	/// @todo add material (or not?) some clear default would be good
	for(uint16_t m = 0; m < M; ++m)
	{
		for(uint16_t n = 0; n < N; ++n)
		{
			sub->addFace(m*(N+1)+n, m*(N+1)+(n+1), (m+1)*(N+1)+n);
			sub->addFace((m+1)*(N+1)+n, m*(N+1)+(n+1), (m+1)*(N+1)+(n+1));
		}
	}

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createSphere(std::string const &name, Ogre::Real radius, uint16_t longitude, uint16_t latitude)
{
	std::clog << "vl::MeshManager::createSphere" << std::endl;

	if(hasMesh(name))
	{ BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name)); }

	Procedural::SphereGenerator generator;
	generator.setRadius(radius).setUTile(longitude).setVTile(latitude);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);

	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createCube(std::string const &name, Ogre::Vector3 size)
{
	std::clog << "vl::MeshManager::createCube" << std::endl;
	if(size.isZeroLength())
	{
		// @todo replace with real exception
		BOOST_THROW_EXCEPTION(vl::exception());
	}
	if(hasMesh(name))
	{
		BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name));
	}

	Procedural::BoxGenerator generator;
	/// @todo add configurable segments
	generator.setSize(size).setNumSegX(1).setNumSegY(1).setNumSegZ(1);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);

	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createCylinder(std::string const &name, vl::scalar radius, 
		vl::scalar height, uint16_t seg_height, uint16_t seg_radius)
{
	std::clog << "vl::MeshManager::createCylinder" << std::endl;

	if(hasMesh(name))
	{ BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name)); }

	Procedural::CylinderGenerator generator;
	generator.setRadius(radius).setHeight(height).setNumSegHeight(seg_height).setNumSegBase(seg_radius);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);
	
	// Fix the origin to middle of the cylinder
	assert(mesh->getNumSubMeshes() > 0);
	SubMesh *sub = mesh->getSubMesh(0);
	for(size_t i = 0; i < sub->vertexData->getNVertices(); ++i)
	{
		sub->vertexData->getVertex(i).position -= mesh->getBounds().getCenter();
	}	
	mesh->calculateBounds();

	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createCapsule(std::string const &name, vl::scalar radius, 
		vl::scalar height, uint16_t seg_height, uint16_t seg_radius, uint16_t segments)
{
	std::clog << "vl::MeshManager::createCapsule with height " << height << std::endl;

	if(hasMesh(name))
	{ BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name)); }

	Procedural::CapsuleGenerator generator;
	// divide height by two because we use height to mean the actual height of
	// the capsule (as in bounding box size) not height from center.
	generator.setRadius(radius).setHeight(height/2).setNumSegHeight(seg_height).setNumRings(seg_radius).setNumSegments(segments);
	vl::MeshRefPtr mesh = make_to_mesh(name, generator);

	std::clog << mesh->getBounds() << std::endl;
	assert(mesh);

	_meshes[name] = mesh;
	return mesh;
}


vl::MeshRefPtr
vl::MeshManager::createPrefab(std::string const &type_name)
{
	vl::MeshRefPtr mesh;
	if(type_name == "prefab_plane")
	{
		if(!hasMesh(type_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			mesh = createPlane(type_name, 20, 20);
		}
	}
	else if(type_name == "prefab_cube")
	{
		if(!hasMesh(type_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			mesh = createCube(type_name);
		}
	}
	else if(type_name == "prefab_sphere")
	{
		if(!hasMesh(type_name))
		{
			/// Creating a mesh leaves it in the manager for as long as
			/// cleanup is called on the manager, which gives us enough
			/// time even if we don't store the ref pointer.
			mesh = createSphere(type_name);
		}
	}
	else
	{ BOOST_THROW_EXCEPTION(vl::invalid_param() << vl::desc("Invalid PREFAB type name : " + type_name)); }

	return mesh;
}

vl::MeshRefPtr 
vl::MeshManager::getMesh(std::string const &name)
{
	MeshMap::iterator iter = _meshes.find(name);
	if(iter != _meshes.end())
	{ return iter->second; }

	/// @todo replace with real exception
	BOOST_THROW_EXCEPTION(vl::exception());
}

bool 
vl::MeshManager::hasMesh(std::string const &name) const
{
	MeshMap::const_iterator iter = _meshes.find(name);
	if(iter != _meshes.end())
	{ return true; }

	return false;
}

void
vl::MeshManager::cleanup_unused(void)
{
	std::clog << "vl::MeshManager::cleanup_unused" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

vl::MeshRefPtr 
vl::MeshManager::createMesh(std::string const &name)
{
	MeshRefPtr mesh(new Mesh(name));
	_meshes[name] = mesh;
	return mesh;
}

bool
vl::MeshManager::checkMaterialUsers(vl::MaterialRefPtr mat)
{
	bool retval = false;

	// temp array for the ones that are left
	std::vector<Ogre::SubEntity *> remaining;
	for(std::vector<Ogre::SubEntity *>::iterator iter = _og_sub_entities.begin();
		iter != _og_sub_entities.end(); ++iter)
	{
		// sub mesh has the original material name before Ogre overwrote it with "BaseWhite"
		if((*iter)->getSubMesh()->getMaterialName() == mat->getName())
		{
			assert(mat->getNative().get());
			(*iter)->setMaterial(mat->getNative());
			retval = true;
		}
		else
		{ remaining.push_back(*iter); }
	}

	_og_sub_entities = remaining;

	// @todo needs to loop through Ogre meshes also... as they are copies of our meshes. Argh.
	//Ogre::MeshManager::getSingleton().
	return retval;
}

void 
vl::MeshManager::meshLoaded(std::string const &mesh_name, vl::MeshRefPtr mesh)
{
	std::clog << "vl::MeshManager::meshLoaded : " << mesh_name 
		<< " :  waiting for loading size " << _waiting_for_loading.size() << std::endl;
	// add to loaded stack
	assert(!hasMesh(mesh_name));
	_meshes[mesh_name] = mesh;

	// check listeners
	ListenerMap::iterator iter = _waiting_for_loading.find(mesh_name);
	if(iter != _waiting_for_loading.end())
	{
		for(size_t i = 0; i < iter->second.size(); ++i)
		{
			iter->second.at(i)->meshLoaded(mesh);
		}
		_waiting_for_loading.erase(iter);
	}
	// else this was called using a blocking loader
}

void
vl::MeshManager::_addSubEntityWithInvalidMaterial(Ogre::SubEntity *se)
{
	if(std::find(_og_sub_entities.begin(), _og_sub_entities.end(), se)
		== _og_sub_entities.end())
	{ _og_sub_entities.push_back(se); }
}
