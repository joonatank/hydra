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

	VertexData *vertexData = 0;
	if(mesh->sharedVertexData)
	{ vertexData = mesh->sharedVertexData; }
	else
	{ vertexData = mesh->getSubMesh(0)->vertexData; }

	assert(vertexData);

	SubMesh const *sm = mesh->getSubMesh(0);
	btIndexedMesh bt_data;
	
	// Ogre meshes use 16bit float for vertices
	bt_data.m_vertexType = PHY_FLOAT;

	assert(vertexData->buffer);
	bt_data.m_numVertices = vertexData->buffer->getNVertices();
	/// Assumes triangle lists
	/// @todo add support for operation type
	assert(sm->indexData.indexCount()%3 == 0);
	bt_data.m_numTriangles = sm->indexData.indexCount()/3;

	// Find the position schematic place in the buffer
	size_t pos_offset = 0;
	for(size_t i = 0; i < vertexData->vertexDeclaration.getElements().size(); ++i)
	{
		Ogre::VertexElement const &elem = vertexData->vertexDeclaration.getElements().at(i);
		if(elem.getSemantic() != Ogre::VES_POSITION)
		{ pos_offset += elem.getSize(); }
		else
		{ break; }
	}

	// Set the pointer to vertex buffer
	bt_data.m_vertexBase = (const unsigned char *)vertexData->buffer->_buffer + pos_offset;
	// Stride is the size of the type
	// so vertex stride is the size of a vertex in the buffer
	bt_data.m_vertexStride = vertexData->buffer->getVertexSize();

	// set the pointer to index buffer
	PHY_ScalarType indexType = PHY_INTEGER;
	if(sm->indexData.getIndexSize() == IT_32BIT)
	{
		bt_data.m_triangleIndexBase = (const unsigned char *)sm->indexData.getBuffer32();
		bt_data.m_triangleIndexStride = 3*sizeof(uint32_t);
		indexType = PHY_INTEGER;
	}
	else
	{
		bt_data.m_triangleIndexBase = (const unsigned char *)sm->indexData.getBuffer16();
		bt_data.m_triangleIndexStride = 3*sizeof(uint16_t);
		indexType = PHY_SHORT;
	}

	// Pass the index type because the one in btIndexedMesh is overwriten
	bt_mesh->addIndexedMesh(bt_data, indexType);
	
	btVector3 aabb_min = math::convert_bt_vec(mesh->getBounds().getMinimum());
	btVector3 aabb_max = math::convert_bt_vec(mesh->getBounds().getMaximum());
	bt_mesh->setPremadeAabb(aabb_min, aabb_max);
}
