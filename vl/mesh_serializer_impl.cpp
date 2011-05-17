/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
/*	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	Modified significantly for Hydra
 *	Still covered by the original MIT license.
 */

/// Interface header
#include "mesh_serializer_impl.hpp"

/// File format definition
#include <OGRE/OgreMeshFileFormat.h>

/// Mesh structure
#include "mesh.hpp"

#include "base/exceptions.hpp"


#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// Disable conversion warnings, we do a lot of them, intentionally
#   pragma warning (disable : 4267)
#endif

/// stream overhead = ID + size
const long STREAM_OVERHEAD_SIZE = sizeof(uint16_t) + sizeof(uint32_t);

/// ----------------------- MeshSerializerImpl -------------------------------
vl::MeshSerializerImpl::MeshSerializerImpl()
{

    // Version number
    mVersion = "[MeshSerializer_v1.8]";
}
//---------------------------------------------------------------------
vl::MeshSerializerImpl::~MeshSerializerImpl()
{
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::exportMesh(vl::Mesh const *pMesh, vl::ResourceStream &stream, Endian endianMode)
{
	std::cout << "MeshSerializer writing mesh data to stream " + stream.getName() + "..." << std::endl;

	// Decide on endian mode
	determineEndianness(endianMode);

    // Check that the mesh has it's bounds set
    if (pMesh->getBounds().isNull() || pMesh->getBoundingSphereRadius() == 0.0f)
    {
		std::string msg("The Mesh you have supplied does not have its");
		msg += " bounds completely defined. Define them first before exporting.";
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
    }

    mStream = &stream;
	if(!stream.isWriteable())
	{
		std::string msg("Unable to use stream " + stream.getName() + " for writing");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
	}

    writeFileHeader();
	std::cout << "File header written." << std::endl;

    std::cout << "Writing mesh data..." << std::endl;
    writeMesh(pMesh);
    std::cout << "Mesh data exported." << std::endl;

    std::cout << "MeshSerializer export successful." << std::endl;
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::importMesh(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
//	std::clog << "vl::MeshSerializerImpl::importMesh : " << stream.getName() << std::endl;
	// Determine endianness (must be the first thing we do!)
	determineEndianness(stream);

    // Check header
    readFileHeader(stream);

    unsigned short streamID;
    while(!stream.eof())
    {
        streamID = readChunk(stream);
        switch (streamID)
        {
		case Ogre::M_MESH:
            readMesh(stream, pMesh);
            break;
		default :
			break;
		}

    }
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeMesh(vl::Mesh const *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::writeMesh" << std::endl;
    // Header
    writeChunkHeader(Ogre::M_MESH, calcMeshSize(pMesh));

	// bool skeletallyAnimated
	bool skelAnim = false; //pMesh->hasSkeleton();
	writeBools(&skelAnim, 1);

    // Write shared geometry
	if(pMesh->sharedVertexData && pMesh->sharedVertexData->getNVertices())
		writeGeometry(pMesh, pMesh->sharedVertexData);

    // Write Submeshes
    for (unsigned short i = 0; i < pMesh->getNumSubMeshes(); ++i)
    {
        std::cout << "Writing submesh..." << std::endl;
        writeSubMesh(pMesh->getSubMesh(i));
        std::cout << "Submesh exported." << std::endl;
    }

    // Write skeleton info if required
	/*	Skeletons are not supported
    if (pMesh->hasSkeleton())
    {
        LogManager::getSingleton().logMessage("Exporting skeleton link...");
        // Write skeleton link
        writeSkeletonLink(pMesh->getSkeletonName());
        LogManager::getSingleton().logMessage("Skeleton link exported.");

        // Write bone assignments
        if (!pMesh->mBoneAssignments.empty())
        {
            LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");

            Mesh::VertexBoneAssignmentList::const_iterator vi;
            for (vi = pMesh->mBoneAssignments.begin();
            vi != pMesh->mBoneAssignments.end(); ++vi)
            {
                writeMeshBoneAssignment(vi->second);
            }

            LogManager::getSingleton().logMessage("Shared geometry bone assignments exported.");
        }
    }

    // Write LOD data if any
	/*	LOD not supported
    if (pMesh->getNumLodLevels() > 1)
    {
        std::cout << "Exporting LOD information...." << std::endl;
        writeLodInfo(pMesh);
        std::cout << "LOD information exported." << std::endl;

    }
	*/
    // Write bounds information
    std::cout << "Exporting bounds information...." << std::endl;
    writeBoundsInfo(pMesh);
    std::cout << "Bounds information exported." << std::endl;

	// Write submesh name table
	std::cout << "Exporting submesh name table..." << std::endl;
	writeSubMeshNameTable(pMesh);
	std::cout << "Submesh name table exported." << std::endl;

	// Write edge lists
	/*	Not supported
	if (pMesh->isEdgeListBuilt())
	{
		std::cout << "Exporting edge lists..." << std::endl;;
		writeEdgeList(pMesh);
		std::cout << "Edge lists exported" << std::endl;
	}
	*/

	// Write morph animation
	/* Not supported
	writePoses(pMesh);
	if (pMesh->hasVertexAnimation())
	{
		writeAnimations(pMesh);
    }
	*/

    // Write submesh extremes
    writeExtremes(pMesh);
}
//---------------------------------------------------------------------
// Added by DrEvil
void 
vl::MeshSerializerImpl::writeSubMeshNameTable(vl::Mesh const *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::writeSubMeshNameTable" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
	// Header
	writeChunkHeader(Ogre::M_SUBMESH_NAME_TABLE, calcSubMeshNameTableSize(pMesh));

	// Loop through and save out the index and names.
	Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();

	while(it != pMesh->mSubMeshNameMap.end())
	{
		// Header
		writeChunkHeader(Ogre::M_SUBMESH_NAME_TABLE_ELEMENT, STREAM_OVERHEAD_SIZE +
			sizeof(unsigned short) + (unsigned long)it->first.length() + 1);

		// write the index
		writeShorts(&it->second, 1);
		// name
	    writeString(it->first);

		++it;
	}
	*/
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeSubMesh(vl::SubMesh const *s)
{
	std::clog << "vl::MeshSerializerImpl::writeSubMesh" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    // Header
    writeChunkHeader(Ogre::M_SUBMESH, calcSubMeshSize(s));

    // char* materialName
    writeString(s->getMaterialName());

    // bool useSharedVertices
    writeBools(&s->useSharedVertices, 1);

	unsigned int indexCount = s->indexData->indexCount;
    writeInts(&indexCount, 1);

    // bool indexes32Bit
    bool idx32bit = (!s->indexData->indexBuffer.isNull() &&
		s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
    writeBools(&idx32bit, 1);

	if (indexCount > 0)
	{
		// unsigned short* faceVertexIndices ((indexCount)
		HardwareIndexBufferSharedPtr ibuf = s->indexData->indexBuffer;
		void* pIdx = ibuf->lock(HardwareBuffer::HBL_READ_ONLY);
		if (idx32bit)
		{
			unsigned int* pIdx32 = static_cast<unsigned int*>(pIdx);
			writeInts(pIdx32, s->indexData->indexCount);
		}
		else
		{
			unsigned short* pIdx16 = static_cast<unsigned short*>(pIdx);
			writeShorts(pIdx16, s->indexData->indexCount);
		}
		ibuf->unlock();
	}

    // M_GEOMETRY stream (Optional: present only if useSharedVertices = false)
    if (!s->useSharedVertices)
    {
        writeGeometry(s->vertexData);
    }

    // end of sub mesh chunk

    // write out texture alias chunks
    writeSubMeshTextureAliases(s);

    // Operation type
    writeSubMeshOperation(s);

    // Bone assignments
    if (!s->mBoneAssignments.empty())
    {
        LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

        SubMesh::VertexBoneAssignmentList::const_iterator vi;
        for (vi = s->mBoneAssignments.begin();
        vi != s->mBoneAssignments.end(); ++vi)
        {
            writeSubMeshBoneAssignment(vi->second);
        }

        LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");
    }
	*/
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeExtremes(vl::Mesh const *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::writeExtremes" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    bool has_extremes = false;
    for (unsigned short i = 0; i < pMesh->getNumSubMeshes(); ++i)
    {
        SubMesh *sm = pMesh->getSubMesh(i);
        if (sm->extremityPoints.empty())
            continue;
        if (!has_extremes)
        {
            has_extremes = true;
            std::cout << "Writing submesh extremes..." << std::endl;
        }
        writeSubMeshExtremes(i, sm);
    }
    if (has_extremes)
	{ std::cout << "Extremes exported." << std::endl; }
	*/
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeSubMeshExtremes(unsigned short idx, vl::SubMesh const *s)
{
	std::clog << "vl::MeshSerializerImpl::writeSubMeshExtremes" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    size_t chunkSize = STREAM_OVERHEAD_SIZE + sizeof (unsigned short) +
        s->extremityPoints.size () * sizeof (float) * 3;
    writeChunkHeader(Ogre::M_TABLE_EXTREMES, chunkSize);

    writeShorts(&idx, 1);

    float *vertices = OGRE_ALLOC_T(float, s->extremityPoints.size() * 3, MEMCATEGORY_GEOMETRY);
	float *pVert = vertices;

    for(std::vector<Ogre::Vector3>::type::const_iterator i = s->extremityPoints.begin();
            i != s->extremityPoints.end(); ++i)
    {
		*pVert++ = i->x;
		*pVert++ = i->y;
		*pVert++ = i->z;
    }

    writeFloats(vertices, s->extremityPoints.size () * 3);
    OGRE_FREE(vertices, MEMCATEGORY_GEOMETRY);
	*/
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeSubMeshTextureAliases(vl::SubMesh const *s)
{
	std::clog << "vl::MeshSerializerImpl::writeSubMeshTextureAliases" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    size_t chunkSize;
    AliasTextureNamePairList::const_iterator i;

	std::cout << "Exporting submesh texture aliases..." << std::endl;

    // iterate through texture aliases and write them out as a chunk
    for (i = s->mTextureAliases.begin(); i != s->mTextureAliases.end(); ++i)
    {
        // calculate chunk size based on string length + 1.  Add 1 for the line feed.
        chunkSize = STREAM_OVERHEAD_SIZE + i->first.length() + i->second.length() + 2;
		writeChunkHeader(Ogre::M_SUBMESH_TEXTURE_ALIAS, chunkSize);
        // write out alias name
        writeString(i->first);
        // write out texture name
        writeString(i->second);
    }

	std::cout << "Submesh texture aliases exported." << std::endl;
	*/
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeSubMeshOperation(vl::SubMesh const *sm)
{
	std::clog << "vl::MeshSerializerImpl::writeSubMeshOperation" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    // Header
    writeChunkHeader(Ogre::M_SUBMESH_OPERATION, calcSubMeshOperationSize(sm));

    // unsigned short operationType
    unsigned short opType = static_cast<unsigned short>(sm->operationType);
    writeShorts(&opType, 1);
	*/
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeGeometry(Mesh const *pMesh, VertexData const *pSrc)
{
	std::clog << "vl::MeshSerializerImpl::writeGeometry" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
	// calc size
    const VertexDeclaration::VertexElementList& elemList =
        vertexData->vertexDeclaration->getElements();
    const VertexBufferBinding::VertexBufferBindingMap& bindings =
        vertexData->vertexBufferBinding->getBindings();
    VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbiend;

	size_t size = STREAM_OVERHEAD_SIZE + sizeof(unsigned int) + // base
		(STREAM_OVERHEAD_SIZE + elemList.size() * (STREAM_OVERHEAD_SIZE + sizeof(unsigned short) * 5)); // elements
    vbiend = bindings.end();
	for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
	{
		const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
		size += (STREAM_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
	}

	// Header
    writeChunkHeader(Ogre::M_GEOMETRY, size);

    unsigned int vertexCount = vertexData->vertexCount;
    writeInts(&vertexCount, 1);

	// Vertex declaration
	size = STREAM_OVERHEAD_SIZE + elemList.size() * (STREAM_OVERHEAD_SIZE + sizeof(unsigned short) * 5);
	writeChunkHeader(Ogre::M_GEOMETRY_VERTEX_DECLARATION, size);

    VertexDeclaration::VertexElementList::const_iterator vei, veiend;
	veiend = elemList.end();
	unsigned short tmp;
	size = STREAM_OVERHEAD_SIZE + sizeof(unsigned short) * 5;
	for (vei = elemList.begin(); vei != veiend; ++vei)
	{
		const VertexElement& elem = *vei;
		writeChunkHeader(Ogre::M_GEOMETRY_VERTEX_ELEMENT, size);
		// unsigned short source;  	// buffer bind source
		tmp = elem.getSource();
		writeShorts(&tmp, 1);
		// unsigned short type;    	// VertexElementType
		tmp = static_cast<unsigned short>(elem.getType());
		writeShorts(&tmp, 1);
		// unsigned short semantic; // VertexElementSemantic
		tmp = static_cast<unsigned short>(elem.getSemantic());
		writeShorts(&tmp, 1);
		// unsigned short offset;	// start offset in buffer in bytes
		tmp = static_cast<unsigned short>(elem.getOffset());
		writeShorts(&tmp, 1);
		// unsigned short index;	// index of the semantic (for colours and texture coords)
		tmp = elem.getIndex();
		writeShorts(&tmp, 1);

	}

	// Buffers and bindings
	vbiend = bindings.end();
	for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
	{
		const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
		size = (STREAM_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
		writeChunkHeader(Ogre::M_GEOMETRY_VERTEX_BUFFER,  size);
		// unsigned short bindIndex;	// Index to bind this buffer to
		tmp = vbi->first;
		writeShorts(&tmp, 1);
		// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
		tmp = (unsigned short)vbuf->getVertexSize();
		writeShorts(&tmp, 1);

		// Data
		size = STREAM_OVERHEAD_SIZE + vbuf->getSizeInBytes();
		writeChunkHeader(Ogre::M_GEOMETRY_VERTEX_BUFFER_DATA, size);
		void* pBuf = vbuf->lock(HardwareBuffer::HBL_READ_ONLY);

		if (mFlipEndian)
		{
			// endian conversion
			// Copy data
			unsigned char* tempData = OGRE_ALLOC_T(unsigned char, vbuf->getSizeInBytes(), MEMCATEGORY_GEOMETRY);
			memcpy(tempData, pBuf, vbuf->getSizeInBytes());
			flipToLittleEndian(
				tempData,
				vertexData->vertexCount,
				vbuf->getVertexSize(),
				vertexData->vertexDeclaration->findElementsBySource(vbi->first));
			writeData(tempData, vbuf->getVertexSize(), vertexData->vertexCount);
			OGRE_FREE(tempData, MEMCATEGORY_GEOMETRY);
		}
		else
		{
			writeData(pBuf, vbuf->getVertexSize(), vertexData->vertexCount);
		}
        vbuf->unlock();
	}
	*/
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcSubMeshNameTableSize(vl::Mesh const *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::calcSubMeshNameTableSize" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
	size_t size = STREAM_OVERHEAD_SIZE;
	// Figure out the size of the Name table.
	// Iterate through the subMeshList & add up the size of the indexes and names.
	Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();
	while(it != pMesh->mSubMeshNameMap.end())
	{
		// size of the index + header size for each element chunk
		size += STREAM_OVERHEAD_SIZE + sizeof(uint16);
		// name
		size += it->first.length() + 1;

		++it;
	}

	// size of the sub-mesh name table.
	return size;
	*/
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcMeshSize(vl::Mesh const *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::calcMeshSize" << std::endl;
    size_t size = STREAM_OVERHEAD_SIZE;

    // Num shared vertices
    size += sizeof(uint32_t);

    // Geometry
	if(pMesh->sharedVertexData && pMesh->sharedVertexData->getNVertices() > 0)
	{
		size += calcGeometrySize(pMesh->sharedVertexData);
	}

    // Submeshes
    for (unsigned short i = 0; i < pMesh->getNumSubMeshes(); ++i)
    {
        size += calcSubMeshSize(pMesh->getSubMesh(i));
    }

    // Skeleton link
    /* not supported
	if (pMesh->hasSkeleton())
    {
        size += calcSkeletonLinkSize(pMesh->getSkeletonName());
    }
	*/

	// Submesh name table
	size += calcSubMeshNameTableSize(pMesh);

	// Edge list
	/* not supported
	if (pMesh->isEdgeListBuilt())
	{
		size += calcEdgeListSize(pMesh);
	}
	*/

	// Animations
	/* not supported
	for (unsigned short a = 0; a < pMesh->getNumAnimations(); ++a)
	{
		Animation* anim = pMesh->getAnimation(a);
		size += calcAnimationSize(anim);
	}
	*/

	return size;
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcSubMeshSize(vl::SubMesh const *pSub)
{
	std::clog << "vl::MeshSerializerImpl::calcSubMeshSize" << std::endl;
    size_t size = STREAM_OVERHEAD_SIZE;
		
	bool idx32bit = (pSub->indexData.indexCount() && pSub->indexData.getIndexSize() == vl::IT_32BIT);

    // Material name
    size += pSub->getMaterial().length() + 1;

    // bool useSharedVertices
    size += sizeof(bool);
    // unsigned int indexCount
    size += sizeof(unsigned int);
    // bool indexes32bit
    size += sizeof(bool);
    // unsigned int* / unsigned short* faceVertexIndices
	if (idx32bit)
		size += sizeof(unsigned int) * pSub->indexData.indexCount();
	else
		size += sizeof(unsigned short) * pSub->indexData.indexCount();
    // Geometry
    if (!pSub->useSharedGeometry)
    {
        size += calcGeometrySize(pSub->vertexData);
    }

    size += calcSubMeshTextureAliasesSize(pSub);
    size += calcSubMeshOperationSize(pSub);

    // Bone assignments
	/* not supported
    if (!pSub->mBoneAssignments.empty())
    {
        SubMesh::VertexBoneAssignmentList::const_iterator vi;
        for (vi = pSub->mBoneAssignments.begin();
                vi != pSub->mBoneAssignments.end(); ++vi)
        {
            size += calcBoneAssignmentSize();
        }
    }
	*/

    return size;
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcSubMeshOperationSize(vl::SubMesh const *pSub)
{
	std::clog << "vl::MeshSerializerImpl::calcSubMeshOperationSize" << std::endl;
    return STREAM_OVERHEAD_SIZE + sizeof(uint16_t);
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcSubMeshTextureAliasesSize(vl::SubMesh const *pSub)
{
	std::clog << "vl::MeshSerializerImpl::calcSubMeshTextureAliasesSize" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	
    size_t chunkSize = 0;
    /*
	AliasTextureNamePairList::const_iterator i;

    // iterate through texture alias map and calc size of strings
    for (i = pSub->mTextureAliases.begin(); i != pSub->mTextureAliases.end(); ++i)
    {
        // calculate chunk size based on string length + 1.  Add 1 for the line feed.
        chunkSize += STREAM_OVERHEAD_SIZE + i->first.length() + i->second.length() + 2;
    }
	*/
    return chunkSize;
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcGeometrySize(VertexData const *vertexData)
{
	std::clog << "vl::MeshSerializerImpl::calcGeometrySize" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());

    size_t size = STREAM_OVERHEAD_SIZE;

    // Num vertices
    size += sizeof(unsigned int);

	/* @todo not implemented
    const VertexDeclaration::VertexElementList& elems =
        vertexData->vertexDeclaration->getElements();

    VertexDeclaration::VertexElementList::const_iterator i, iend;
    iend = elems.end();
    for (i = elems.begin(); i != iend; ++i)
    {
        const VertexElement& elem = *i;
        // Vertex element
        size += VertexElement::getTypeSize(elem.getType()) * vertexData->vertexCount;
    }
	*/

    return size;
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readGeometry(vl::ResourceStream &stream, vl::Mesh *pMesh, VertexData *pDest)
{
//	std::clog << "vl::MeshSerializerImpl::readGeometry" << std::endl;
	assert(pMesh && pDest);
//    dest->vertexStart = 0;

    unsigned int vertexCount = 0;
    readInts(stream, &vertexCount, 1);
    pDest->setNVertices(vertexCount);
	//dest->vertexCount = vertexCount;

    // Find optional geometry streams
    if (!stream.eof())
    {
        unsigned short streamID = readChunk(stream);
        while(!stream.eof() &&
            (streamID == Ogre::M_GEOMETRY_VERTEX_DECLARATION ||
                streamID == Ogre::M_GEOMETRY_VERTEX_BUFFER ))
        {
            switch (streamID)
            {
            case Ogre::M_GEOMETRY_VERTEX_DECLARATION:
                readGeometryVertexDeclaration(stream, pMesh, pDest);
                break;
            case Ogre::M_GEOMETRY_VERTEX_BUFFER:
                readGeometryVertexBuffer(stream, pMesh, pDest);
                break;
            }
			// Get next stream
			if(!stream.eof())
			{
				streamID = readChunk(stream);
			}
		}
		if(!stream.eof())
		{
			// Backpedal back to start of non-submesh stream
			stream.skip(-STREAM_OVERHEAD_SIZE);
		}
	}
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readGeometryVertexDeclaration(vl::ResourceStream &stream, vl::Mesh *pMesh, VertexData *pDest)
{
//	std::clog << "vl::MeshSerializerImpl::readGeometryVertexDeclaration" << std::endl;
    // Find optional geometry streams
    if(!stream.eof())
    {
        unsigned short streamID = readChunk(stream);
        while(!stream.eof() &&
            (streamID == Ogre::M_GEOMETRY_VERTEX_ELEMENT ))
        {
            switch(streamID)
            {
			case Ogre::M_GEOMETRY_VERTEX_ELEMENT:
                readGeometryVertexElement(stream, pMesh, pDest);
                break;
            }
            // Get next stream
            if(!stream.eof())
            {
                streamID = readChunk(stream);
            }
        }
        if(!stream.eof())
        {
            // Backpedal back to start of non-submesh stream
            stream.skip(-STREAM_OVERHEAD_SIZE);
        }
    }

}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readGeometryVertexElement(vl::ResourceStream &stream, vl::Mesh *pMesh, VertexData *pDest)
{
//	std::clog << "vl::MeshSerializerImpl::readGeometryVertexElement" << std::endl;
	assert(pMesh && pDest);

	unsigned short source, offset, index, tmp;
	Ogre::VertexElementType vType;
	Ogre::VertexElementSemantic vSemantic;
	// unsigned short source;  	// buffer bind source
	readShorts(stream, &source, 1);
	// unsigned short type;    	// VertexElementType
	readShorts(stream, &tmp, 1);
	vType = static_cast<Ogre::VertexElementType>(tmp);
	// unsigned short semantic; // VertexElementSemantic
	readShorts(stream, &tmp, 1);
	vSemantic = static_cast<Ogre::VertexElementSemantic>(tmp);
	// unsigned short offset;	// start offset in buffer in bytes
	readShorts(stream, &offset, 1);
	// unsigned short index;	// index of the semantic
	readShorts(stream, &index, 1);

//	std::clog << "Data read : type =" << vType << " semantic = " << vSemantic << std::endl;
	pDest->vertexDeclaration.addSemantic(vSemantic, vType);

	//dest->vertexDeclaration->addElement(source, offset, vType, vSemantic, index);

	if (vType == Ogre::VET_COLOUR)
	{
		std::clog
			<< "Warning: VET_COLOUR element type is deprecated, you should use "
			<< "one of the more specific types to indicate the byte order. "
			<< "Use OgreMeshUpgrade on " << pMesh->getName() << " as soon as possible. ";
	}

}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readGeometryVertexBuffer(vl::ResourceStream &stream, vl::Mesh* pMesh, VertexData *pDest)
{
//	std::clog << "vl::MeshSerializerImpl::readGeometryVertexBuffer" << std::endl;

	unsigned short bindIndex, vertexSize;
	// unsigned short bindIndex;	// Index to bind this buffer to
	readShorts(stream, &bindIndex, 1);
	// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
	readShorts(stream, &vertexSize, 1);

	// Check for vertex data header
	unsigned short headerID;
	headerID = readChunk(stream);
	if (headerID != Ogre::M_GEOMETRY_VERTEX_BUFFER_DATA)
	{
		std::string msg("Can't find vertex buffer data area");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
	}
	// Check that vertex size agrees
	// @todo this needs a vertex attributes to be disabled/enabled on need
	if(pDest->vertexDeclaration.vertexSize() != vertexSize)
	{
		std::stringstream ss;
		ss << "Buffer vertex size " << vertexSize
			<< " does not agree with vertex declaration size "
			<< pDest->vertexDeclaration.vertexSize();
		std::clog << ss.str() << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
	}

	// Create / populate vertex buffer
	size_t vertexCount = pDest->getNVertices();
	size_t bufferSize = vertexCount * vertexSize;

	if(bindIndex != 0)
	{
		std::clog << "Trying to bind data to other than first Buffer. Not supported." << std::endl;
	}

//	std::clog << "Adding " << vertexCount << " vertices." << std::endl;
	/// @todo replace with copying the whole stream at once
	char *pBuf = new char[vertexSize];
	for(size_t i = 0; i < vertexCount; ++i)
	{
		stream.read(pBuf, vertexSize);

		pDest->setVertex(i, pBuf, vertexSize);
	}

	// endian conversion for OSX
	/*	Endian conversion not supported
	flipFromLittleEndian(
		pBuf,
		dest->vertexCount,
		vertexSize,
		dest->vertexDeclaration->findElementsBySource(bindIndex));
	*/

//	vbuf->unlock();

	// Set binding
//	dest->vertexBufferBinding->setBinding(bindIndex, vbuf);
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readSubMeshNameTable(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
//	std::clog << "vl::MeshSerializerImpl::readSubMeshNameTable" << std::endl;

	// The map for
	std::map<unsigned short, std::string> subMeshNames;
	unsigned short streamID, subMeshIndex;

	// Need something to store the index, and the objects name
	// This table is a method that imported meshes can retain their naming
	// so that the names established in the modelling software can be used
	// to get the sub-meshes by name. The exporter must support exporting
	// the optional stream M_SUBMESH_NAME_TABLE.

    // Read in all the sub-streams. Each sub-stream should contain an index and Ogre::String for the name.
	if(!stream.eof())
	{
		streamID = readChunk(stream);
		while(!stream.eof() && (streamID == Ogre::M_SUBMESH_NAME_TABLE_ELEMENT ))
		{
			// Read in the index of the submesh.
			readShorts(stream, &subMeshIndex, 1);
			// Read in the String and map it to its index.
			subMeshNames[subMeshIndex] = readString(stream);

			// If we're not end of file get the next stream ID
			if (!stream.eof())
				streamID = readChunk(stream);
		}
		if (!stream.eof())
		{
			// Backpedal back to start of stream
			stream.skip(-STREAM_OVERHEAD_SIZE);
		}
	}

	// Set all the submeshes names
	// ?

	// Loop through and save out the index and names.

	std::map<unsigned short, std::string>::const_iterator it = subMeshNames.begin();

	while(it != subMeshNames.end())
	{
		// Name this submesh to the stored name.
		pMesh->nameSubMesh(it->second, it->first);
		++it;
	}
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readMesh(vl::ResourceStream &stream, Mesh *pMesh)
{
//	std::clog << "vl::MeshSerializerImpl::readMesh" << std::endl;

	unsigned short streamID;

	// bool skeletallyAnimated
	bool skeletallyAnimated;
	readBools(stream, &skeletallyAnimated, 1);

	// Find all substreams
	if(!stream.eof())
	{
//		std::clog << "vl::MeshSerializerImpl::readMesh : stream has data" << std::endl;
		streamID = readChunk(stream);
		while(!stream.eof() &&
			(streamID == Ogre::M_GEOMETRY ||
				streamID == Ogre::M_SUBMESH ||
				streamID == Ogre::M_MESH_SKELETON_LINK ||
				streamID == Ogre::M_MESH_BONE_ASSIGNMENT ||
				streamID == Ogre::M_MESH_LOD ||
				streamID == Ogre::M_MESH_BOUNDS ||
				streamID == Ogre::M_SUBMESH_NAME_TABLE ||
				streamID == Ogre::M_EDGE_LISTS ||
				streamID == Ogre::M_POSES ||
				streamID == Ogre::M_ANIMATIONS ||
				streamID == Ogre::M_TABLE_EXTREMES))
		{
//			std::clog << "Reading Mesh chunck" << std::endl;
			switch(streamID)
			{
				case Ogre::M_GEOMETRY:
					assert(!pMesh->sharedVertexData);
					pMesh->sharedVertexData = new VertexData;
					try {
//						std::clog << "Reading Mesh shared geometry." << std::endl;
						readGeometry(stream, pMesh, pMesh->sharedVertexData);
					}
					catch(...)
					{
						std::clog << "Exception thrown by readGeometry" << std::endl;
						throw;
					}
					/*
					catch(Exception& e)
					{
						if (e.getNumber() == Exception::ERR_ITEM_NOT_FOUND)
						{
							// duff geometry data entry with 0 vertices
							OGRE_DELETE pMesh->sharedVertexData;
							pMesh->sharedVertexData = 0;
							// Skip this stream (pointer will have been returned to just after header)
							stream->skip(mCurrentstreamLen - STREAM_OVERHEAD_SIZE);
						}
						else
						{
							throw;
						}
					}
					*/
					break;
				case Ogre::M_SUBMESH:
					readSubMesh(stream, pMesh);
					break;
				case Ogre::M_MESH_SKELETON_LINK:
				{
					std::string msg("Trying to read Skeleton link, NO READER");
					std::clog << msg << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
					//readSkeletonLink(stream, pMesh, listener);
				}
				break;

				case Ogre::M_MESH_BONE_ASSIGNMENT:
				{
					std::string msg("Trying to read Bone assigments, NO READER");
					std::clog << msg << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
					//readMeshBoneAssignment(stream, pMesh);
				}
				break;
			
				case Ogre::M_MESH_LOD:
				{
					std::string msg("Trying to read LOD, NO READER");
					std::clog << msg << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
					//readMeshLodInfo(stream, pMesh);
				}
				break;
			
				case Ogre::M_MESH_BOUNDS:
					readBoundsInfo(stream, pMesh);
					break;

				case Ogre::M_SUBMESH_NAME_TABLE:
    				readSubMeshNameTable(stream, pMesh);
					break;

				case Ogre::M_EDGE_LISTS:
					readEdgeList(stream, pMesh);
					break;

				case Ogre::M_POSES:
				{
					std::string msg("Trying to read Poses, NO READER");
					std::clog << msg << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
					//readPoses(stream, pMesh);
				}
				break;
			
				case Ogre::M_ANIMATIONS:
				{
					std::string msg("Trying to read Animations, NO READER");
					std::clog << msg << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
					//readAnimations(stream, pMesh);
				}
				break;
			
				case Ogre::M_TABLE_EXTREMES:
					readExtremes(stream, pMesh);
					break;
			}

            if (!stream.eof())
            {
                streamID = readChunk(stream);
            }

        }
        if(!stream.eof())
        {
            // Backpedal back to start of stream
            stream.skip(-STREAM_OVERHEAD_SIZE);
        }
    }
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readSubMesh(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
//	std::clog << "vl::MeshSerializerImpl::readSubMesh" << std::endl;
    unsigned short streamID;

    SubMesh* sm = pMesh->createSubMesh();

    // char* materialName
    std::string materialName = readString(stream);
    sm->setMaterial(materialName);

    readBools(stream, &sm->useSharedGeometry, 1);
	
	// @todo should we clear the sub mesh indexes?
//    sm->indexData->indexStart = 0;
    unsigned int indexCount = 0;
    readInts(stream, &indexCount, 1);
    //sm->indexData->indexCount = indexCount;
	if(indexCount%3)
	{
		std::clog << "Something fishy in SubMesh index count is not dividable with three." << std::endl;
		BOOST_THROW_EXCEPTION(vl::not_implemented());
	}

    //HardwareIndexBufferSharedPtr ibuf;
    bool idx32bit;
    readBools(stream, &idx32bit, 1);
    if (indexCount > 0)
    {
        if (idx32bit)
        {
//			std::clog << "Reading 32-bit indexes, " << indexCount << " of them." << std::endl;
			/*
            ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_32BIT,
                    sm->indexData->indexCount,
                    pMesh->mIndexBufferUsage,
					pMesh->mIndexBufferShadowBuffer);
            // unsigned int* faceVertexIndices
            unsigned int* pIdx = static_cast<unsigned int*>(
                ibuf->lock(HardwareBuffer::HBL_DISCARD)
                );
			*/
			sm->indexData.setIndexSize(vl::IT_32BIT);
			sm->indexData.setIndexCount(indexCount);
			readInts(stream, sm->indexData.getBuffer32(), indexCount);

            //ibuf->unlock();
        }
        else // 16-bit
        {
			/*
            ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_16BIT,
                    sm->indexData->indexCount,
                    pMesh->mIndexBufferUsage,
					pMesh->mIndexBufferShadowBuffer);
            // unsigned short* faceVertexIndices
            unsigned short* pIdx = static_cast<unsigned short*>(
                ibuf->lock(HardwareBuffer::HBL_DISCARD)
                );
            readShorts(stream, pIdx, sm->indexData->indexCount);
            ibuf->unlock();
			*/
//			std::clog << "Reading 16-bit indexes, " << indexCount << " of them." << std::endl;
			sm->indexData.setIndexSize(vl::IT_16BIT);
			/// Get the half count of indexes, if it's odd it will be increased
			/// by one half, if it's even then the odd one is out in the division
			/// We use the half size to resize the container, but the read is
			/// of course done with the correct number of indices.
			sm->indexData.setIndexCount(indexCount);
			readShorts(stream, sm->indexData.getBuffer16(), indexCount);
			/*
			std::clog << "Index buffer = ";
			for(size_t i = 0; i < indexCount; ++i)
			{
				std::clog << sm->indexData.getVec16().at(i) << ", ";
			}
			std::clog << std::endl;
			*/
			/*
			for(size_t i = 0; i < indexCount/3; ++i)
			{
				uint16_t buf[3];
				readShorts(stream, buf, 3);
				sm->addFace(buf[0], buf[1], buf[3]);
			}
			*/
		}
	}
//    sm->indexData->indexBuffer = ibuf;

    // M_GEOMETRY stream (Optional: present only if useSharedVertices = false)
    if(!sm->useSharedGeometry)
    {
//		std::clog << "Reading non shared geometry." << std::endl;
        streamID = readChunk(stream);
        if (streamID != Ogre::M_GEOMETRY)
        {
			std::string msg("Missing geometry data in mesh file");
			std::clog << msg << std::endl;
			BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
        }
        sm->vertexData = new VertexData;
        readGeometry(stream, pMesh, sm->vertexData);
    }

    // Find all bone assignments, submesh operation, and texture aliases (if present)
    if(!stream.eof())
    {
//		std::clog << "Reading all other submesh attributes." << std::endl;
        streamID = readChunk(stream);
        while(!stream.eof() &&
            (streamID == Ogre::M_SUBMESH_BONE_ASSIGNMENT ||
                streamID == Ogre::M_SUBMESH_OPERATION ||
                streamID == Ogre::M_SUBMESH_TEXTURE_ALIAS))
        {
            switch(streamID)
            {
            case Ogre::M_SUBMESH_OPERATION:
                readSubMeshOperation(stream, sm);
                break;
            case Ogre::M_SUBMESH_BONE_ASSIGNMENT:
                readSubMeshBoneAssignment(stream, pMesh, sm);
                break;
            case Ogre::M_SUBMESH_TEXTURE_ALIAS:
                readSubMeshTextureAlias(stream, sm);
                break;
            }

            if(!stream.eof())
            {
                streamID = readChunk(stream);
            }

        }
        if(!stream.eof())
        {
            // Backpedal back to start of stream
            stream.skip(-STREAM_OVERHEAD_SIZE);
        }
    }
}

//---------------------------------------------------------------------
void
vl::MeshSerializerImpl::readSubMeshOperation(vl::ResourceStream &stream, vl::SubMesh *sm)
{
//	std::clog << "vl::MeshSerializerImpl::readSubMeshOperation" << std::endl;
    // unsigned short operationType
    unsigned short opType;
    readShorts(stream, &opType, 1);
    sm->operationType = static_cast<Ogre::RenderOperation::OperationType>(opType);
}
//---------------------------------------------------------------------
void
vl::MeshSerializerImpl::readSubMeshTextureAlias(vl::ResourceStream &stream, vl::SubMesh *sub)
{
//	std::clog << "vl::MeshSerializerImpl::readSubMeshTextureAlias" << std::endl;
    std::string aliasName = readString(stream);
    std::string textureName = readString(stream);
    std::cout << "vl::SubMesh::addTextureAlias does not exists." << std::endl;
	//sub->addTextureAlias(aliasName, textureName);
}
//---------------------------------------------------------------------
///	Skeletons not supported
void 
vl::MeshSerializerImpl::writeSkeletonLink(std::string const &skelName)
{
	std::clog << "vl::MeshSerializerImpl::writeSkeletonLink" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*	Skeletons not supported
    writeChunkHeader(M_MESH_SKELETON_LINK, calcSkeletonLinkSize(skelName));

    writeString(skelName);
	*/

}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readSkeletonLink(vl::ResourceStream &stream, vl::Mesh* pMesh)
{
	std::clog << "vl::MeshSerializerImpl::readSkeletonLink" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    std::string skelName = readString(stream);

    pMesh->setSkeletonName(skelName);
	*/
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcSkeletonLinkSize(std::string const &skelName)
{
	std::clog << "vl::MeshSerializerImpl::calcSkeletonLinkSize" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
    size_t size = STREAM_OVERHEAD_SIZE;

    size += skelName.length() + 1;

    return size;
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeMeshBoneAssignment(vl::VertexBoneAssignment const &assign)
{
	std::clog << "vl::MeshSerializerImpl::writeMeshBoneAssignment" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    writeChunkHeader(Ogre::M_MESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

    // unsigned int vertexIndex;
    writeInts(&(assign.vertexIndex), 1);
    // unsigned short boneIndex;
    writeShorts(&(assign.boneIndex), 1);
    // float weight;
    writeFloats(&(assign.weight), 1);
	*/
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeSubMeshBoneAssignment(vl::VertexBoneAssignment const &assign)
{
	std::clog << "vl::MeshSerializerImpl::writeSubMeshBoneAssignment" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    writeChunkHeader(M_SUBMESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

    // unsigned int vertexIndex;
    writeInts(&(assign.vertexIndex), 1);
    // unsigned short boneIndex;
    writeShorts(&(assign.boneIndex), 1);
    // float weight;
    writeFloats(&(assign.weight), 1);
	*/
}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readMeshBoneAssignment(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::readMeshBoneAssignment" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    VertexBoneAssignment assign;

    // unsigned int vertexIndex;
    readInts(stream, &(assign.vertexIndex),1);
    // unsigned short boneIndex;
    readShorts(stream, &(assign.boneIndex),1);
    // float weight;
    readFloats(stream, &(assign.weight), 1);

    pMesh->addBoneAssignment(assign);
	*/

}
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readSubMeshBoneAssignment(vl::ResourceStream &stream, vl::Mesh *pMesh, vl::SubMesh *sub)
{
	std::clog << "vl::MeshSerializerImpl::readSubMeshBoneAssignment" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
    VertexBoneAssignment assign;

    // unsigned int vertexIndex;
    readInts(stream, &(assign.vertexIndex),1);
    // unsigned short boneIndex;
    readShorts(stream, &(assign.boneIndex),1);
    // float weight;
    readFloats(stream, &(assign.weight), 1);

    sub->addBoneAssignment(assign);
	*/
}
//---------------------------------------------------------------------
size_t 
vl::MeshSerializerImpl::calcBoneAssignmentSize(void)
{
	std::clog << "vl::MeshSerializerImpl::calcBoneAssignmentSize" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
    size_t size = STREAM_OVERHEAD_SIZE;

    // Vert index
    size += sizeof(unsigned int);
    // Bone index
    size += sizeof(unsigned short);
    // weight
    size += sizeof(float);

    return size;
}

//---------------------------------------------------------------------
/*	LOD not suported
void MeshSerializerImpl::writeLodInfo(const Mesh* pMesh)
{
    const LodStrategy *strategy = pMesh->getLodStrategy();
    unsigned short numLods = pMesh->getNumLodLevels();
    bool manual = pMesh->isLodManual();
    writeLodSummary(numLods, manual, strategy);

	// Loop from LOD 1 (not 0, this is full detail)
    for (unsigned short i = 1; i < numLods; ++i)
    {
		const MeshLodUsage& usage = pMesh->getLodLevel(i);
		if (manual)
		{
			writeLodUsageManual(usage);
		}
		else
		{
			writeLodUsageGenerated(pMesh, usage, i);
		}

    }


}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeLodSummary(unsigned short numLevels, bool manual, const LodStrategy *strategy)
{
    // Header
    size_t size = STREAM_OVERHEAD_SIZE;
    // unsigned short numLevels;
    size += sizeof(unsigned short);
    // bool manual;  (true for manual alternate meshes, false for generated)
    size += sizeof(bool);
    writeChunkHeader(M_MESH_LOD, size);

    // Details
    // string strategyName;
    writeString(strategy->getName());
    // unsigned short numLevels;
    writeShorts(&numLevels, 1);
    // bool manual;  (true for manual alternate meshes, false for generated)
    writeBools(&manual, 1);


}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeLodUsageManual(const MeshLodUsage& usage)
{
    // Header
    size_t size = STREAM_OVERHEAD_SIZE;
    size_t manualSize = STREAM_OVERHEAD_SIZE;
    // float lodValue;
    size += sizeof(float);
    // Manual part size

    // String manualMeshName;
    manualSize += usage.manualName.length() + 1;

    size += manualSize;

    writeChunkHeader(M_MESH_LOD_USAGE, size);
    writeFloats(&(usage.userValue), 1);

    writeChunkHeader(M_MESH_LOD_MANUAL, manualSize);
    writeString(usage.manualName);


}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeLodUsageGenerated(const Mesh* pMesh, const MeshLodUsage& usage,
	unsigned short lodNum)
{
	// Usage Header
    size_t size = STREAM_OVERHEAD_SIZE;
	unsigned short subidx;

    // float fromDepthSquared;
    size += sizeof(float);

    // Calc generated SubMesh sections size
	for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
	{
		// header
		size += STREAM_OVERHEAD_SIZE;
		// unsigned int numFaces;
		size += sizeof(unsigned int);
		SubMesh* sm = pMesh->getSubMesh(subidx);
        const IndexData* indexData = sm->mLodFaceList[lodNum - 1];

        // bool indexes32Bit
		size += sizeof(bool);
		// int* faceIndexes;
        if (!indexData->indexBuffer.isNull() &&
			indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
        {
			size += static_cast<unsigned long>(
                sizeof(unsigned int) * indexData->indexCount);
        }
        else
        {
			size += static_cast<unsigned long>(
                sizeof(unsigned short) * indexData->indexCount);
        }

	}

    writeChunkHeader(M_MESH_LOD_USAGE, size);
    writeFloats(&(usage.userValue), 1);

	// Now write sections
    // Calc generated SubMesh sections size
	for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
	{
		size = STREAM_OVERHEAD_SIZE;
		// unsigned int numFaces;
		size += sizeof(unsigned int);
		SubMesh* sm = pMesh->getSubMesh(subidx);
        const IndexData* indexData = sm->mLodFaceList[lodNum - 1];
        // bool indexes32Bit
		size += sizeof(bool);
		// Lock index buffer to write
		HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;
		// bool indexes32bit
		bool idx32 = (!ibuf.isNull() && ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
		// int* faceIndexes;
        if (idx32)
        {
			size += static_cast<unsigned long>(
                sizeof(unsigned int) * indexData->indexCount);
        }
        else
        {
			size += static_cast<unsigned long>(
                sizeof(unsigned short) * indexData->indexCount);
        }

		writeChunkHeader(Ogre::M_MESH_LOD_GENERATED, size);
		unsigned int idxCount = static_cast<unsigned int>(indexData->indexCount);
		writeInts(&idxCount, 1);
		writeBools(&idx32, 1);

		if (idxCount > 0)
		{
			if (idx32)
			{
				unsigned int* pIdx = static_cast<unsigned int*>(
					ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
				writeInts(pIdx, indexData->indexCount);
				ibuf->unlock();
			}
			else
			{
				unsigned short* pIdx = static_cast<unsigned short*>(
					ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
				writeShorts(pIdx, indexData->indexCount);
				ibuf->unlock();
			}
		}
	}
}
*/

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::writeBoundsInfo(const Mesh* pMesh)
{
	std::clog << "vl::MeshSerializerImpl::writeBoundsInfo" << std::endl;
	// Usage Header
    unsigned long size = STREAM_OVERHEAD_SIZE;

    size += sizeof(float) * 7;
    writeChunkHeader(Ogre::M_MESH_BOUNDS, size);

    // float minx, miny, minz
    const Ogre::Vector3& min = pMesh->getBounds().getMinimum();
    const Ogre::Vector3& max = pMesh->getBounds().getMaximum();
    writeFloats(&min.x, 1);
    writeFloats(&min.y, 1);
    writeFloats(&min.z, 1);
    // float maxx, maxy, maxz
    writeFloats(&max.x, 1);
    writeFloats(&max.y, 1);
    writeFloats(&max.z, 1);
    // float radius
	Ogre::Real radius = pMesh->getBoundingSphereRadius();
    writeFloats(&radius, 1);
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readBoundsInfo(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
//	std::clog << "vl::MeshSerializerImpl::readBoundsInfo" << std::endl;
    Ogre::Vector3 min, max;
    // float minx, miny, minz
    readFloats(stream, &min.x, 1);
    readFloats(stream, &min.y, 1);
    readFloats(stream, &min.z, 1);
    // float maxx, maxy, maxz
    readFloats(stream, &max.x, 1);
    readFloats(stream, &max.y, 1);
    readFloats(stream, &max.z, 1);
    Ogre::AxisAlignedBox box(min, max);
    pMesh->setBounds(box);
    // float radius
    float radius;
    readFloats(stream, &radius, 1);
    pMesh->setBoundingSphereRadius(radius);
}

//---------------------------------------------------------------------
/*	LOD not supported
void MeshSerializerImpl::readMeshLodInfo(DataStreamPtr& stream, Mesh* pMesh)
{
	unsigned short streamID, i;

    // Read the strategy to be used for this mesh
    String strategyName = readString(stream);
    LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(strategyName);
    pMesh->setLodStrategy(strategy);

    // unsigned short numLevels;
	readShorts(stream, &(pMesh->mNumLods), 1);
    // bool manual;  (true for manual alternate meshes, false for generated)
	readBools(stream, &(pMesh->mIsLodManual), 1);

	// Preallocate submesh lod face data if not manual
	if (!pMesh->mIsLodManual)
	{
		unsigned short numsubs = pMesh->getNumSubMeshes();
		for (i = 0; i < numsubs; ++i)
		{
			SubMesh* sm = pMesh->getSubMesh(i);
			sm->mLodFaceList.resize(pMesh->mNumLods-1);
		}
	}

	// Loop from 1 rather than 0 (full detail index is not in file)
	for (i = 1; i < pMesh->mNumLods; ++i)
	{
		streamID = readChunk(stream);
		if (streamID != M_MESH_LOD_USAGE)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
				"Missing M_MESH_LOD_USAGE stream in " + pMesh->getName(),
				"MeshSerializerImpl::readMeshLodInfo");
		}
		// Read depth
		MeshLodUsage usage;
		readFloats(stream, &(usage.userValue), 1);

		if (pMesh->isLodManual())
		{
			readMeshLodUsageManual(stream, pMesh, i, usage);
		}
		else //(!pMesh->isLodManual)
		{
			readMeshLodUsageGenerated(stream, pMesh, i, usage);
		}
        usage.edgeData = NULL;

		// Save usage
		pMesh->mMeshLodUsageList.push_back(usage);
	}


}
//---------------------------------------------------------------------
void MeshSerializerImpl::readMeshLodUsageManual(DataStreamPtr& stream,
    Mesh* pMesh, unsigned short lodNum, MeshLodUsage& usage)
{
	unsigned long streamID;
	// Read detail stream
	streamID = readChunk(stream);
	if (streamID != M_MESH_LOD_MANUAL)
	{
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			"Missing M_MESH_LOD_MANUAL stream in " + pMesh->getName(),
			"MeshSerializerImpl::readMeshLodUsageManual");
	}

	usage.manualName = readString(stream);
	usage.manualMesh.setNull(); // will trigger load later
}
//---------------------------------------------------------------------
void MeshSerializerImpl::readMeshLodUsageGenerated(DataStreamPtr& stream,
    Mesh* pMesh, unsigned short lodNum, MeshLodUsage& usage)
{
	usage.manualName = "";
	usage.manualMesh.setNull();

	// Get one set of detail per SubMesh
	unsigned short numSubs, i;
	unsigned long streamID;
	numSubs = pMesh->getNumSubMeshes();
	for (i = 0; i < numSubs; ++i)
	{
		streamID = readChunk(stream);
		if (streamID != M_MESH_LOD_GENERATED)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
				"Missing M_MESH_LOD_GENERATED stream in " + pMesh->getName(),
				"MeshSerializerImpl::readMeshLodUsageGenerated");
		}

		SubMesh* sm = pMesh->getSubMesh(i);
		// lodNum - 1 because SubMesh doesn't store full detail LOD
        sm->mLodFaceList[lodNum - 1] = OGRE_NEW IndexData();
		IndexData* indexData = sm->mLodFaceList[lodNum - 1];
        // unsigned int numIndexes
        unsigned int numIndexes;
		readInts(stream, &numIndexes, 1);
        indexData->indexCount = static_cast<size_t>(numIndexes);
        // bool indexes32Bit
        bool idx32Bit;
        readBools(stream, &idx32Bit, 1);
        // int* faceIndexes;  ((v1, v2, v3) * numFaces)
        if (idx32Bit)
        {
            indexData->indexBuffer = HardwareBufferManager::getSingleton().
                createIndexBuffer(HardwareIndexBuffer::IT_32BIT, indexData->indexCount,
                pMesh->mIndexBufferUsage, pMesh->mIndexBufferShadowBuffer);
            unsigned int* pIdx = static_cast<unsigned int*>(
                indexData->indexBuffer->lock(
                    0,
                    indexData->indexBuffer->getSizeInBytes(),
                    HardwareBuffer::HBL_DISCARD) );

			readInts(stream, pIdx, indexData->indexCount);
            indexData->indexBuffer->unlock();

        }
        else
        {
            indexData->indexBuffer = HardwareBufferManager::getSingleton().
                createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexData->indexCount,
                pMesh->mIndexBufferUsage, pMesh->mIndexBufferShadowBuffer);
            unsigned short* pIdx = static_cast<unsigned short*>(
                indexData->indexBuffer->lock(
                    0,
                    indexData->indexBuffer->getSizeInBytes(),
                    HardwareBuffer::HBL_DISCARD) );
			readShorts(stream, pIdx, indexData->indexCount);
            indexData->indexBuffer->unlock();

        }

	}
}
*/
//---------------------------------------------------------------------

//void 
//vl::MeshSerializerImpl::flipFromLittleEndian(void* pData, size_t vertexCount,
//    size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
//{
	/*
	if(_flip_endian)
	{
	    flipEndian(pData, vertexCount, vertexSize, elems);
	}
	*/
//}
//---------------------------------------------------------------------
//void 
//vl::MeshSerializerImpl::flipToLittleEndian(void* pData, size_t vertexCount,
//		size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
//{
	/*
	if (_flip_endian)
	{
	    flipEndian(pData, vertexCount, vertexSize, elems);
	}
	*/
//}
//---------------------------------------------------------------------
//void 
//vl::MeshSerializerImpl::flipEndian(void* pData, size_t vertexCount,
//    size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
//{
	// Endian flipping not supported
	/*
	void *pBase = pData;
	for (size_t v = 0; v < vertexCount; ++v)
	{
		VertexDeclaration::VertexElementList::const_iterator ei, eiend;
		eiend = elems.end();
		for (ei = elems.begin(); ei != eiend; ++ei)
		{
			void *pElem;
			// re-base pointer to the element
			(*ei).baseVertexPointerToElement(pBase, &pElem);
			// Flip the endian based on the type
			size_t typeSize = 0;
			switch (VertexElement::getBaseType((*ei).getType()))
			{
				case VET_FLOAT1:
					typeSize = sizeof(float);
					break;
				case VET_SHORT1:
					typeSize = sizeof(short);
					break;
				case VET_COLOUR:
				case VET_COLOUR_ABGR:
				case VET_COLOUR_ARGB:
					typeSize = sizeof(RGBA);
					break;
				case VET_UBYTE4:
					typeSize = 0; // NO FLIPPING
					break;
				default:
					assert(false); // Should never happen
			};
            Serializer::flipEndian(pElem, typeSize,
				VertexElement::getTypeCount((*ei).getType()));

		}

		pBase = static_cast<void*>(
			static_cast<unsigned char*>(pBase) + vertexSize);

	}
	*/
//}
//---------------------------------------------------------------------
/*	Edge list not supported
size_t MeshSerializerImpl::calcEdgeListSize(const Mesh* pMesh)
{
    size_t size = STREAM_OVERHEAD_SIZE;

    for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
    {

        const EdgeData* edgeData = pMesh->getEdgeList(i);
        bool isManual = pMesh->isLodManual() && (i > 0);

        size += calcEdgeListLodSize(edgeData, isManual);

    }

    return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcEdgeListLodSize(const EdgeData* edgeData, bool isManual)
{
    size_t size = STREAM_OVERHEAD_SIZE;

    // unsigned short lodIndex
    size += sizeof(uint16);

    // bool isManual			// If manual, no edge data here, loaded from manual mesh
    size += sizeof(bool);
    if (!isManual)
    {
        // bool isClosed
        size += sizeof(bool);
        // unsigned long numTriangles
        size += sizeof(uint32);
        // unsigned long numEdgeGroups
        size += sizeof(uint32);
        // Triangle* triangleList
        size_t triSize = 0;
        // unsigned long indexSet
        // unsigned long vertexSet
        // unsigned long vertIndex[3]
        // unsigned long sharedVertIndex[3]
        // float normal[4]
        triSize += sizeof(uint32) * 8
                + sizeof(float) * 4;

        size += triSize * edgeData->triangles.size();
        // Write the groups
        for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
            gi != edgeData->edgeGroups.end(); ++gi)
        {
            const EdgeData::EdgeGroup& edgeGroup = *gi;
            size += calcEdgeGroupSize(edgeGroup);
        }

    }

    return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcEdgeGroupSize(const EdgeData::EdgeGroup& group)
{
    size_t size = STREAM_OVERHEAD_SIZE;

    // unsigned long vertexSet
    size += sizeof(uint32);
    // unsigned long triStart
    size += sizeof(uint32);
    // unsigned long triCount
    size += sizeof(uint32);
    // unsigned long numEdges
    size += sizeof(uint32);
    // Edge* edgeList
    size_t edgeSize = 0;
    // unsigned long  triIndex[2]
    // unsigned long  vertIndex[2]
    // unsigned long  sharedVertIndex[2]
    // bool degenerate
    edgeSize += sizeof(uint32) * 6 + sizeof(bool);
    size += edgeSize * group.edges.size();

    return size;
}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeEdgeList(const Mesh* pMesh)
{
    writeChunkHeader(M_EDGE_LISTS, calcEdgeListSize(pMesh));

    for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
    {
        const EdgeData* edgeData = pMesh->getEdgeList(i);
        bool isManual = pMesh->isLodManual() && (i > 0);
        writeChunkHeader(M_EDGE_LIST_LOD, calcEdgeListLodSize(edgeData, isManual));

        // unsigned short lodIndex
        writeShorts(&i, 1);

        // bool isManual			// If manual, no edge data here, loaded from manual mesh
        writeBools(&isManual, 1);
        if (!isManual)
        {
            // bool isClosed
            writeBools(&edgeData->isClosed, 1);
            // unsigned long  numTriangles
            uint32 count = static_cast<uint32>(edgeData->triangles.size());
            writeInts(&count, 1);
            // unsigned long numEdgeGroups
            count = static_cast<uint32>(edgeData->edgeGroups.size());
            writeInts(&count, 1);
            // Triangle* triangleList
            // Iterate rather than writing en-masse to allow endian conversion
            EdgeData::TriangleList::const_iterator t = edgeData->triangles.begin();
            EdgeData::TriangleFaceNormalList::const_iterator fni = edgeData->triangleFaceNormals.begin();
            for ( ; t != edgeData->triangles.end(); ++t, ++fni)
            {
                const EdgeData::Triangle& tri = *t;
                // unsigned long indexSet;
                uint32 tmp[3];
                tmp[0] = tri.indexSet;
                writeInts(tmp, 1);
                // unsigned long vertexSet;
                tmp[0] = tri.vertexSet;
                writeInts(tmp, 1);
                // unsigned long vertIndex[3];
                tmp[0] = tri.vertIndex[0];
                tmp[1] = tri.vertIndex[1];
                tmp[2] = tri.vertIndex[2];
                writeInts(tmp, 3);
                // unsigned long sharedVertIndex[3];
                tmp[0] = tri.sharedVertIndex[0];
                tmp[1] = tri.sharedVertIndex[1];
                tmp[2] = tri.sharedVertIndex[2];
                writeInts(tmp, 3);
                // float normal[4];
                writeFloats(&(fni->x), 4);

            }
            // Write the groups
            for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                gi != edgeData->edgeGroups.end(); ++gi)
            {
                const EdgeData::EdgeGroup& edgeGroup = *gi;
                writeChunkHeader(M_EDGE_GROUP, calcEdgeGroupSize(edgeGroup));
                // unsigned long vertexSet
                uint32 vertexSet = static_cast<uint32>(edgeGroup.vertexSet);
                writeInts(&vertexSet, 1);
                // unsigned long triStart
                uint32 triStart = static_cast<uint32>(edgeGroup.triStart);
                writeInts(&triStart, 1);
                // unsigned long triCount
                uint32 triCount = static_cast<uint32>(edgeGroup.triCount);
                writeInts(&triCount, 1);
                // unsigned long numEdges
                count = static_cast<uint32>(edgeGroup.edges.size());
                writeInts(&count, 1);
                // Edge* edgeList
                // Iterate rather than writing en-masse to allow endian conversion
                for (EdgeData::EdgeList::const_iterator ei = edgeGroup.edges.begin();
                    ei != edgeGroup.edges.end(); ++ei)
                {
                    const EdgeData::Edge& edge = *ei;
                    uint32 tmp[2];
                    // unsigned long  triIndex[2]
                    tmp[0] = edge.triIndex[0];
                    tmp[1] = edge.triIndex[1];
                    writeInts(tmp, 2);
                    // unsigned long  vertIndex[2]
                    tmp[0] = edge.vertIndex[0];
                    tmp[1] = edge.vertIndex[1];
                    writeInts(tmp, 2);
                    // unsigned long  sharedVertIndex[2]
                    tmp[0] = edge.sharedVertIndex[0];
                    tmp[1] = edge.sharedVertIndex[1];
                    writeInts(tmp, 2);
                    // bool degenerate
                    writeBools(&(edge.degenerate), 1);
                }

            }

        }

    }
}
*/
/// Necessary for the Dummy reader functions
#include <OGRE/OgreEdgeListBuilder.h>

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readEdgeList(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
//	std::clog << "vl::MeshSerializerImpl::readEdgeList : DUMMY" << std::endl;

	unsigned short streamID;

    if(!stream.eof())
    {
        streamID = readChunk(stream);
        while(!stream.eof() &&
            streamID == Ogre::M_EDGE_LIST_LOD)
        {
            // Process single LOD
//			std::clog <<  "vl::MeshSerializerImpl::readEdgeList : process single LOD" << std::endl;
            // unsigned short lodIndex
            unsigned short lodIndex;
            readShorts(stream, &lodIndex, 1);

            // bool isManual			// If manual, no edge data here, loaded from manual mesh
            bool isManual;
            readBools(stream, &isManual, 1);
            // Only load in non-manual levels; others will be connected up by Mesh on demand
            if (!isManual)
            {
                //Ogre::MeshLodUsage& usage = const_cast<MeshLodUsage&>(pMesh->getLodLevel(lodIndex));

                Ogre::EdgeData dummy;

                // Read detail information of the edge list
				// for the purpose of removing that data from the stream
                readEdgeListLodInfo(stream, &dummy);

                // Postprocessing edge groups
                /*
				EdgeData::EdgeGroupList::iterator egi, egend;
                egend = usage.edgeData->edgeGroups.end();
                for (egi = usage.edgeData->edgeGroups.begin(); egi != egend; ++egi)
                {
                    EdgeData::EdgeGroup& edgeGroup = *egi;
                    // Populate edgeGroup.vertexData pointers
                    // If there is shared vertex data, vertexSet 0 is that,
                    // otherwise 0 is first dedicated
                    if (pMesh->sharedVertexData)
                    {
                        if (edgeGroup.vertexSet == 0)
                        {
                            edgeGroup.vertexData = pMesh->sharedVertexData;
                        }
                        else
                        {
                            edgeGroup.vertexData = pMesh->getSubMesh(
                                (unsigned short)edgeGroup.vertexSet-1)->vertexData;
                        }
                    }
                    else
                    {
                        edgeGroup.vertexData = pMesh->getSubMesh(
                            (unsigned short)edgeGroup.vertexSet)->vertexData;
                    }
                }
				*/
            }

			if(!stream.eof())
			{
				streamID = readChunk(stream);
			}

        }
        if(!stream.eof())
        {
            // Backpedal back to start of stream
            stream.skip(-STREAM_OVERHEAD_SIZE);
        }
    }
	
	//pMesh->mEdgeListsBuilt = true;
//	std::clog << "vl::MeshSerializerImpl::readEdgeList : DONE" << std::endl;
}

//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readEdgeListLodInfo(vl::ResourceStream &stream,
    Ogre::EdgeData *edgeData)
{
//	std::clog << "vl::MeshSerializerImpl::readEdgeListLodInfo : DUMMY" << std::endl;

	assert(edgeData);

    // bool isClosed
    readBools(stream, &edgeData->isClosed, 1);
    // unsigned long numTriangles
    uint32_t numTriangles;
    readInts(stream, &numTriangles, 1);
    // Allocate correct amount of memory
    edgeData->triangles.resize(numTriangles);
    edgeData->triangleFaceNormals.resize(numTriangles);
    edgeData->triangleLightFacings.resize(numTriangles);
    // unsigned long numEdgeGroups
    uint32_t numEdgeGroups;
    readInts(stream, &numEdgeGroups, 1);
    // Allocate correct amount of memory
    edgeData->edgeGroups.resize(numEdgeGroups);
    // Triangle* triangleList
    uint32_t tmp[3];
    for (size_t t = 0; t < numTriangles; ++t)
    {
        Ogre::EdgeData::Triangle& tri = edgeData->triangles[t];
        // unsigned long indexSet
        readInts(stream, tmp, 1);
        tri.indexSet = tmp[0];
        // unsigned long vertexSet
        readInts(stream, tmp, 1);
        tri.vertexSet = tmp[0];
        // unsigned long vertIndex[3]
        readInts(stream, tmp, 3);
        tri.vertIndex[0] = tmp[0];
        tri.vertIndex[1] = tmp[1];
        tri.vertIndex[2] = tmp[2];
        // unsigned long sharedVertIndex[3]
        readInts(stream, tmp, 3);
        tri.sharedVertIndex[0] = tmp[0];
        tri.sharedVertIndex[1] = tmp[1];
        tri.sharedVertIndex[2] = tmp[2];
        // float normal[4]
        readFloats(stream, &(edgeData->triangleFaceNormals[t].x), 4);

    }

    for (uint32_t eg = 0; eg < numEdgeGroups; ++eg)
    {
        unsigned short streamID = readChunk(stream);
        if (streamID != Ogre::M_EDGE_GROUP)
        {
			BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing M_EDGE_GROUP stream"));
        }
        Ogre::EdgeData::EdgeGroup& edgeGroup = edgeData->edgeGroups[eg];

        // unsigned long vertexSet
        readInts(stream, tmp, 1);
        edgeGroup.vertexSet = tmp[0];
        // unsigned long triStart
        readInts(stream, tmp, 1);
        edgeGroup.triStart = tmp[0];
        // unsigned long triCount
        readInts(stream, tmp, 1);
        edgeGroup.triCount = tmp[0];
        // unsigned long numEdges
        uint32_t numEdges;
        readInts(stream, &numEdges, 1);
        edgeGroup.edges.resize(numEdges);
        // Edge* edgeList
        for (uint32_t e = 0; e < numEdges; ++e)
        {
            Ogre::EdgeData::Edge& edge = edgeGroup.edges[e];
            // unsigned long  triIndex[2]
            readInts(stream, tmp, 2);
            edge.triIndex[0] = tmp[0];
            edge.triIndex[1] = tmp[1];
            // unsigned long  vertIndex[2]
            readInts(stream, tmp, 2);
            edge.vertIndex[0] = tmp[0];
            edge.vertIndex[1] = tmp[1];
            // unsigned long  sharedVertIndex[2]
            readInts(stream, tmp, 2);
            edge.sharedVertIndex[0] = tmp[0];
            edge.sharedVertIndex[1] = tmp[1];
            // bool degenerate
            readBools(stream, &(edge.degenerate), 1);
        }
	}
//	std::clog << "vl::MeshSerializerImpl::readEdgeListLodInfo : DONE" << std::endl;
}

//---------------------------------------------------------------------
/*	Animation not supported
size_t MeshSerializerImpl::calcAnimationsSize(const Mesh* pMesh)
{
	size_t size = STREAM_OVERHEAD_SIZE;

	for (unsigned short a = 0; a < pMesh->getNumAnimations(); ++a)
	{
		Animation* anim = pMesh->getAnimation(a);
		size += calcAnimationSize(anim);
	}
	return size;

}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcAnimationSize(const Animation* anim)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// char* name
	size += anim->getName().length() + 1;

	// float length
	size += sizeof(float);

	Animation::VertexTrackIterator trackIt = anim->getVertexTrackIterator();
	while (trackIt.hasMoreElements())
	{
		VertexAnimationTrack* vt = trackIt.getNext();
		size += calcAnimationTrackSize(vt);
	}

	return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcAnimationTrackSize(const VertexAnimationTrack* track)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// uint16 type
	size += sizeof(uint16);
	// unsigned short target		// 0 for shared geometry,
	size += sizeof(unsigned short);

	if (track->getAnimationType() == VAT_MORPH)
	{
		for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
		{
			VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(i);
			size += calcMorphKeyframeSize(kf, track->getAssociatedVertexData()->vertexCount);
		}
	}
	else
	{
		for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
		{
			VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(i);
			size += calcPoseKeyframeSize(kf);
		}
	}
	return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcMorphKeyframeSize(const VertexMorphKeyFrame* kf,
	size_t vertexCount)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// float time
	size += sizeof(float);
	// float x,y,z[,nx,ny,nz]
	bool includesNormals = kf->getVertexBuffer()->getVertexSize() > (sizeof(float) * 3);
	size += sizeof(float) * (includesNormals ? 6 : 3) * vertexCount;

	return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcPoseKeyframeSize(const VertexPoseKeyFrame* kf)
{
	size_t size = STREAM_OVERHEAD_SIZE;

	// float time
	size += sizeof(float);

	size += calcPoseKeyframePoseRefSize() * kf->getPoseReferences().size();

	return size;

}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcPoseKeyframePoseRefSize(void)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// unsigned short poseIndex
	size += sizeof(uint16);
	// float influence
	size += sizeof(float);

	return size;

}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcPosesSize(const Mesh* pMesh)
{
	size_t size = STREAM_OVERHEAD_SIZE;

	Mesh::ConstPoseIterator poseIt = pMesh->getPoseIterator();
	while (poseIt.hasMoreElements())
	{
		size += calcPoseSize(poseIt.getNext());
	}
	return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcPoseSize(const Pose* pose)
{
	size_t size = STREAM_OVERHEAD_SIZE;

	// char* name (may be blank)
	size += pose->getName().length() + 1;
	// unsigned short target
	size += sizeof(uint16);
	// bool includesNormals
	size += sizeof(bool);

	// vertex offsets
	size += pose->getVertexOffsets().size() * calcPoseVertexSize(pose);

	return size;

}
//---------------------------------------------------------------------
size_t MeshSerializerImpl::calcPoseVertexSize(const Pose* pose)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// unsigned long vertexIndex
	size += sizeof(uint32);
	// float xoffset, yoffset, zoffset
	size += sizeof(float) * 3;
	// optional normals
	if (!pose->getNormals().empty())
		size += sizeof(float) * 3;

	return size;
}
//---------------------------------------------------------------------
void MeshSerializerImpl::writePoses(const Mesh* pMesh)
{
	Mesh::ConstPoseIterator poseIterator = pMesh->getPoseIterator();
	if (poseIterator.hasMoreElements())
	{
		writeChunkHeader(M_POSES, calcPosesSize(pMesh));
		while (poseIterator.hasMoreElements())
		{
			writePose(poseIterator.getNext());
		}
	}

}
//---------------------------------------------------------------------
void MeshSerializerImpl::writePose(const Pose* pose)
{
	writeChunkHeader(M_POSE, calcPoseSize(pose));

	// char* name (may be blank)
	writeString(pose->getName());

	// unsigned short target
	ushort val = pose->getTarget();
	writeShorts(&val, 1);
		
	// bool includesNormals
	bool includesNormals = !pose->getNormals().empty();
	writeBools(&includesNormals, 1);

	size_t vertexSize = calcPoseVertexSize(pose);
	Pose::ConstVertexOffsetIterator vit = pose->getVertexOffsetIterator();
	Pose::ConstNormalsIterator nit = pose->getNormalsIterator();
	while (vit.hasMoreElements())
	{
		uint32 vertexIndex = (uint32)vit.peekNextKey();
		Vector3 offset = vit.getNext();
		writeChunkHeader(M_POSE_VERTEX, vertexSize);
		// unsigned long vertexIndex
		writeInts(&vertexIndex, 1);
		// float xoffset, yoffset, zoffset
		writeFloats(offset.ptr(), 3);
		if (includesNormals)
		{
			Vector3 normal = nit.getNext();
			// float xnormal, ynormal, znormal
			writeFloats(normal.ptr(), 3);
		}
	}


}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeAnimations(const Mesh* pMesh)
{
	writeChunkHeader(M_ANIMATIONS, calcAnimationsSize(pMesh));

	for (unsigned short a = 0; a < pMesh->getNumAnimations(); ++a)
	{
		Animation* anim = pMesh->getAnimation(a);
		LogManager::getSingleton().logMessage("Exporting animation " + anim->getName());
		writeAnimation(anim);
		LogManager::getSingleton().logMessage("Animation exported.");
	}
}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeAnimation(const Animation* anim)
{
	writeChunkHeader(M_ANIMATION, calcAnimationSize(anim));
	// char* name
	writeString(anim->getName());
	// float length
	float len = anim->getLength();
	writeFloats(&len, 1);
	Animation::VertexTrackIterator trackIt = anim->getVertexTrackIterator();
	while (trackIt.hasMoreElements())
	{
		VertexAnimationTrack* vt = trackIt.getNext();
		writeAnimationTrack(vt);
	}


}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeAnimationTrack(const VertexAnimationTrack* track)
{
	writeChunkHeader(M_ANIMATION_TRACK, calcAnimationTrackSize(track));
	// unsigned short type			// 1 == morph, 2 == pose
	uint16 animType = (uint16)track->getAnimationType();
	writeShorts(&animType, 1);
	// unsigned short target
	uint16 target = track->getHandle();
	writeShorts(&target, 1);

	if (track->getAnimationType() == VAT_MORPH)
	{
		for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
		{
			VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(i);
			writeMorphKeyframe(kf, track->getAssociatedVertexData()->vertexCount);
		}
	}
	else // VAT_POSE
	{
		for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
		{
			VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(i);
			writePoseKeyframe(kf);
		}
	}

}
//---------------------------------------------------------------------
void MeshSerializerImpl::writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount)
{
	writeChunkHeader(M_ANIMATION_MORPH_KEYFRAME, calcMorphKeyframeSize(kf, vertexCount));
	// float time
	float timePos = kf->getTime();
	writeFloats(&timePos, 1);
	// bool includeNormals
	bool includeNormals = kf->getVertexBuffer()->getVertexSize() > (sizeof(float) * 3);
	writeBools(&includeNormals, 1);
	// float x,y,z			// repeat by number of vertices in original geometry
	float* pSrc = static_cast<float*>(
		kf->getVertexBuffer()->lock(HardwareBuffer::HBL_READ_ONLY));
	writeFloats(pSrc, vertexCount * (includeNormals ? 6 : 3));
	kf->getVertexBuffer()->unlock();
}
//---------------------------------------------------------------------
void MeshSerializerImpl::writePoseKeyframe(const VertexPoseKeyFrame* kf)
{
	writeChunkHeader(M_ANIMATION_POSE_KEYFRAME, calcPoseKeyframeSize(kf));
	// float time
	float timePos = kf->getTime();
	writeFloats(&timePos, 1);

	// pose references
	VertexPoseKeyFrame::ConstPoseRefIterator poseRefIt =
		kf->getPoseReferenceIterator();
	while (poseRefIt.hasMoreElements())
	{
		writePoseKeyframePoseRef(poseRefIt.getNext());
	}



}
//---------------------------------------------------------------------
void MeshSerializerImpl::writePoseKeyframePoseRef(
	const VertexPoseKeyFrame::PoseRef& poseRef)
{
	writeChunkHeader(M_ANIMATION_POSE_REF, calcPoseKeyframePoseRefSize());
	// unsigned short poseIndex
	writeShorts(&(poseRef.poseIndex), 1);
	// float influence
	writeFloats(&(poseRef.influence), 1);
}
//---------------------------------------------------------------------
void MeshSerializerImpl::readPoses(DataStreamPtr& stream, Mesh* pMesh)
{
	unsigned short streamID;

	// Find all substreams
	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			(streamID == M_POSE))
		{
			switch(streamID)
			{
			case M_POSE:
				readPose(stream, pMesh);
				break;

			}

			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}
}
//---------------------------------------------------------------------
void MeshSerializerImpl::readPose(DataStreamPtr& stream, Mesh* pMesh)
{
	// char* name (may be blank)
	String name = readString(stream);
	// unsigned short target
	unsigned short target;
	readShorts(stream, &target, 1);

	// bool includesNormals
	bool includesNormals;
	readBools(stream, &includesNormals, 1);
		
	Pose* pose = pMesh->createPose(target, name);

	// Find all substreams
	unsigned short streamID;
	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			(streamID == M_POSE_VERTEX))
		{
			switch(streamID)
			{
			case M_POSE_VERTEX:
				// create vertex offset
				uint32 vertIndex;
				Vector3 offset, normal;
				// unsigned long vertexIndex
				readInts(stream, &vertIndex, 1);
				// float xoffset, yoffset, zoffset
				readFloats(stream, offset.ptr(), 3);
					
				if (includesNormals)
				{
					readFloats(stream, normal.ptr(), 3);
					pose->addVertex(vertIndex, offset, normal);						
				}
				else 
				{
					pose->addVertex(vertIndex, offset);
				}


				break;

			}

			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}

}
//---------------------------------------------------------------------
void MeshSerializerImpl::readAnimations(DataStreamPtr& stream, Mesh* pMesh)
{
	unsigned short streamID;

	// Find all substreams
	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			(streamID == M_ANIMATION))
		{
			switch(streamID)
			{
			case M_ANIMATION:
				readAnimation(stream, pMesh);
				break;

			}

			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}


}
//---------------------------------------------------------------------
void MeshSerializerImpl::readAnimation(DataStreamPtr& stream, Mesh* pMesh)
{

	// char* name
	String name = readString(stream);
	// float length
	float len;
	readFloats(stream, &len, 1);

	Animation* anim = pMesh->createAnimation(name, len);

	// tracks
	unsigned short streamID;

	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			streamID == M_ANIMATION_TRACK)
		{
			switch(streamID)
			{
			case M_ANIMATION_TRACK:
				readAnimationTrack(stream, anim, pMesh);
				break;
			};
			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}
}
//---------------------------------------------------------------------
void MeshSerializerImpl::readAnimationTrack(DataStreamPtr& stream,
	Animation* anim, Mesh* pMesh)
{
	// ushort type
	uint16 inAnimType;
	readShorts(stream, &inAnimType, 1);
	VertexAnimationType animType = (VertexAnimationType)inAnimType;

	// unsigned short target
	uint16 target;
	readShorts(stream, &target, 1);

	VertexAnimationTrack* track = anim->createVertexTrack(target,
		pMesh->getVertexDataByTrackHandle(target), animType);

	// keyframes
	unsigned short streamID;

	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			(streamID == M_ANIMATION_MORPH_KEYFRAME ||
				streamID == M_ANIMATION_POSE_KEYFRAME))
		{
			switch(streamID)
			{
			case M_ANIMATION_MORPH_KEYFRAME:
				readMorphKeyFrame(stream, track);
				break;
			case M_ANIMATION_POSE_KEYFRAME:
				readPoseKeyFrame(stream, track);
				break;
			};
			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}

}
//---------------------------------------------------------------------
void MeshSerializerImpl::readMorphKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track)
{
	// float time
	float timePos;
	readFloats(stream, &timePos, 1);
		
	// bool includesNormals
	bool includesNormals;
	readBools(stream, &includesNormals, 1);

	VertexMorphKeyFrame* kf = track->createVertexMorphKeyFrame(timePos);

	// Create buffer, allow read and use shadow buffer
	size_t vertexCount = track->getAssociatedVertexData()->vertexCount;
	size_t vertexSize = sizeof(float) * (includesNormals ? 6 : 3);
	HardwareVertexBufferSharedPtr vbuf =
		HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexSize, vertexCount,
			HardwareBuffer::HBU_STATIC, true);
	// float x,y,z			// repeat by number of vertices in original geometry
	float* pDst = static_cast<float*>(
		vbuf->lock(HardwareBuffer::HBL_DISCARD));
	readFloats(stream, pDst, vertexCount * (includesNormals ? 6 : 3));
	vbuf->unlock();
	kf->setVertexBuffer(vbuf);

}
//---------------------------------------------------------------------
void MeshSerializerImpl::readPoseKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track)
{
	// float time
	float timePos;
	readFloats(stream, &timePos, 1);

	// Create keyframe
	VertexPoseKeyFrame* kf = track->createVertexPoseKeyFrame(timePos);

	unsigned short streamID;

	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			streamID == M_ANIMATION_POSE_REF)
		{
			switch(streamID)
			{
			case M_ANIMATION_POSE_REF:
				uint16 poseIndex;
				float influence;
				// unsigned short poseIndex
				readShorts(stream, &poseIndex, 1);
				// float influence
				readFloats(stream, &influence, 1);

				kf->addPoseReference(poseIndex, influence);

				break;
			};
			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}

}
*/
//---------------------------------------------------------------------
void 
vl::MeshSerializerImpl::readExtremes(vl::ResourceStream &stream, vl::Mesh *pMesh)
{
	std::clog << "vl::MeshSerializerImpl::readExtremes : NOT IMPLEMENTED." << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/*
	unsigned short idx;
	readShorts(stream, &idx, 1);

	SubMesh *sm = pMesh->getSubMesh(idx);
		
	int n_floats = (mCurrentstreamLen - STREAM_OVERHEAD_SIZE -
					sizeof (unsigned short)) / sizeof (float);
		
	assert((n_floats % 3) == 0);
		
	float *vert = new float[n_floats];
	readFloats(stream, vert, n_floats);
		
	for (int i = 0; i < n_floats; i += 3)
		sm->extremityPoints.push_back(Ogre::Vector3(vert [i], vert [i + 1], vert [i + 2]));

	delete [] vert;
	*/
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
vl::MeshSerializerImpl_v1_41::MeshSerializerImpl_v1_41()
{
    // Version number
    mVersion = "[MeshSerializer_v1.41]";
}
//---------------------------------------------------------------------
vl::MeshSerializerImpl_v1_41::~MeshSerializerImpl_v1_41()
{
}

/*	Animation not supported
//---------------------------------------------------------------------
void MeshSerializerImpl_v1_41::writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount)
{
	writeChunkHeader(M_ANIMATION_MORPH_KEYFRAME, calcMorphKeyframeSize(kf, vertexCount));
	// float time
	float timePos = kf->getTime();
	writeFloats(&timePos, 1);
	// float x,y,z			// repeat by number of vertices in original geometry
	float* pSrc = static_cast<float*>(
		kf->getVertexBuffer()->lock(HardwareBuffer::HBL_READ_ONLY));
	writeFloats(pSrc, vertexCount * 3);
	kf->getVertexBuffer()->unlock();
}
//---------------------------------------------------------------------
void MeshSerializerImpl_v1_41::readMorphKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track)
{
	// float time
	float timePos;
	readFloats(stream, &timePos, 1);

	VertexMorphKeyFrame* kf = track->createVertexMorphKeyFrame(timePos);

	// Create buffer, allow read and use shadow buffer
	size_t vertexCount = track->getAssociatedVertexData()->vertexCount;
	HardwareVertexBufferSharedPtr vbuf =
		HardwareBufferManager::getSingleton().createVertexBuffer(
			VertexElement::getTypeSize(VET_FLOAT3), vertexCount,
			HardwareBuffer::HBU_STATIC, true);
	// float x,y,z			// repeat by number of vertices in original geometry
	float* pDst = static_cast<float*>(
		vbuf->lock(HardwareBuffer::HBL_DISCARD));
	readFloats(stream, pDst, vertexCount * 3);
	vbuf->unlock();
	kf->setVertexBuffer(vbuf);
}
//---------------------------------------------------------------------
void MeshSerializerImpl_v1_41::writePose(const Pose* pose)
{
	writeChunkHeader(M_POSE, calcPoseSize(pose));

	// char* name (may be blank)
	writeString(pose->getName());

	// unsigned short target
	ushort val = pose->getTarget();
	writeShorts(&val, 1);

	size_t vertexSize = calcPoseVertexSize();
	Pose::ConstVertexOffsetIterator vit = pose->getVertexOffsetIterator();
	while (vit.hasMoreElements())
	{
		uint32 vertexIndex = (uint32)vit.peekNextKey();
		Vector3 offset = vit.getNext();
		writeChunkHeader(M_POSE_VERTEX, vertexSize);
		// unsigned long vertexIndex
		writeInts(&vertexIndex, 1);
		// float xoffset, yoffset, zoffset
		writeFloats(offset.ptr(), 3);
	}
}
//---------------------------------------------------------------------
void MeshSerializerImpl_v1_41::readPose(DataStreamPtr& stream, Mesh* pMesh)
{
	// char* name (may be blank)
	String name = readString(stream);
	// unsigned short target
	unsigned short target;
	readShorts(stream, &target, 1);

	Pose* pose = pMesh->createPose(target, name);

	// Find all substreams
	unsigned short streamID;
	if (!stream->eof())
	{
		streamID = readChunk(stream);
		while(!stream->eof() &&
			(streamID == M_POSE_VERTEX))
		{
			switch(streamID)
			{
			case M_POSE_VERTEX:
				// create vertex offset
				uint32 vertIndex;
				Vector3 offset;
				// unsigned long vertexIndex
				readInts(stream, &vertIndex, 1);
				// float xoffset, yoffset, zoffset
				readFloats(stream, offset.ptr(), 3);

				pose->addVertex(vertIndex, offset);
				break;

			}

			if (!stream->eof())
			{
				streamID = readChunk(stream);
			}

		}
		if (!stream->eof())
		{
			// Backpedal back to start of stream
			stream->skip(-STREAM_OVERHEAD_SIZE);
		}
	}
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl_v1_41::calcPoseSize(const Pose* pose)
{
	size_t size = STREAM_OVERHEAD_SIZE;

	// char* name (may be blank)
	size += pose->getName().length() + 1;
	// unsigned short target
	size += sizeof(uint16);

	// vertex offsets
	size += pose->getVertexOffsets().size() * calcPoseVertexSize();

	return size;

}
//---------------------------------------------------------------------
size_t MeshSerializerImpl_v1_41::calcPoseVertexSize(void)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// unsigned long vertexIndex
	size += sizeof(uint32);
	// float xoffset, yoffset, zoffset
	size += sizeof(float) * 3;

	return size;
}
//---------------------------------------------------------------------
size_t MeshSerializerImpl_v1_41::calcMorphKeyframeSize(const VertexMorphKeyFrame* kf,
	size_t vertexCount)
{
	size_t size = STREAM_OVERHEAD_SIZE;
	// float time
	size += sizeof(float);
	// float x,y,z
	size += sizeof(float) * 3 * vertexCount;

	return size;
}
*/
