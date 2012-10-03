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
 *
 *	Purpose:
 *		Loading of meshes into software constructs that can be converted to different libraries
 *		Loading meshes in platforms without 3d Hardware
 *		Loading meshes into format that can be distributed
 *
 *	Modifications:
 *		removal of hardware buffers, use software constructs
 *		removal of Ogre Managers, do not use any Singletons
 *		Minimal dependencies to Ogre, preferably only headers
 *
 *	@todo Mesh writing is not supported
 *	@todo Lots of Vertex attributes are not supported, Blend weights, Binormals etc.
 *	@todo almost all of Vertex attribute types are not supported
 *	@todo unsupported features will throw, so useless for general consumption
 *	@todo background loading is not supported
 */

#ifndef HYDRA_MESH_SERIALIZER_IMPL_HPP
#define HYDRA_MESH_SERIALIZER_IMPL_HPP

#include <stdint.h>

#include "mesh.hpp"
#include "typedefs.hpp"
#include "resource.hpp"

/// Base class
#include "serializer.hpp"

namespace vl {

/** Internal implementation of Mesh reading / writing for the latest version of the
.mesh format.
@remarks
In order to maintain compatibility with older versions of the .mesh format, there
will be alternative subclasses of this class to load older versions, whilst this class
will remain to load the latest version.

	@note
	This mesh format was used from Ogre v1.8.

*/
class MeshSerializerImpl : public Serializer
{
public:
    MeshSerializerImpl();
    virtual ~MeshSerializerImpl(void);

    /** Exports a mesh to the file specified. 
    @remarks
    This method takes an externally created Mesh object, and exports both it
    and optionally the Materials it uses to a .mesh file.
    @param pMesh Pointer to the Mesh to export
    @param stream The destination stream
	@param endianMode The endian mode for the written file
	@todo add DataStreams for writing, instead of Memory fields
    */
    void exportMesh(const Mesh* pMesh, vl::ResourceStream &stream,
		Endian endianMode = ENDIAN_NATIVE);

    /** Imports Mesh and (optionally) Material data from a .mesh file DataStream.
    @remarks
    This method imports data from a DataStream opened from a .mesh file and places it's
    contents into the Mesh object which is passed in. 
    @param stream The DataStream holding the .mesh data. Must be initialised (pos at the start of the buffer).
    @param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
	@todo add DataStreams for reading instead of memory fields
	@todo add callbacks listener
    */
    void importMesh(vl::ResourceStream &stream, vl::Mesh *pDest);

protected:

	// Internal methods
	/// @todo Writing meshes is not yet supported
	virtual void writeSubMeshNameTable(Mesh const *pMesh);
	virtual void writeMesh(Mesh const *pMesh);
	virtual void writeSubMesh(SubMesh const *s);
	virtual void writeSubMeshOperation(SubMesh const *s);
	virtual void writeSubMeshTextureAliases(SubMesh const *s);
	virtual void writeGeometry(Mesh const *pMesh, VertexData const *pSrc);
	// Skeletons not supported, only stubs provided
	virtual void writeSkeletonLink(std::string const &skelName);
	virtual void writeMeshBoneAssignment(VertexBoneAssignment const &assign);
	virtual void writeSubMeshBoneAssignment(VertexBoneAssignment const &assign);

	/* LOD not supported
	virtual void writeLodInfo(const Mesh* pMesh);
	virtual void writeLodSummary(unsigned short numLevels, bool manual, const LodStrategy *strategy);
	virtual void writeLodUsageManual(const MeshLodUsage& usage);
	virtual void writeLodUsageGenerated(const Mesh* pMesh, const MeshLodUsage& usage, unsigned short lodNum);
	*/
	virtual void writeBoundsInfo(Mesh const *pMesh);
	// Edge list not supported
	//virtual void writeEdgeList(const Mesh* pMesh);
	/* Animations not supported
	virtual void writeAnimations(const Mesh* pMesh);
	virtual void writeAnimation(const Animation* anim);
	virtual void writePoses(const Mesh* pMesh);
	virtual void writePose(const Pose* pose);
	virtual void writeAnimationTrack(const VertexAnimationTrack* track);
	virtual void writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount);
	virtual void writePoseKeyframe(const VertexPoseKeyFrame* kf);
	virtual void writePoseKeyframePoseRef(const VertexPoseKeyFrame::PoseRef& poseRef);
	*/
	virtual void writeExtremes(Mesh const *pMesh);
	virtual void writeSubMeshExtremes(unsigned short idx, SubMesh const *s);

	virtual size_t calcMeshSize(Mesh const *pMesh);
	virtual size_t calcSubMeshSize(SubMesh const *pSub);
	virtual size_t calcGeometrySize(VertexData const *vertexData);
	virtual size_t calcSkeletonLinkSize(std::string const &skelName);
	virtual size_t calcBoneAssignmentSize(void);
	virtual size_t calcSubMeshOperationSize(const SubMesh* pSub);
	virtual size_t calcSubMeshNameTableSize(const Mesh* pMesh);
	/* Edge lists not supported
	virtual size_t calcEdgeListSize(const Mesh* pMesh);
	virtual size_t calcEdgeListLodSize(const EdgeData* data, bool isManual);
	virtual size_t calcEdgeGroupSize(const EdgeData::EdgeGroup& group);
	*/
	/*	Animations not supported
	virtual size_t calcPosesSize(const Mesh* pMesh);
	virtual size_t calcPoseSize(const Pose* pose);
	virtual size_t calcAnimationsSize(const Mesh* pMesh);
	virtual size_t calcAnimationSize(const Animation* anim);
	virtual size_t calcAnimationTrackSize(const VertexAnimationTrack* track);
	virtual size_t calcMorphKeyframeSize(const VertexMorphKeyFrame* kf, size_t vertexCount);
	virtual size_t calcPoseKeyframeSize(const VertexPoseKeyFrame* kf);
	virtual size_t calcPoseKeyframePoseRefSize(void);
	virtual size_t calcPoseVertexSize(const Pose* pose);
	*/
	virtual size_t calcSubMeshTextureAliasesSize(const SubMesh* pSub);


	// Materials in the Mesh are not supported
	//virtual void readTextureLayer(vl::Resource &resource, Mesh* pMesh, MaterialPtr& pMat);
	virtual void readSubMeshNameTable(vl::ResourceStream &stream, Mesh* pMesh);
	virtual void readMesh(vl::ResourceStream &stream, Mesh* pMesh);
	virtual void readSubMesh(vl::ResourceStream &stream, Mesh* pMesh);
	virtual void readSubMeshOperation(vl::ResourceStream &stream, SubMesh* sub);
	virtual void readSubMeshTextureAlias(vl::ResourceStream &stream, SubMesh* sub);
	virtual void readGeometry(vl::ResourceStream &stream, Mesh* pMesh, VertexData *pDest);
	virtual void readGeometryVertexDeclaration(vl::ResourceStream &stream, Mesh* pMesh, VertexData *pDest);
	virtual void readGeometryVertexElement(vl::ResourceStream &stream, Mesh* pMesh, VertexData *pDest);
	virtual void readGeometryVertexBuffer(vl::ResourceStream &stream, Mesh* pMesh, VertexData *pDest, size_t vertexCount);

	/// Skeleton not supported, only stubs provided
	virtual void readSkeletonLink(vl::ResourceStream &stream, Mesh* pMesh);
	virtual void readMeshBoneAssignment(vl::ResourceStream &stream, Mesh* pMesh);
	virtual void readSubMeshBoneAssignment(vl::ResourceStream &stream, Mesh* pMesh, 
		SubMesh* sub);

	/* LOD not supported
	virtual void readMeshLodInfo(vl::ResourceStream &stream, Mesh* pMesh);
	virtual void readMeshLodUsageManual(vl::ResourceStream &stream, Mesh* pMesh, 
		unsigned short lodNum, MeshLodUsage& usage);
	virtual void readMeshLodUsageGenerated(vl::ResourceStream &stream, Mesh* pMesh, 
		unsigned short lodNum, MeshLodUsage& usage);
	*/
	virtual void readBoundsInfo(vl::ResourceStream &stream, Mesh *pMesh);
	/// @todo Dummy functions
	virtual void readEdgeList(vl::ResourceStream &stream, vl::Mesh *pMesh);
	virtual void readEdgeListLodInfo(vl::ResourceStream &stream, Ogre::EdgeData *edgeData);
	/* Animation not supported
	virtual void readPoses(vl::Resource &resource, Mesh* pMesh);
	virtual void readPose(vl::Resource &resource, Mesh* pMesh);
	virtual void readAnimations(vl::Resource &resource, Mesh* pMesh);
	virtual void readAnimation(vl::Resource &resource, Mesh* pMesh);
	virtual void readAnimationTrack(vl::Resource &resource, Animation* anim, Mesh* pMesh);
	virtual void readMorphKeyFrame(vl::Resource &resource, VertexAnimationTrack* track);
	virtual void readPoseKeyFrame(vl::Resource &resource, VertexAnimationTrack* track);
	*/
	virtual void readExtremes(vl::ResourceStream &stream, Mesh *pMesh);
	
	std::string mVersion;

};	// class MeshSerializerImpl

}	// namespace vl

#endif	// HYDRA_MESH_SERIALIZER_IMPL_HPP
