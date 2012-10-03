/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh_ogre.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	Ogre conversion from our Mesh structure, 
 *	in separate file so that the Ogre dependency is easily avoided (or can in future)
 */

#include "mesh_ogre.hpp"

#include <OGRE/OgreMesh.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreDefaultHardwareBufferManager.h>
#include <OGRE/OgreMeshManager.h>

/// ------------------------------- Global -----------------------------------
Ogre::MeshPtr
vl::create_ogre_mesh(std::string const &name, vl::MeshRefPtr mesh)
{
	Ogre::MeshPtr og_mesh = Ogre::MeshManager::getSingleton().createManual(name, 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// Copy the shared geometry
	// Skip empty shared geometry
	if(mesh->sharedVertexData && mesh->sharedVertexData->buffer->getNVertices() > 0) 
	{
		// Create shared VertexData
		og_mesh->sharedVertexData = new Ogre::VertexData;
		convert_ogre_geometry(mesh->sharedVertexData, og_mesh->sharedVertexData);
	}

	// Set bounding box
	og_mesh->_setBoundingSphereRadius(mesh->getBoundingSphereRadius());
	og_mesh->_setBounds(mesh->getBounds(), false);

	// Convert submeshes
	convert_ogre_submeshes(mesh.get(), og_mesh.get());

	return og_mesh;
}

void 
vl::convert_ogre_geometry(vl::VertexData const *vertexData, Ogre::VertexData *og_vertexData)
{
	assert(vertexData && og_vertexData);

	Ogre::VertexDeclaration *decl = og_vertexData->vertexDeclaration;
	
	// What is bufCount? it's the binding we are using. Currently we only support one bindibng
	// this need to be changed if we want to properly read all Ogre files
	// like the ogre.mesh file.
	const unsigned short bufCount = 0;
	unsigned short totalTexCoords = 0; // across all buffers

	// Add elements
	std::vector<Ogre::VertexElement> const &elements = vertexData->vertexDeclaration.getElements();
	for(size_t i = 0; i < elements.size(); ++i)
	{
		size_t index = elements.at(i).getIndex();
		size_t source = elements.at(i).getSource();
		size_t offset = elements.at(i).getOffset();
		decl->addElement(source, offset, elements.at(i).getType(), elements.at(i).getSemantic(), index);
		assert(bufCount == source);
	}

	og_vertexData->vertexCount = vertexData->buffer->getNVertices();
	
	assert(og_vertexData->vertexDeclaration->getVertexSize(bufCount) == vertexData->buffer->getVertexSize());

	// Now create the vertex buffer
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(vertexData->buffer->getVertexSize(), og_vertexData->vertexCount, 
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	/// direct copy of the Vertex data
	vbuf->writeData(0, vertexData->buffer->size(), vertexData->buffer->_buffer, true);
	
	// Bind it
	Ogre::VertexBufferBinding *bind = og_vertexData->vertexBufferBinding;
	bind->setBinding(bufCount, vbuf);

	/// @todo add support for multiple bindings

	// Vertexbuffer done
}

void 
vl::convert_ogre_submeshes(vl::Mesh const *mesh, Ogre::Mesh *og_mesh)
{
	for(size_t i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
		// All children should be submeshes 
		Ogre::SubMesh *og_sm = og_mesh->createSubMesh();

		vl::SubMesh const *sm = mesh->getSubMesh(i); 
		convert_ogre_submesh(sm, og_sm);

		if(!sm->getName().empty())
		{ og_mesh->nameSubMesh(sm->getName(), i); }
    }
}

void 
vl::convert_ogre_submesh(vl::SubMesh const *sm, Ogre::SubMesh *og_sm)
{
	assert(sm);
	assert(og_sm);

	if(!sm->getMaterial().empty())
	{ og_sm->setMaterialName(sm->getMaterial()); }

	if(sm->indexData.indexCount() == 0)
	{
		std::clog << "Something really fishy SubMesh index count = 0" << std::endl;
	}
	else
	{
		// Indeces
		og_sm->operationType = sm->operationType;
		og_sm->indexData->indexCount = sm->indexData.indexCount();

		if(sm->indexData.getIndexSize() == vl::IT_32BIT)
		{
			// Allocate space
			Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
				createIndexBuffer(Ogre::HardwareIndexBuffer::IT_32BIT, 
					og_sm->indexData->indexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC);

			/// Upload the index data to the card
			ibuf->writeData(0, ibuf->getSizeInBytes(), sm->indexData.getBuffer32(), true);

			og_sm->indexData->indexBuffer = ibuf;
		}
		else
		{
			// Allocate space
			Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
				createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, 
					og_sm->indexData->indexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC);
			
			/// Upload the index data to the card
			ibuf->writeData(0, ibuf->getSizeInBytes(), sm->indexData.getBuffer16(), true);

			og_sm->indexData->indexBuffer = ibuf;
		}
	}


	// Geometry
	og_sm->useSharedVertices = sm->useSharedGeometry;
	if(!sm->useSharedGeometry)
	{
		assert(sm->vertexData);
		og_sm->vertexData = new Ogre::VertexData;
		convert_ogre_geometry(sm->vertexData, og_sm->vertexData);
	}

	// texture aliases, NOT supported
	/*
	TiXmlElement* textureAliasesNode = smElem->FirstChildElement("textures");
	if(textureAliasesNode)
	{ readTextureAliases(textureAliasesNode, sm); }
	*/

	// Bone assignments, NOT supported
	/*
	TiXmlElement* boneAssigns = smElem->FirstChildElement("boneassignments");
	if(boneAssigns)
	{ readBoneAssignments(boneAssigns, sm); }
	*/
}
