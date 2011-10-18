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

std::string const MATERIAL("BaseWhite");

//------------------------------
vl::dae::MeshImporter::MeshImporter(MeshManagerRefPtr mesh_manager, COLLADAFW::Mesh const *mesh)
	: _collada_mesh(mesh)
	, _mesh_manager(mesh_manager)
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
	vl::MeshRefPtr mesh = _mesh_manager->createMesh(_collada_mesh->getName());

	mesh->createSharedVertexData();
	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_POSITION, Ogre::VET_FLOAT3);
	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_DIFFUSE, Ogre::VET_FLOAT3);
	
	/// Shared vertex data
	COLLADAFW::MeshVertexData const &positions = _collada_mesh->getPositions();
	for(size_t i = 0; i < positions.getValuesCount()/3; ++i)
	{
		Vertex vert;
		Ogre::Vector3 pos;
		if( positions.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
		{
			const double* positionsArray = positions.getDoubleValues()->getData();
			positionsArray += 3*i;
			COLLADABU::Math::Vector3 position(positionsArray[0], positionsArray[1], positionsArray[2]);
			//position = mCurrentRotationMatrix * position + mCurrentTranslationVector;
			pos.x = (Ogre::Real)position.x;
			pos.y = (Ogre::Real)position.y;
			pos.z = (Ogre::Real)position.z;
		}
		else
		{
			const float* positionsArray = positions.getFloatValues()->getData();
			positionsArray += 3*i;
			COLLADABU::Math::Vector3 position(positionsArray[0], positionsArray[1], positionsArray[2]);
			//position = mCurrentRotationMatrix * position + mCurrentTranslationVector;
			pos.x = (Ogre::Real)position.x;
			pos.y = (Ogre::Real)position.y;
			pos.z = (Ogre::Real)position.z;
		}
		
		vert.position = pos;
		std::clog << "Position = " << pos << std::endl;

		/// This needs to add UV coords before the mesh is usable
		mesh->sharedVertexData->addVertex(vert);
	}

	std::clog << "Mesh with " << mesh->sharedVertexData->getNVertices() << " vertices created." << std::endl;

	/*	TODO add support for normals
	if(_collada_mesh->hasNormals())
	{
		mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_NORMAL, Ogre::VET_FLOAT3);
		COLLADAFW::MeshVertexData const &normals = _collada_mesh->getNormals();
	}
	
	COLLADAFW::MeshVertexData const &colours = _collada_mesh->getColors();
	*/

	/// Submeshes
	COLLADAFW::MeshPrimitiveArray const &submesh_array = _collada_mesh->getMeshPrimitives();
	std::clog << "Creating " << submesh_array.getCount() << " submeshes." << std::endl;
	for(size_t i = 0; i < submesh_array.getCount(); ++i)
	{
		// @todo this world matrix business I don't understand.
		COLLADABU::Math::Matrix4 worldMatrix;
		COLLADAFW::MeshPrimitive* meshPrimitive = submesh_array[i];
		vl::SubMesh *submesh = mesh->createSubMesh();
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

	mesh->calculateBounds();

	std::clog << "Mesh bounds = " << mesh->getBounds() << std::endl;

	return true;
}

//------------------------------
/*
void
vl::dae::MeshImporter::addTupleIndex(Tuple const &tuple)
{
	TupleIndexMap::const_iterator it = mTupleMap.find(tuple);
	if ( it == mTupleMap.end() )
	{
		mOgreIndices.append( mTupleMap[tuple] = mNextTupleIndex++ );

		if ( mMeshPositions.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
		{
			const double* positionsArray = mMeshPositions.getDoubleValues()->getData();
			positionsArray += 3*tuple.positionIndex;
			COLLADABU::Math::Vector3 position(positionsArray[0], positionsArray[1], positionsArray[2]);
			position = mCurrentRotationMatrix * position + mCurrentTranslationVector;
			mOgrPositions.append((float)position.x);
			mOgrPositions.append((float)position.y);
			mOgrPositions.append((float)position.z);
		}
		else
		{
			const float* positionsArray = mMeshPositions.getFloatValues()->getData();
			positionsArray += 3*tuple.positionIndex;
			COLLADABU::Math::Vector3 position(positionsArray[0], positionsArray[1], positionsArray[2]);
			position = mCurrentRotationMatrix * position + mCurrentTranslationVector;
			mOgrPositions.append((float)position.x);
			mOgrPositions.append((float)position.y);
			mOgrPositions.append((float)position.z);
		}

		if ( mHasNormals )
		{
			if ( mMeshNormals.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
			{
				const double* normalsArray = mMeshNormals.getDoubleValues()->getData();
				normalsArray += 3*tuple.normalIndex;
				COLLADABU::Math::Vector3 normal(normalsArray[0], normalsArray[1], normalsArray[2]);
				normal = mCurrentRotationMatrix * normal;
				normal.normalise();
				mOgreNormals.append((float)normal.x);
				mOgreNormals.append((float)normal.y);
				mOgreNormals.append((float)normal.z);
			}
			else
			{
				const float* normalsArray = mMeshNormals.getFloatValues()->getData();
				normalsArray += 3*tuple.normalIndex;
				COLLADABU::Math::Vector3 normal(normalsArray[0], normalsArray[1], normalsArray[2]);
				normal = mCurrentRotationMatrix * normal;
				normal.normalise();
				mOgreNormals.append((float)normal.x);
				mOgreNormals.append((float)normal.y);
				mOgreNormals.append((float)normal.z);
			}
		}

		if ( mHasUVCoords )
		{
			if ( mMeshUVCoordinates.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE )
			{
				const double* uVCoordinateArray = mMeshUVCoordinates.getDoubleValues()->getData();
				uVCoordinateArray += mMeshUVCoordinates.getStride(0)*tuple.textureIndex;
				mOgreUVCoordinates.append((float)uVCoordinateArray[0]);
				mOgreUVCoordinates.append((float)uVCoordinateArray[1]);
			}
			else
			{
				const float* uVCoordinateArray = mMeshUVCoordinates.getFloatValues()->getData();
				uVCoordinateArray += mMeshUVCoordinates.getStride(0)*tuple.textureIndex;
				mOgreUVCoordinates.append((float)uVCoordinateArray[0]);
				mOgreUVCoordinates.append((float)uVCoordinateArray[1]);
			}
		}

	}
	else
	{
		mOgreIndices.append(it->second);
	}
}
*/

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

	const COLLADAFW::UIntValuesArray& normalIndices =  meshPrimitive->getNormalIndices();
	size_t normalIndicesCount = normalIndices.getCount();
	has_normals = (normalIndicesCount != 0);


	const COLLADAFW::UIntValuesArray* uvIndices;
	size_t uvIndicesCount = 0;
	const COLLADAFW::IndexListArray& uVIndicesList = meshPrimitive->getUVCoordIndicesArray();
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


	assert( !has_normals || (positionIndicesCount == normalIndicesCount));
	assert( !has_uv_coords || (positionIndicesCount == uvIndicesCount));

	std::clog << positionIndicesCount << " indeces." << std::endl;

	// Should calculate the necessary size of the index buffer
	// select either 16bit or 32bit buffer and then resize the buffer
	ibf->setIndexSize(vl::IT_32BIT);

	// Do not resize the index buffer here
	// because we don't have the exact size before handling the type of indeces 
	// and splitting all polygons to triangles.

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
			for(size_t f = 0; f < polygons->getFaceCount(); ++f)
			{
				size_t const vert_count = polygons->getGroupedVerticesVertexCount(f);

				// Ignores UV and normal indeces
				// how to handle face normals and face UVs?

				// Split quads
				if(vert_count == 4)
				{
					/// @todo copy working version for quads to SubMesh::addFace
					ibf->push_back(positionIndices[index]);
					ibf->push_back(positionIndices[index+1]);
					ibf->push_back(positionIndices[index+2]);
					ibf->push_back(positionIndices[index]);
					ibf->push_back(positionIndices[index+2]);
					ibf->push_back(positionIndices[index+3]);
				}
				else if(vert_count == 3)
				{
					ibf->push_back(positionIndices[index]);
					ibf->push_back(positionIndices[index+1]);
					ibf->push_back(positionIndices[index+2]);
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
			numIndices = (int)positionIndicesCount;

			// All faces have three indices, we can copy them as is after resize
			ibf->setIndexCount(positionIndicesCount);

			for ( int j = 0; j < numIndices; ++j )
			{
				unsigned int positionIndex = positionIndices[j];

				unsigned int normalIndex = 0;
				if(has_normals)
					normalIndex = normalIndices[j];

				unsigned int uvIndex = 0;
				if(has_uv_coords)
					uvIndex = (*uvIndices)[j];

				ibf->set(j, positionIndex);
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
}

//------------------------------
void
vl::dae::MeshImporter::handleSubMesh(COLLADAFW::MeshPrimitive* meshPrimitive, 
	const COLLADABU::Math::Matrix4& matrix, vl::SubMesh *submesh)
{
	std::clog << "vl::dae::MeshImporter::handleSubMesh" << std::endl;
	assert(submesh);
	assert(meshPrimitive);

	std::clog << "Submesh material = " << meshPrimitive->getMaterial() << std::endl;
	submesh->setMaterial(meshPrimitive->getMaterial());
	handleIndexBuffer(meshPrimitive, matrix, &submesh->indexData);
}


//-----------------------------------------------------------------------
void
vl::dae::MeshImporter::handleVertexBuffer(vl::VertexData *vbf)
{
	std::clog << "vl::dae::MeshImporter::handleVertexBuffer" << std::endl;

	assert(vbf);
}

//-----------------------------------------------------------------------
void
vl::dae::MeshImporter::handleSubMeshOperation( COLLADAFW::MeshPrimitive::PrimitiveType primitiveType )
{
	/*
	// Header
	size_t csubMeshOperationSize = STREAM_OVERHEAD_SIZE + sizeof( uint16 );

	writeChunkHeader( Ogre::M_SUBMESH_OPERATION, csubMeshOperationSize );

	// unsigned short operationType
	unsigned short opType = 0;

	switch ( primitiveType )
	{
	case COLLADAFW::MeshPrimitive::TRIANGLES:
		opType = static_cast<unsigned short>( Ogre::OT_TRIANGLE_LIST );
		break;
	case COLLADAFW::MeshPrimitive::TRIANGLE_FANS:
		opType = static_cast<unsigned short>( Ogre::OT_TRIANGLE_FAN );
		break;
	case COLLADAFW::MeshPrimitive::TRIANGLE_STRIPS:
		opType = static_cast<unsigned short>( Ogre::OT_TRIANGLE_STRIP );
		break;
	}
	writeShorts( &opType, 1 );
	*/
}
