/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_mesh_importer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
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

#include "math/math.hpp"

namespace {

void convert_vertex_data(vl::VertexBufferRefPtr vbf, COLLADAFW::MeshVertexData const &data, size_t offset)
{
	size_t vbf_i = offset;
	for(size_t i = 0; i < vbf->getNVertices(); ++i)
	{
		assert(data.getValuesCount() <= vbf->getNVertices()*3);
		if(data.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
		{
			const double* arr = data.getDoubleValues()->getData();
			arr += 3*i;
			vbf->write(vbf_i, (Ogre::Real)arr[0]);
			vbf->write(vbf_i+1*sizeof(Ogre::Real), (Ogre::Real)arr[1]);
			vbf->write(vbf_i+2*sizeof(Ogre::Real), (Ogre::Real)arr[2]);
		}
		else
		{
			const float* arr = data.getFloatValues()->getData();
			arr += 3*i;
			vbf->write(vbf_i, (Ogre::Real)arr[0]);
			vbf->write(vbf_i+1*sizeof(Ogre::Real), (Ogre::Real)arr[1]);
			vbf->write(vbf_i+2*sizeof(Ogre::Real), (Ogre::Real)arr[2]);
		}
		vbf_i += vbf->getVertexSize();
	}
}

Ogre::Vector3 convert_vertex_data(COLLADAFW::MeshVertexData const &data, size_t index)
{
	Ogre::Vector3 v;
	if(data.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
	{
		const double* arr = data.getDoubleValues()->getData();
		arr += 3*index;
		v.x = (Ogre::Real)arr[0];
		v.y = (Ogre::Real)arr[1];
		v.z = (Ogre::Real)arr[2];
	}
	else
	{
		const float* arr = data.getFloatValues()->getData();
		arr += 3*index;
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
vl::dae::MeshImporter::MeshImporter(MeshManagerRefPtr mesh_manager)
	: _mesh_manager(mesh_manager)
{}

//------------------------------
vl::dae::MeshImporter::~MeshImporter()
{
}


//------------------------------
bool
vl::dae::MeshImporter::read(COLLADAFW::Mesh const *mesh, ImporterSettings const &settings)
{
	std::clog << "vl::dae::MeshImporter::read" << std::endl;

//	bool skelAnim = _collada_mesh->hasSkeleton();

	assert(_mesh_manager);
	assert(mesh);
	_mesh = _mesh_manager->createMesh(mesh->getName());

	_mesh->createSharedVertexData();
	handleVertexBuffer(mesh, _mesh->sharedVertexData);

	std::clog << "Mesh with " << _mesh->sharedVertexData->getVertexCount() << " vertices created." << std::endl;

	/// Submeshes
	COLLADAFW::MeshPrimitiveArray const &submesh_array = mesh->getMeshPrimitives();
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
		COLLADAFW::MeshPrimitiveArray const &meshPrimitives = mesh->getMeshPrimitives();
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

	/// @todo clean up all the stored values so we can be run again

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

	/* We handle uvs and normals later after creating index lists
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
	*/

	assert( !has_normals || (positionIndicesCount == normalIndicesCount));
	assert( !has_uv_coords || (positionIndicesCount == uvIndicesCount));

	std::clog << positionIndicesCount << " indeces." << std::endl;

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

				if(has_normals)
					tri_normal_indices.at(j) = normalIndices[j];

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
vl::dae::MeshImporter::handleVertexBuffer(COLLADAFW::Mesh const *mesh, vl::VertexData *vdata)
{
	std::clog << "vl::dae::MeshImporter::handleVertexBuffer" << std::endl;
	assert(vdata);

	/// Correct semantics
	size_t pos_offset, n_offset, uv_offset, col_offset;
	pos_offset = 0;
	vdata->vertexDeclaration.addElement(0, pos_offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	n_offset = vdata->vertexDeclaration.getTypeSize(Ogre::VET_FLOAT3);

	if(mesh->hasNormals())
	{
		vdata->vertexDeclaration.addElement(0, n_offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
		uv_offset = pos_offset + vdata->vertexDeclaration.getTypeSize(Ogre::VET_FLOAT3);
	}
	

	COLLADAFW::MeshVertexData const &colours = mesh->getColors();
	COLLADAFW::MeshVertexData const &uvs = mesh->getUVCoords();
	bool has_uvs = (uvs.getValuesCount() != 0);
	if(has_uvs)
	{
		vdata->vertexDeclaration.addElement(0, uv_offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
		col_offset = uv_offset + vdata->vertexDeclaration.getTypeSize(Ogre::VET_FLOAT2);
	}

	bool has_colours = (colours.getValuesCount() != 0);
	if(has_colours)
	{
		vdata->vertexDeclaration.addElement(0, col_offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
		col_offset += vdata->vertexDeclaration.getTypeSize(Ogre::VET_FLOAT2);
	}

	// @todo should resize the vertex buffer here
	// and then use array indexing when reading the other attributes
	COLLADAFW::MeshVertexData const &positions = mesh->getPositions();
	assert(positions.getValuesCount() % 3 == 0);
	size_t n_vertices = positions.getValuesCount()/3; 

	VertexBufferRefPtr vbuf = VertexBuffer::create(vdata->vertexDeclaration.getVertexSize(), n_vertices);
	vdata->setBinding(0, vbuf);
	convert_vertex_data(vbuf, positions, pos_offset);

	// Collada uses real normals (aka face normals) so we don't process them here
	// but store them for use when faces are processed.
	// todo
	// figure out what these normals actually are and how they relate to the vertices
	// at the moment it's bit iffy
	if(mesh->hasNormals())
	{
		COLLADAFW::MeshVertexData const &normals = mesh->getNormals();
		// Will make it bit more difficult for us if some vertices have normals
		// and some don't. Doh.
		// this fails for some reason
		//assert(normals.getValuesCount() / 3 == n_vertices);
		// These are not vertex normals though, they are face normals of course
		// because vertex normals are a rasterization helper not a real thing.
		size_t n_normals = normals.getValuesCount()/3;
		assert(normals.getValuesCount() % 3 == 0);

		// How do we do the mapping?
		_normals.resize(n_normals);
		for(size_t i = 0; i < n_normals; ++i)
		{
			_normals.at(i) = convert_vertex_data(normals, i);
		}

		std::clog << "Mesh has " << vbuf->getNVertices() 
			<< " vertices and " << n_normals << " normals." << std::endl;
	}

	if(has_uvs)
	{
		std::clog << "Mesh has UVs." << std::endl;
		/// What type of uvs are these? two double values?
		// The 2 dimensional uv coordinates array. 
        // UV coordinates can be stored as float or double values.
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

	size_t size() const
	{ return normals.size(); }

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
	vert_to_normal.resize(_mesh->sharedVertexData->getVertexCount());

	// @todo can we reduce the complexity of this function?
	// it will take a significant amount of time considering it iterates
	// twice over the indexes
	for(size_t i = 0; i < _mesh->sharedVertexData->getVertexCount(); ++i)
	{
		for(size_t j = 0; j < ibf->indexCount(); ++j)
		{
			// Find the face id
			// @todo this should be done before by using a temporary array
			if(ibf->at(j) == i)
			{
				Ogre::Vector3 const &normal = _normals.at(normalIndices.at(j));
				// @todo we should not be using push_back but resize and at
				vert_to_normal.at(i).normals.push_back(normal);
			}
		}
	}

	// if smooth shading is on
	// the vertex normal is the avarage of all the face normals
	// @todo flat shading is not implemented
	// because flat shading needs vertices to be added
	// we should create this mesh first and then use an operation
	// that creates a flat shaded one.
	
	// trying to figure out if the normal array is same size as vertices array
	std::clog << "Copy vertex normals. Mesh has "
		<< _mesh->sharedVertexData->getVertexCount() << " vertices and "
		<< vert_to_normal.size() << " normals." << std::endl;
	// just do some checking here if this is true all the time
	// we can remove some dynamic array resizing from code above
	assert(vert_to_normal.size() == _mesh->sharedVertexData->getVertexCount());

	size_t index = 0;
	for(NormalIterator iter = _mesh->sharedVertexData->getNormalIterator(); !iter.end(); ++iter)
	{
		// @todo can we assume that the normal array is the same size as vertex array?
		// it should be by design unless there are vertices without normals which is really
		// weird.
		// Well seems like we have four times as many normals than vertices because normals
		// are mapped to indices instead of vertices.
		assert(index < vert_to_normal.size());
		
		VertexNormal const &vn = vert_to_normal.at(index);
		if(vn.size() > 0)
		{
			*iter = vn.avarage();
			if(*iter == Ogre::Vector3::ZERO)
			{
				std::clog << "Something very funny we have a zero length normal vector." << std::endl;
			}
		}
		else
		{
			// some meshes in Firebird model end here for some reason
			// why?
			//std::clog << "We don't have a proper normal for some reason." << std::endl;
		}
		++index;
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

/// @brief adds a copy of vertex and remaps the face in index buffer 
/// modifies both index and vertex buffer
/// does not add any new idices to index buffer, only modifies existing ones
/// inefficent way to split vertices and does not take into account possibility
/// to remap multiple faces to a single new vertex
/// Only works on 32bit indices.
/// @return vertex index which we added
/* Removed for the moment
size_t split_vertex(vl::IndexBuffer *ibf, vl::VertexData *vbf, size_t face_index, size_t vertex_index)
{
	assert(ibf->indexCount() > face_index);
	assert(vbf->getNVertices() > vertex_index);

	vl::Vertex const &vertex = vbf->getVertex(vertex_index);
	vbf->addVertex(vertex);
	// last element in vertex buffer
	std::clog << "Changing face index : " << face_index << " from vertex " 
		<< ibf->getVec32()[face_index] << " to " << vbf->getNVertices()-1 << std::endl;
	size_t vert_idx = vbf->getNVertices()-1;
	ibf->getVec32()[face_index] = vert_idx;

	return vert_idx;
}

struct VertexUV
{
	// uses huge amounts of memory but should never be necessary to resize
	VertexUV(void)
		: vertex_index(0)
	{
		uvs.reserve(32);
	}

	void add(size_t face_index, Ogre::Vector2 const &uv, size_t v_index)
	{
		bool unique = true;
		// This opmisation breaks the splitting for some reason
		// now we split all vertices so we get all uvs correct
		//for(size_t i = 0; i < uvs.size(); ++i)
		//{
		//	// @todo add tolerance
		//	if(vl::equal(uv, uvs.at(i).second))
		//	{ unique = false; break; }
		//}

		if(unique)
		{ uvs.push_back(std::make_pair(face_index, uv)); }

		if(vertex_index != 0)
		{ assert(vertex_index == v_index); }
		vertex_index = v_index;
	}

	bool needs_splitting(void) const
	{ return uvs.size() > 1; }

	// Return array of splitted vertices
	// inefficent we should pass a pre allocated array and index (or a pointer to memory)
	void split(vl::IndexBuffer *ibf, vl::VertexData *vbf, size_t v_index, std::vector<VertexUV> &new_vert_uvs)
	{
		if(!needs_splitting())
		{ return; }
		
		// skip the first uv set, as it remains valid
		std::clog << "Splitting vertex " << uvs.size()-1 << " times." << std::endl;
		for(size_t i = 1; i < uvs.size(); ++i)
		{
			assert(vertex_index == v_index);
			size_t face_idx = uvs.at(i).first;
			size_t vert_idx = split_vertex(ibf, vbf, face_idx, v_index);
			new_vert_uvs.push_back(VertexUV());
			
			new_vert_uvs.back().add(face_idx, uvs.at(i).second, vert_idx);
		}

		// Clean this structure to reflect the splitting
		uvs.erase(uvs.begin()+1, uvs.end());
	}

	// face index uv pair array
	std::vector<std::pair<size_t, Ogre::Vector2> > uvs;
	size_t vertex_index;
};

std::ostream &operator<<(std::ostream &os, VertexUV const &uv)
{
	os << uv.uvs.size() << " vertex uvs : ";
	for(size_t i = 0; i < uv.uvs.size(); ++i)
	{
		os << uv.uvs.at(i).second << " , ";
	}

	return os;
}
*/


void
vl::dae::MeshImporter::_calculate_vertex_uvs(std::vector<uint32_t> const &uvIndices, vl::IndexBuffer *ibf)
{
	std::clog << "vl::dae::MeshImporter::_calculate_vertex_uvs" << std::endl;

	assert(!_uvs.empty());

	/* Removed for the moment
	std::clog << "uv array size = " << _uvs.size() << " uv indices size = " << uvIndices.size() << std::endl;
	/// For now just assume that every vertex has an unique uv coordinate
	/// later we need to implement vertex splitting
	/// but we need to coordinate with vertex normal generation for this
	/// as both of these will split vertices based on different parameters

	// we have a map from faces -> uvs (uvIndices)
	// we have a map from vertices -> faces (ibf)
	// we need a map from vertices -> uvs
	VertexData *vbf = _mesh->sharedVertexData;
	std::vector<VertexUV> vert_to_uv;
	vert_to_uv.resize(vbf->getNVertices());

	for(size_t j = 0; j < ibf->indexCount(); ++j)
	{
		// Find the face id
		// @todo this should be done before by using a temporary array
		size_t vert_index = ibf->getVec32().at(j);
		assert(vert_index < vbf->getNVertices());

		Ogre::Vector2 const &uv = _uvs.at(uvIndices.at(j));
		std::clog << "Vetex : " << vbf->getVertex(vert_index).position 
			<< " uv = " << uv << " face index : " << j/3 << std::endl;
		vert_to_uv.at(vert_index).add(j, uv, vert_index);
	}

	std::vector<VertexUV> new_vert_uvs;
	// Splitting phase
	for(size_t i = 0; i < vert_to_uv.size(); ++i)
	{
		assert(i == vert_to_uv.at(i).vertex_index);
		VertexUV &vertex_uv = vert_to_uv.at(i);
		if(vertex_uv.needs_splitting())
		{
			vertex_uv.split(ibf, vbf, i, new_vert_uvs);
		}
	}

	// Combine the arrays
	for(size_t i = 0; i < new_vert_uvs.size(); ++i)
	{
		vert_to_uv.push_back(new_vert_uvs.at(i));
	}

	assert(vert_to_uv.size() == vbf->getNVertices());

	// Debug print the mesh structure
	std::clog << "Mesh vertices : " << std::endl;
	for(size_t i = 0; i < vbf->getNVertices(); ++i)
	{
		std::clog << "\t" << vbf->getVertex(i) << std::endl;
	}

	std::clog << "Mesh indices : " << std::endl;
	for(size_t i = 0; i < ibf->indexCount(); ++i)
	{
		std::clog << "\t" << ibf->getVec32().at(i) << std::endl;
	}

	// Calculate the real uvs
	// this really does nothing except copying the first ones and then warning for all the others
	for(size_t i = 0; i < vert_to_uv.size(); ++i)
	{
		size_t vertex_index = vert_to_uv.at(i).vertex_index;
		assert(vertex_index < vert_to_uv.size());
		//assert(i == vert_to_uv.at(i).vertex_index);
		VertexUV const &vertex_uv = vert_to_uv.at(i);
		Vertex &vertex = vbf->getVertex(vertex_index);
		if(vertex_uv.uvs.size() > 1)
		{
			std::clog << "Warning : vertex has " << vertex_uv.uvs.size() 
				<< " uvs of which only 1 is supported." << std::endl;
			std::clog << "All uvs : " << vertex_uv << std::endl;
			// now we should duplicate the vertex
			// we can assume that all attributes are already set
			// because this is the last step
			// this does not work for normals though
			// also this might mess the face indices
		}
		if(vertex_uv.uvs.size() > 0)
		{
			assert(ibf->getVec32().at(vertex_uv.uvs.at(0).first) == vertex_index);
			vertex.uv = vertex_uv.uvs.at(0).second;
			std::clog << "Copying uvs for vertex at " << vertex_index 
				<< " : uvs = " << vertex.uv.x <<  ", " << vertex.uv.y << std::endl;
		}
		else
		{ std::clog << "Warning : vertex has no uvs." << std::endl; }
	}
	*/
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
