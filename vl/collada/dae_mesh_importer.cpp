/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file collada/dae_mesh_importer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *
 
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/// Interface
#include "dae_mesh_importer.hpp"

#include <COLLADAFramework/COLLADAFWUniqueId.h>
#include <COLLADAFramework/COLLADAFWTriangles.h>
#include <COLLADAFramework/COLLADAFWTrifans.h>
#include <COLLADAFramework/COLLADAFWTristrips.h>
#include <COLLADAFramework/COLLADAFWPolygons.h>

#include <COLLADABUPlatform.h>

#ifdef COLLADABU_OS_WIN
#	include <windows.h>
#endif

/// Necessary because we create meshes here
#include "mesh_manager.hpp"

/// Necessary for typedefs
#include "dae_importer.hpp"

namespace {

Ogre::Vector3 convert_vertex_data(COLLADAFW::MeshVertexData const &data, size_t index)
{
	Ogre::Vector3 v;
	if(data.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
	{
		const double* arr = data.getDoubleValues()->getData();
		arr += 3*index;
		//position = mCurrentRotationMatrix * position + mCurrentTranslationVector;
		v.x = (Ogre::Real)arr[0];
		v.y = (Ogre::Real)arr[1];
		v.z = (Ogre::Real)arr[2];
	}
	else
	{
		const float* arr = data.getFloatValues()->getData();
		arr += 3*index;
		//position = mCurrentRotationMatrix * position + mCurrentTranslationVector;
		v.x = (Ogre::Real)arr[0];
		v.y = (Ogre::Real)arr[1];
		v.z = (Ogre::Real)arr[2];
	}

	return v;
}

Ogre::Vector2 convert_uvs(COLLADAFW::MeshVertexData const &data, size_t index)
{
	Ogre::Vector2 uv;
	if(data.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
	{
		const double* arr = data.getDoubleValues()->getData();
		arr += 2*index;
		uv.x = (Ogre::Real)arr[0];
		uv.y = (Ogre::Real)arr[1];
	}
	else
	{
		const float* arr = data.getFloatValues()->getData();
		arr += 2*index;
		uv.x = (Ogre::Real)arr[0];
		uv.y = (Ogre::Real)arr[1];
	}

	return uv;
}

}	// unamed namespace


//------------------------------
vl::dae::MeshImporter::MeshImporter(MeshManagerRefPtr mesh_manager, 
	COLLADAFW::Mesh const *mesh, bool flat_shading)
	: _collada_mesh(mesh)
	, _mesh_manager(mesh_manager)
	, _flat_shading(flat_shading)
{}

//------------------------------
vl::dae::MeshImporter::~MeshImporter()
{
}


//------------------------------
bool
vl::dae::MeshImporter::write()
{
	std::clog << "vl::dae::MeshImporter::write" << std::endl;

//	bool skelAnim = _collada_mesh->hasSkeleton();

	assert(_mesh_manager);
	assert(_collada_mesh);
	_mesh = _mesh_manager->createMesh(_collada_mesh->getName());

	_mesh->createSharedVertexData();
	handleVertexBuffer(_collada_mesh, _mesh->sharedVertexData);

	std::clog << "Mesh with " << _mesh->sharedVertexData->getNVertices() << " vertices created." << std::endl;

	/// Submeshes
	COLLADAFW::MeshPrimitiveArray const &submesh_array = _collada_mesh->getMeshPrimitives();
	std::clog << "Creating " << submesh_array.getCount() << " submeshes." << std::endl;
	for(size_t i = 0; i < submesh_array.getCount(); ++i)
	{
		// @todo this world matrix business I don't understand.
		COLLADABU::Math::Matrix4 worldMatrix;
		COLLADAFW::MeshPrimitive* meshPrimitive = submesh_array[i];
		vl::SubMesh *submesh = _mesh->createSubMesh();
		handleSubMesh(meshPrimitive, worldMatrix, submesh);
	}

	/// Why the need to use instance geometry?
	/// Couldn't we just read the mesh data from the COLLADAFW::Mesh structure?
	/*
	const COLLADAFW::UniqueId& meshUniqueId = mMesh->getUniqueId();
	OgreWriter::GeometryUniqueIdInstanceGeometryInfoMap const &instanceGeometryMap = getGeometryUniqueIdInstanceGeometryInfoMap();
	FileDeserializer::GeometryUniqueIdInstanceGeometryInfoMap::const_iterator lowerBound = instanceGeometryMap.lower_bound(meshUniqueId);
	FileDeserializer::GeometryUniqueIdInstanceGeometryInfoMap::const_iterator upperBound = instanceGeometryMap.upper_bound(meshUniqueId);

	for(FileDeserializer::GeometryUniqueIdInstanceGeometryInfoMap::const_iterator it = lowerBound; it != upperBound; ++it)
	{
		FileDeserializer::InstanceGeometryInfo const &instanceGeometryInfo = it->second;
		COLLADABU::Math::Matrix4 const &worldMatrix = instanceGeometryInfo.worldMatrix;
		COLLADAFW::MeshPrimitiveArray const &meshPrimitives = _collada_mesh->getMeshPrimitives();
		COLLADAFW::InstanceGeometry const *instanceGeometry = instanceGeometryInfo.fwInstanceGeometry;

		worldMatrix.extract3x3Matrix(mCurrentRotationMatrix);
		mCurrentTranslationVector.set(worldMatrix[0][3], worldMatrix[1][3], worldMatrix[2][3]);

		for(size_t i = 0, count = meshPrimitives.getCount(); i < count; ++i)
		{
			COLLADAFW::MeshPrimitive* meshPrimitive = meshPrimitives[i];
			writeSubMesh(meshPrimitive, worldMatrix);
		}
	}
	*/

	_mesh->calculateBounds();

	std::clog << "Mesh bounds = " << _mesh->getBounds() << std::endl;

	return true;
}

void
vl::dae::MeshImporter::handleIndexBuffer(COLLADAFW::MeshPrimitive* meshPrimitive, 
	COLLADABU::Math::Matrix4 const &matrix, vl::IndexBuffer *ibf)
{
	std::clog << "vl::dae::MeshImporter::handleIndexBuffer" << std::endl;

	assert(ibf);

	int numIndices = 0;

	bool has_normals = false;
	bool has_uv_coords = false;

	const COLLADAFW::UIntValuesArray& positionIndices =  meshPrimitive->getPositionIndices();
	size_t positionIndicesCount  = positionIndices.getCount();

	COLLADAFW::UIntValuesArray const &normalIndices =  meshPrimitive->getNormalIndices();
	size_t normalIndicesCount = normalIndices.getCount();
	has_normals = (normalIndicesCount != 0);


	COLLADAFW::UIntValuesArray const *uvIndices;
	size_t uvIndicesCount = 0;
	// Multiple uv coordinates
	COLLADAFW::IndexListArray const &uVIndicesList = meshPrimitive->getUVCoordIndicesArray();
	if ( !uVIndicesList.empty() )
	{
		uvIndices = &uVIndicesList[0]->getIndices();
		uvIndicesCount = uvIndices->getCount();
		has_uv_coords = (uvIndicesCount != 0);
	}
	else
	{
		has_uv_coords = false;
	}

	if(has_uv_coords)
	{
		std::clog << "Submesh has face uvs : NOT SUPPORTED" << std::endl;
		std::clog << uvIndicesCount << " uv indices." << std::endl;
	}

	if(has_normals)
	{
		std::clog << "Submesh has face normals : NOT SUPPORTED" << std::endl;
		std::clog << normalIndices.getCount() << " face normals." << std::endl;
	}

	assert( !has_normals || (positionIndicesCount == normalIndicesCount));
	assert( !has_uv_coords || (positionIndicesCount == uvIndicesCount));

	std::clog << positionIndicesCount << " indeces." << std::endl;

	// Should calculate the necessary size of the index buffer
	// select either 16bit or 32bit buffer and then resize the buffer
	ibf->setIndexSize(vl::IT_32BIT);

	// Do not resize the index buffer here
	// because we don't have the exact size before handling the type of indeces 
	// and splitting all polygons to triangles.
	size_t index_buffer_size = _calculate_index_buffer_size(meshPrimitive);

	std::clog << "Index buffer size = " << index_buffer_size << std::endl;

	ibf->setIndexCount(index_buffer_size);

	// Custom arrays for triangle normal and uv indices
	std::vector<uint32_t> tri_uv_indices(index_buffer_size);
	std::vector<uint32_t> tri_normal_indices(index_buffer_size);

	switch (meshPrimitive->getPrimitiveType())
	{
	case COLLADAFW::MeshPrimitive::LINES:
		std::clog << "Unhandled geometry primitive of type LINES." << std::endl;
		break;
	case COLLADAFW::MeshPrimitive::LINE_STRIPS:
		std::clog << "Unhandled geometry primitive of type LINE_STRIPS." << std::endl;
		break;
	case COLLADAFW::MeshPrimitive::POLYLIST:
		std::clog << "Unhandled geometry primitive of type POLYLIST." << std::endl;
		break;

	case COLLADAFW::MeshPrimitive::POLYGONS:
		std::clog << "Geometry primitive of type POLYGONS" << std::endl;
		{
			const COLLADAFW::Polygons *polygons = (const COLLADAFW::Polygons*) meshPrimitive;

			std::clog << "Mesh with " << polygons->getFaceCount() << " faces." << std::endl;
			/// Handle quads (four vertices per face)
			size_t index = 0;
			size_t ibf_index = 0;
			for(size_t f = 0; f < polygons->getFaceCount(); ++f)
			{
				size_t const vert_count = polygons->getGroupedVerticesVertexCount(f);

				// Ignores UV and normal indeces
				// how to handle face normals and face UVs?

				// Split quads
				if(vert_count == 4)
				{
					/// @todo copy working version for quads to SubMesh::addFace
					// @todo move to using shorthand ibf_index++
					ibf->set(ibf_index, positionIndices[index]);
					ibf->set(ibf_index+1, positionIndices[index+1]);
					ibf->set(ibf_index+2, positionIndices[index+2]);
					ibf->set(ibf_index+3, positionIndices[index]);
					ibf->set(ibf_index+4, positionIndices[index+2]);
					ibf->set(ibf_index+5, positionIndices[index+3]);

					if(has_normals)
					{
						tri_normal_indices.at(ibf_index) = normalIndices[index];
						tri_normal_indices.at(ibf_index+1) = normalIndices[index+1];
						tri_normal_indices.at(ibf_index+2) = normalIndices[index+2];
						tri_normal_indices.at(ibf_index+3) = normalIndices[index];
						tri_normal_indices.at(ibf_index+4) = normalIndices[index+2];
						tri_normal_indices.at(ibf_index+5) = normalIndices[index+3];
					}
					if(has_uv_coords)
					{
						tri_uv_indices.at(ibf_index) = (*uvIndices)[index];
						tri_uv_indices.at(ibf_index+1) = (*uvIndices)[index+1];
						tri_uv_indices.at(ibf_index+2) = (*uvIndices)[index+2];
						tri_uv_indices.at(ibf_index+3) = (*uvIndices)[index];
						tri_uv_indices.at(ibf_index+4) = (*uvIndices)[index+2];
						tri_uv_indices.at(ibf_index+5) = (*uvIndices)[index+3];
					}

					ibf_index += 6;
				}
				else if(vert_count == 3)
				{
					ibf->set(ibf_index, positionIndices[index]);
					ibf->set(ibf_index+1, positionIndices[index+1]);
					ibf->set(ibf_index+2, positionIndices[index+2]);
			
					if(has_normals)
					{
						tri_normal_indices.at(ibf_index) = normalIndices[index];
						tri_normal_indices.at(ibf_index+1) = normalIndices[index+1];
						tri_normal_indices.at(ibf_index+2) = normalIndices[index+2];
					}

					if(has_uv_coords)
					{
						tri_uv_indices.at(ibf_index) = (*uvIndices)[index];
						tri_uv_indices.at(ibf_index+1) = (*uvIndices)[index+1];
						tri_uv_indices.at(ibf_index+2) = (*uvIndices)[index+2];
					}

					ibf_index += 3;
				}
				else
				{
					std::stringstream ss;
					ss << "Unknown vertex count for face : count = " << vert_count;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
				}

				index += vert_count;
			}
		}
		break;
	case COLLADAFW::MeshPrimitive::TRIANGLES:
		{
			const COLLADAFW::Triangles* triangles = (const COLLADAFW::Triangles*) meshPrimitive;

			for(int j = 0; j < index_buffer_size; ++j)
			{
				ibf->set(j, positionIndices[j]);
				//unsigned int normalIndex = 0;
				if(has_normals)
					tri_normal_indices.at(j) = normalIndices[j];

				//unsigned int uvIndex = 0;
				if(has_uv_coords)
					tri_uv_indices.at(j) = (*uvIndices)[j];
			}
		}
		break;
	case COLLADAFW::MeshPrimitive::TRIANGLE_FANS:
		std::clog << "Unhandled geometry primitive of type TRIANGLE_FANS." << std::endl;
		break;
	case COLLADAFW::MeshPrimitive::TRIANGLE_STRIPS:
		std::clog << "Unhandled geometry primitive of type TRIANGLE_STRIPS." << std::endl;
		break;
	case COLLADAFW::MeshPrimitive::POINTS:
		std::clog << "Unhandled geometry primitive of type POITS." << std::endl;
		break;
	default:
		std::clog << "Unhandled geometry primitive type : " 
			<< meshPrimitive->getPrimitiveType() << std::endl;
		break;
	}

	/// @todo this will probably cause problems for any Mesh that needed
	/// modifications to the IndexBuffer for example splitting quads
	/// because the length of the ibf is no longer the same as normalIndices...
	if(has_normals)
	{
		_calculate_vertex_normals(tri_normal_indices, ibf);
	}

	if(has_uv_coords)
	{
		_calculate_vertex_uvs(tri_uv_indices, ibf);
	}
}

//------------------------------
void
vl::dae::MeshImporter::handleSubMesh(COLLADAFW::MeshPrimitive* meshPrimitive, 
	const COLLADABU::Math::Matrix4& matrix, vl::SubMesh *submesh)
{
	std::clog << "vl::dae::MeshImporter::handleSubMesh" << std::endl;
	assert(submesh);
	assert(meshPrimitive);

//	std::clog << "Submesh material = " << meshPrimitive->getMaterial() << std::endl;
//	submesh->setMaterial(meshPrimitive->getMaterialId());
	_submeshes.push_back(std::make_pair(submesh, meshPrimitive->getMaterialId()));
	handleIndexBuffer(meshPrimitive, matrix, &submesh->indexData);
}

//-----------------------------------------------------------------------
void
vl::dae::MeshImporter::handleVertexBuffer(COLLADAFW::Mesh const *mesh, vl::VertexData *vbf)
{
	std::clog << "vl::dae::MeshImporter::handleVertexBuffer" << std::endl;
	assert(vbf);

	/// Correct semantics
	/// Not used yet but later on these will be crucial
	vbf->vertexDeclaration.addSemantic(Ogre::VES_POSITION, Ogre::VET_FLOAT3);
	if(mesh->hasNormals())
	{
		std::clog << "Mesh has normals." << std::endl;
		vbf->vertexDeclaration.addSemantic(Ogre::VES_NORMAL, Ogre::VET_FLOAT3);
	}
	COLLADAFW::MeshVertexData const &colours = mesh->getColors();
	COLLADAFW::MeshVertexData const &uvs = mesh->getUVCoords();
	bool has_uvs = (uvs.getValuesCount() != 0);
	if(has_uvs)
	{
		vbf->vertexDeclaration.addSemantic(Ogre::VES_TEXTURE_COORDINATES, Ogre::VET_FLOAT2);
	}

	bool has_colours = (colours.getValuesCount() != 0);
	if(has_colours)
	{
		vbf->vertexDeclaration.addSemantic(Ogre::VES_DIFFUSE, Ogre::VET_COLOUR);
	}

	// @todo should resize the vertex buffer here
	// and then use array indexing when reading the other attributes
	COLLADAFW::MeshVertexData const &positions = mesh->getPositions();
	assert(positions.getValuesCount() % 3 == 0);
	size_t n_vertices = positions.getValuesCount()/3; 
	vbf->setNVertices(n_vertices);
	for(size_t i = 0; i < n_vertices; ++i)
	{
		vbf->getVertex(i).position = convert_vertex_data(positions, i);
	}

	//	TODO add support for normals
	// Collada uses real normals (aka face normals) so we don't process them here
	// but store them for use when faces are processed.
	if(mesh->hasNormals())
	{
		COLLADAFW::MeshVertexData const &normals = _collada_mesh->getNormals();
		// Will make it bit more difficult for us if some vertices have normals
		// and some don't. Doh.
		// this fails for some reason
		//assert(normals.getValuesCount() / 3 == n_vertices);
		size_t n_normals = normals.getValuesCount()/3;
		assert(normals.getValuesCount() % 3 == 0);

		// How do we do the mapping?
		_normals.resize(n_normals);
		for(size_t i = 0; i < n_normals; ++i)
		{
			_normals.at(i) = convert_vertex_data(normals, i);
		}
	}

	if(has_uvs)
	{
		/// What type of uvs are these? two double values?
		// The 2 dimensional uv coordinates array. 
        // UV coordinates can be stored as float or double values.
		// @todo are these vertex or face uvs?
		std::clog << "Mesh has uvs : NOT_IMPLEMENTED" << std::endl;
		size_t n_uvs = uvs.getValuesCount()/2;
		assert(uvs.getValuesCount() % 2 == 0); 

		_uvs.resize(n_uvs);
		for(size_t i = 0; i < n_uvs; ++i)
		{
			_uvs.at(i) = convert_uvs(uvs, i);
		}
	}

	if(has_colours)
	{
		// What type of a colour is this? three or four floats?
		// seems like 3 floats/doubles but seems like OpenCollada does not store
		// this information here. Question where is it stored then?
		std::clog << "Mesh has vertex colours : NOT_IMPLEMENTED" << std::endl;
	}
}

struct VertexNormal
{
	// uses huge amounts of memory but should never be necessary to resize
	VertexNormal(void)
	{
		normals.reserve(32);
	}

	Ogre::Vector3 avarage(void) const
	{
		if(normals.size() == 0)
		{ return Ogre::Vector3::ZERO; }

		return sum()/normals.size();
	}

	Ogre::Vector3 sum(void) const
	{
		Ogre::Vector3 s = Ogre::Vector3::ZERO;
		for(size_t i = 0; i < normals.size(); ++i)
		{
			s += normals.at(i);
		}

		return s;
	}

	std::vector<Ogre::Vector3> normals;
};

void
vl::dae::MeshImporter::_calculate_vertex_normals(std::vector<uint32_t> const &normalIndices, vl::IndexBuffer *ibf)
{
	std::clog << "vl::dae::MeshImporter::_calculate_vertex_normals" << std::endl;

	// we have a map from faces -> normals (normalIndices)
	// we have a map from vertices -> faces (ibf)
	// we need a map from vertices -> normals
	std::vector<VertexNormal> vert_to_normal;
	vert_to_normal.resize(_mesh->sharedVertexData->getNVertices());

	for(size_t i = 0; i < _mesh->sharedVertexData->getNVertices(); ++i)
	{
		for(size_t j = 0; j < ibf->indexCount(); ++j)
		{
			// Find the face id
			// @todo this should be done before by using a temporary array
			if(ibf->getVec32().at(j) == i)
			{
				Ogre::Vector3 normal = _normals.at(normalIndices.at(j));
				vert_to_normal.at(i).normals.push_back(normal);
			}
		}
	}

	// if smooth shading is on
	// the vertex normal is the avarage of all the face normals
	// @todo flat shading is not implemented
//	if(!_flat_shading)
	{
		for(size_t i = 0; i < vert_to_normal.size(); ++i)
		{
			VertexNormal const &vn = vert_to_normal.at(i);
			Vertex &vertex = _mesh->sharedVertexData->getVertex(i);
			vertex.normal = vn.avarage();
			// just to check
			if(vertex.normal == Ogre::Vector3::ZERO)
			{
				std::clog << "Something very funny we have a zero length normal vector." << std::endl;
			}
		}
	}
	// if flat shading is on
	// we need to create new vertices when fn1 != fn2 or fn1 != fn3 or fn2 != fn3
	// so that every normal that is different has a dedicated vertex
	// this will need to update vbf (adding new vertices) 
	// and also need to update ibf to match the new vertices
//	else
	{
		// first we need to create new map which has unique
	}

}

void
vl::dae::MeshImporter::_calculate_vertex_uvs(std::vector<uint32_t> const &uvIndices, vl::IndexBuffer *ibf)
{
	std::clog << "vl::dae::MeshImporter::_calculate_vertex_uvs : NOT_IMPLEMENTED" << std::endl;
}

size_t
vl::dae::MeshImporter::_calculate_index_buffer_size(COLLADAFW::MeshPrimitive *meshPrimitive) const
{
	switch(meshPrimitive->getPrimitiveType())
	{
		case COLLADAFW::MeshPrimitive::POLYGONS:
		{
			const COLLADAFW::Polygons *polygons = (const COLLADAFW::Polygons*) meshPrimitive;
			size_t size = 0;
			for(size_t f = 0; f < polygons->getFaceCount(); ++f)
			{
				size_t vertices = polygons->getGroupedVerticesVertexCount(f);
				if(vertices == 3)
				{ size += 3; }
				else if(vertices == 4)
				{ size += 6; }
				else
				{ BOOST_THROW_EXCEPTION(vl::exception()); }
			}
			return size;
		}
		case COLLADAFW::MeshPrimitive::TRIANGLES:
		{
			return meshPrimitive->getFaceCount()*3;
		}

		// Not supported types
		case COLLADAFW::MeshPrimitive::LINES:
		case COLLADAFW::MeshPrimitive::LINE_STRIPS:
		case COLLADAFW::MeshPrimitive::POLYLIST:
		case COLLADAFW::MeshPrimitive::TRIANGLE_FANS:
		case COLLADAFW::MeshPrimitive::TRIANGLE_STRIPS:
		case COLLADAFW::MeshPrimitive::POINTS:
		default:
			return 0;
	}
}
