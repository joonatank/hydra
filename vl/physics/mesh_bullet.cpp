/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file physics/mesh_bullet.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "mesh_bullet.hpp"

#include "math/conversion.hpp"

void 
vl::convert_bullet_geometry(vl::Mesh const *mesh, btTriangleIndexVertexArray *bt_mesh)
{
	// empty mesh
	if(mesh->getNumSubMeshes() == 0)
	{ return; }

	/// Supports only single sub meshes for now
	assert(mesh->getNumSubMeshes() == 1);
	/// Supports only shared geometry for now
	assert(mesh->sharedVertexData);

	SubMesh const *sm = mesh->getSubMesh(0);
	btIndexedMesh bt_data;
	
	// Stride is the size of the type
	// assuming 16bit index buffer, we will change it to 32bit later if necessary
	bt_data.m_triangleIndexStride = sizeof(uint16_t)*3;

	PHY_ScalarType indexType = PHY_SHORT;
	if(sm->indexData.getIndexSize() == IT_32BIT)
	{
		indexType = PHY_INTEGER;
		bt_data.m_triangleIndexStride = sizeof(uint32_t)*3;
	}
	// Ogre meshes use 16bit float for vertices
	bt_data.m_vertexType = PHY_FLOAT;
	bt_data.m_vertexStride = sizeof(float)*3;

	bt_data.m_numVertices = mesh->sharedVertexData->getNVertices();
	/// Assumes triangle lists
	/// @todo add support for operation type
	assert(sm->indexData.indexCount()%3 == 0);
	bt_data.m_numTriangles = sm->indexData.indexCount()/3;

	// copy the vertex buffer
	size_t vertex_size = 3*sizeof(float);
	assert(vertex_size == sizeof(Ogre::Vector3));

	/// @todo these will leak memory when the bullet mesh is destroyed
	/// @todo this is inefficient, better would be that the VertexData structure
	/// holds the vertex positions in a continous memory block (no need to copy them)
	/// especially as Bullet can reuse the memory block.
	unsigned char *vbuf = new unsigned char[mesh->sharedVertexData->getNVertices()*vertex_size];
	for(size_t i = 0; i < mesh->sharedVertexData->getNVertices(); ++i)
	{
		::memcpy(vbuf+i*vertex_size, &mesh->sharedVertexData->getVertex(i).position, vertex_size);
	}

	bt_data.m_vertexBase = vbuf;

	unsigned char *ibf = 0;
	if(sm->indexData.getIndexSize() == IT_32BIT)
	{
		//ibf = (unsigned char const *)sm->indexData.getBuffer32();
		size_t arr_size = sm->indexData.indexCount()*sizeof(uint32_t);
		ibf = new unsigned char[arr_size];
		memcpy(ibf, sm->indexData.getBuffer32(), arr_size);
	}
	else
	{
		//ibf = (unsigned char const *)sm->indexData.getBuffer16(); 
		size_t arr_size = sm->indexData.indexCount()*sizeof(uint16_t);
		ibf = new unsigned char[arr_size];
		memcpy(ibf, sm->indexData.getBuffer16(), arr_size);
	}
	bt_data.m_triangleIndexBase = ibf;

	// Pass the index type because the one in btIndexedMesh is overwriten
	bt_mesh->addIndexedMesh(bt_data, indexType);
	
	btVector3 aabb_min = math::convert_bt_vec(mesh->getBounds().getMinimum());
	btVector3 aabb_max = math::convert_bt_vec(mesh->getBounds().getMaximum());
	bt_mesh->setPremadeAabb(aabb_min, aabb_max);
}
