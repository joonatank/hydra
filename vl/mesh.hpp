/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
 *	Mesh data structure, no real functionality but can be copied to
 *	Ogre and/or Bullet as the user sees fit.
 *	Also used by the serialization modules used by editor and exporters.
 *
 *	Minamal wrappers for Ogre Mesh so that we don't need Ogre to read, write
 *	or modify these structures.
 */

#ifndef HYDRA_MESH_HPP
#define HYDRA_MESH_HPP

#include <vector>
#include <string>

#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreAxisAlignedBox.h>

/// Necessary for Ref ptrs
#include "typedefs.hpp"
/// Necessary for serializing Meshes
#include "cluster/message.hpp"

// Necessary for Ogre enums
//#include <OGRE/OgreMesh.h>
#include <OGRE/OgreRenderOperation.h>

namespace vl
{

/// @todo the parameters in the vertex should be optional
/// for example we could use an array with offsets (similar to VertexData), hard to use
/// or we could use a stack of Vertex attributes
/// or we could use multiple Vertex structures
///
/// At the moment best approach seems to be to use vertexSemantics to map different attributes
/// like position, normal, uv, tangent to float arrays
/// vertexType would indicate the size of the float array (or the amount useful data in it)
/// Provide an interface to the vertex with setPosition and so on
/// Probably  best to use just a single floating point array, size determined by
/// VertexDeclaration and really a static array for every vertex if possible.
/// Oh it's not possible to create variable size array on the stack
/// so we should probably do a similar implementation as in Ogre using a single
/// VertexData array in Mesh/SubMesh and just provide an interface for that.
/// The upside for using single dynamic array is that the amount of costly allocation
/// calls on the head is minimized. Also we can calculate the Mesh size first when
/// the declaration is made so that we only need single allocation call.
struct Vertex
{
	Ogre::Vector3 position;
	Ogre::Vector3 normal;
	Ogre::Vector3 tangent;
	Ogre::ColourValue diffuse;
	Ogre::ColourValue specular;
	Ogre::Vector2 uv;
};

std::ostream &operator<<( std::ostream &os, Vertex const &v );

/* VertexElementSematic
VES_POSITION 	Position, 3 reals per vertex.

VES_BLEND_WEIGHTS 	Blending weights.

VES_BLEND_INDICES 	Blending indices.

VES_NORMAL 	Normal, 3 reals per vertex.

VES_DIFFUSE 	Diffuse colours.

VES_SPECULAR 	Specular colours.

VES_TEXTURE_COORDINATES 	Texture coordinates.

VES_BINORMAL 	Binormal (Y axis if normal is Z)

VES_TANGENT 
*/

/* VertexElementType
VET_FLOAT1 	
VET_FLOAT2 	
VET_FLOAT3 	
VET_FLOAT4 	
VET_COLOUR 	alias to more specific colour type - use the current rendersystem's colour packing

VET_SHORT1 	
VET_SHORT2 	
VET_SHORT3 	
VET_SHORT4 	
VET_UBYTE4 	
VET_COLOUR_ARGB 	D3D style compact colour.

VET_COLOUR_ABGR 	GL style compact colour.
*/

struct VertexDeclaration
{
	VertexDeclaration(void)
	{}

	size_t vertexSize(void) const;

	typedef std::pair<Ogre::VertexElementSemantic, Ogre::VertexElementType> Semantic;

	void addSemantic(Ogre::VertexElementSemantic semantic, Ogre::VertexElementType type)
	{
		_semantics.push_back(std::make_pair(semantic, type));
	}

	std::vector<Semantic> const &getSemantics(void) const
	{ return _semantics; }

	std::vector<Semantic> &getSemantics(void)
	{ return _semantics; }

	Semantic getSemantic(size_t i) const
	{ return _semantics.at(i); }

	size_t getNSemantics(void) const
	{ return _semantics.size(); }

	/// @todo not sure if these are correct
	static size_t getTypeSize(Ogre::VertexElementType type);

private :
	std::vector<Semantic> _semantics;

};

/// @class VertexData
/// @todo Replace Vertex structure with a buffer where VertexElememt arrays
/// are continous memory segments.
/// for example [positions | normals | uvs | tangents]
/// For now we can use a structure where every VertexElement has a
/// different array, so it's easier to resize it.
struct VertexData
{
	typedef std::vector<Vertex> VertexList;

	void setVertex(size_t i, char const *buf, size_t size);

	void addVertex(Vertex const &vertex)
	{ _vertices.push_back(vertex); }

	Vertex const &getVertex(size_t i) const
	{ return _vertices.at(i); }

	Vertex &getVertex(size_t i)
	{ return _vertices.at(i); }

	void setNVertices(size_t n)
	{ _vertices.resize(n); }

	size_t getNVertices(void) const
	{ return _vertices.size(); }

	VertexList &getVertices(void)
	{ return _vertices; }

	VertexList const &getVertices(void) const
	{ return _vertices; }

	VertexList _vertices;

	VertexDeclaration vertexDeclaration;

};	// struct VertexData

/// Stub
struct VertexBoneAssignment
{
};

enum INDEX_SIZE
{
	IT_16BIT,
	IT_32BIT,
};

class IndexBuffer
{
public :
	IndexBuffer(void);

	~IndexBuffer(void);

	/// @todo these should return pointers to the correct data strucures
	/// i.e. for 16-bit indices they should be uint16_t and for
	/// 32-bit uint32_t. Just plain pointers are fine.
	uint16_t const *getBuffer16(void) const
	{ return &_buffer_16[0]; }

	uint16_t *getBuffer16(void)
	{ return &_buffer_16[0]; }
	
	uint32_t const *getBuffer32(void) const
	{ return &_buffer_32[0]; }

	uint32_t *getBuffer32(void)
	{ return &_buffer_32[0]; }
	
	std::vector<uint16_t> const &getVec16(void) const
	{ return _buffer_16; }

	std::vector<uint16_t> &getVec16(void)
	{ return _buffer_16; }

	std::vector<uint32_t> const &getVec32(void) const
	{ return _buffer_32; }

	std::vector<uint32_t> &getVec32(void)
	{ return _buffer_32; }

	/// @todo this should change the current buffer automatically to 16-bit or 32-bit
	/// depending on the count
	void setIndexCount(size_t count);

	size_t indexCount(void) const;

	INDEX_SIZE getIndexSize(void) const
	{ return _buffer_size; }

	/// @todo resetting the index size should invalidate the index buffer
	/// or resize all the indices in it.
	/// this will reset the buffer
	/// @todo after adding the automatic buffer change this can be removed
	void setIndexSize(INDEX_SIZE size);

	void push_back(uint16_t index);

	void push_back(uint32_t index);

	void set(size_t i, uint32_t index);

	void set(size_t i, uint16_t index);

	// can't define operator[] 
	// because the only difference for overloads can't be the return type

	/// @brief Calculate the index count from the 
	//void recalculateIndexCount(void)
private :
	void _resize_buffer(size_t size);

	INDEX_SIZE _buffer_size;

	std::vector<uint16_t> _buffer_16;
	std::vector<uint32_t> _buffer_32;

};	// class IndexBuffer

/// Wrappers for Ogre::Mesh for writing and reading them
/// @class SubMesh
/// @todo add merge, merges SubMeshes and uses the material of the first one
/// @todo add getPolygonCount
/// no support for quads because they are ambigious and Ogre does not support them
class SubMesh
{
public :
	SubMesh(void)
		: operationType(Ogre::RenderOperation::OT_TRIANGLE_LIST)
		, useSharedGeometry(true)
		, vertexData(0)
	{}

	~SubMesh(void) {}

	std::string const &getName(void) const
	{ return _name; }

	void setName(std::string const &name)
	{ _name = name; }
	
	std::string const &getMaterial(void) const
	{ return _material; }

	void setMaterial(std::string const &material)
	{ _material = material; }

	void allocateFaces(size_t n_faces);

	size_t getNumFaces(void) const;

	void setFace(size_t face, uint32_t  i1, uint32_t  i2, uint32_t i3);

	void addFace(uint32_t i1, uint32_t i2, uint32_t i3);

	/// --------------- Public Data --------------------
	Ogre::RenderOperation::OperationType operationType;

	bool useSharedGeometry;

	VertexData *vertexData;

	IndexBuffer indexData;

private :
	// Non-copyable
	SubMesh(SubMesh const &);
	SubMesh &operator=(SubMesh const &);

	/// --------------- Private Data --------------------
	std::string _name;
	std::string _material;

};	// class SubMesh


std::ostream &operator<<( std::ostream &os, SubMesh const &m );

/// @class Mesh
/// @todo add rebuildTangents function, similar to the one in Ogre::Mesh
/// @todo add calculateSmoothNormals, for Blender exporter
/// @todo add getPolygonCount, calculates the number of polygons in all SubMeshes
/// @todo add merge, merges two meshes preserving their SubMeshes
/// @todo add nameSubMesh
/// @todo add mergeSubMeshes, merges all SubMeshes to one and uses the material supplied
class Mesh
{
public :
	typedef std::vector<SubMesh *> SubMeshList;

	Mesh(std::string const &name);

	~Mesh(void);

	std::string const &getName(void) const
	{ return _name; }

	SubMesh *createSubMesh(void);

	void removeSubMesh(uint16_t index);

	SubMesh *getSubMesh(uint16_t index)
	{ return _sub_meshes.at(index); }

	SubMesh const *getSubMesh(uint16_t index) const
	{ return _sub_meshes.at(index); }

	unsigned int getNumSubMeshes(void) const
	{ return _sub_meshes.size(); }

	SubMeshList &getSubMeshes(void)
	{ return _sub_meshes; }

	SubMeshList const &getSubMeshes(void) const
	{ return _sub_meshes; }

	/// @todo implement
	/// Naming sub meshes is not supported
	void nameSubMesh(std::string const &name, uint16_t index);

	Ogre::AxisAlignedBox &getBounds(void)
	{ return _bounds; }

	Ogre::AxisAlignedBox const &getBounds(void) const
	{ return _bounds; }

	void setBounds(Ogre::AxisAlignedBox const &box)
	{ _bounds = box; }

	Ogre::Real &getBoundingSphereRadius(void)
	{ return _bound_radius; }

	Ogre::Real const &getBoundingSphereRadius(void) const
	{ return _bound_radius; }

	void setBoundingSphereRadius(Ogre::Real radius)
	{ _bound_radius = radius; }

	/// @brief calculates the bounding box and bounding sphere for this mesh
	/// Only uses sharedVertexData at this moment 
	/// @todo should be expanded to calculate the bounding boxes of sub meshes 
	/// and adding them to the shared geometry
	void calculateBounds(void);

	void createSharedVertexData(void)
	{ sharedVertexData = new VertexData; }

	/// ---------------------- Public Data ------------------------
	VertexData *sharedVertexData;

private :
	// Non-copyable
	Mesh(Mesh const &);
	Mesh &operator=(Mesh const &);

	/// ---------------------- Private Data ------------------------
	std::string _name;

	SubMeshList _sub_meshes;
	Ogre::AxisAlignedBox _bounds;
	Ogre::Real _bound_radius;

};	// class Mesh

Ogre::MeshPtr create_ogre_mesh(std::string const &name, vl::MeshRefPtr mesh);

void calculate_bounds(vl::VertexData const *vertexData, Ogre::AxisAlignedBox &box, Ogre::Real &sphere);

std::ostream &operator<<( std::ostream &os, Mesh const &m );

namespace cluster
{

template<>
ByteStream &operator<<(ByteStream &msg, vl::VertexDeclaration const &decl);

template<>
ByteStream &operator>>(ByteStream &msg, vl::VertexDeclaration &decl);

template<>
ByteStream &operator<<(ByteStream &msg, vl::VertexData const &vbuf);

template<>
ByteStream &operator>>(ByteStream &msg, vl::VertexData &vbuf);

template<>
ByteStream &
operator<<(ByteStream &msg, vl::IndexBuffer const &ibf);

template<>
ByteStream &
operator>>(ByteStream &msg, vl::IndexBuffer &ibf);

template<>
ByteStream &operator<<(ByteStream &msg, vl::SubMesh const &sm);

template<>
ByteStream &operator>>(ByteStream &msg, vl::SubMesh &sm);

template<>
ByteStream &operator<<(ByteStream &msg, vl::Mesh const &mesh);

template<>
ByteStream &operator>>(ByteStream &msg, vl::Mesh &mesh);

}	// namespace cluster

}	// namespace vl

#endif	// HYDRA_MESH_HPP
