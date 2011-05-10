/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file mesh.hpp
 *
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

#include <boost/tuple/tuple.hpp>

#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreAxisAlignedBox.h>
#include <OGRE/OgreMesh.h>

/// Necessary for Ref ptrs
#include "typedefs.hpp"
/// Necessary for serializing Meshes
#include "cluster/message.hpp"

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

struct VertexDeclaration
{
	VertexDeclaration(void)
	{}

	size_t vertexSize(void) const
	{
		size_t size = 0;
		
		for(size_t i =0; i < _semantics.size(); ++i)
		{
			size += getTypeSize(_semantics.at(i).second);
		}

		return size;
	}

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
	static size_t getTypeSize(Ogre::VertexElementType type)
	{
		switch(type)
		{
		case Ogre::VET_FLOAT1:
			return 1*sizeof(float);
		case Ogre::VET_FLOAT2:
			return 2*sizeof(float);
		case Ogre::VET_FLOAT3:
			return 3*sizeof(float);
		case Ogre::VET_FLOAT4:
			return 4*sizeof(float);
		case Ogre::VET_COLOUR:
			return 4*sizeof(float);
		case Ogre::VET_SHORT1:
			return 1*sizeof(short);
		case Ogre::VET_SHORT2:
			return 2*sizeof(short);
		case Ogre::VET_SHORT3:
			return 3*sizeof(short);
		case Ogre::VET_SHORT4:
			return 4*sizeof(short);
		case Ogre::VET_UBYTE4:
			return 4*1;
		case Ogre:: VET_COLOUR_ARGB:
			return 4*sizeof(float);
		case Ogre:: VET_COLOUR_ABGR:
			return 4*sizeof(float);
		default:
			return 0;
		}
	}

private :
	std::vector<Semantic> _semantics;

};

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

	size_t getNVertices(void) const
	{ return _vertices.size(); }

	VertexList &getVertices(void)
	{ return _vertices; }

	VertexList const &getVertices(void) const
	{ return _vertices; }

	void setNVertices(size_t size)
	{ _vertices.resize(size); }

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
	IndexBuffer(void)
		: _index_count(0)
		, _buffer_size(IT_16BIT)
	{}

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
	void setIndexCount(size_t count)
	{
		if(_index_count != count)
		{
			_index_count = count;
			_resize_buffer(count);
		}
	}

	size_t indexCount(void) const
	{ return _index_count; }

	INDEX_SIZE getIndexSize(void) const
	{ return _buffer_size; }

	/// @todo resetting the index size should invalidate the index buffer
	/// or resize all the indices in it.
	/// this will reset the buffer
	/// @todo after adding the automatic buffer change this can be removed
	void setIndexSize(INDEX_SIZE size)
	{
		if( size != _buffer_size )
		{
			_buffer_size = size;
			_buffer_32.clear();
			_buffer_16.clear();
		}
	}

	void push_back(uint16_t index)
	{
		push_back((uint32_t)(index));
	}

	void push_back(uint32_t index)
	{
		if(_buffer_size == IT_32BIT)
		{
			_buffer_16.push_back(index);
		}
		else
		{
			_buffer_16.push_back((uint16_t)index);
		}
		++_index_count;
	}

private :
	void _resize_buffer(size_t size)
	{
		if(_buffer_size == IT_32BIT)
		{
			_buffer_32.resize(size);
		}
		else
		{
			_buffer_16.resize(size);
		}
	}

	size_t _index_count;
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

	void addFace(uint32_t i1, uint32_t i2, uint32_t i3);

	/// --------------- Public Data --------------------
	Ogre::RenderOperation::OperationType operationType;

	bool useSharedGeometry;

	VertexData *vertexData;

	IndexBuffer indexData;

private :
	/// --------------- Private Data --------------------
	std::string _name;
	std::string _material;

};	// class SubMesh


std::ostream &operator<<( std::ostream &os, SubMesh const &m );
/// @class Mesh
/// @todo add rebuildTangents function, similar to the one in Ogre::Mesh
/// @todo add calculateSmoothNormals, for Blender exporter
/// @todo add calculateBounds, for autogenerated meshes
/// @todo add getPolygonCount, calculates the number of polygons in all SubMeshes
/// @todo add merge, merges two meshes and preserving their SubMeshes
/// @todo add name, used as file name and to distinguis the Mesh
/// @todo add mergeSubMeshes, merges all SubMeshes to one and uses the material supplied
class Mesh
{
public :
	typedef std::vector<SubMesh *> SubMeshList;

	Mesh(std::string const &name)
		: sharedVertexData(0)
		, _name(name)
		, _bound_radius(0)
	{}

	~Mesh(void);

	std::string const &getName(void) const
	{ return _name; }

	SubMesh *createSubMesh(void);

	SubMesh *getSubMesh(unsigned int index)
	{ return _sub_meshes.at(index); }

	SubMesh const *getSubMesh(unsigned int size) const
	{ return _sub_meshes.at(size); }

	unsigned int getNumSubMeshes(void) const
	{ return _sub_meshes.size(); }

	SubMeshList &getSubMeshes(void)
	{ return _sub_meshes; }

	SubMeshList const &getSubMeshes(void) const
	{ return _sub_meshes; }

	/// @todo implement
	/// Naming sub meshes is not supported
	void nameSubMesh(std::string const &name, uint16_t index);

	Ogre::AxisAlignedBox const &getBounds(void) const
	{ return _bounds; }

	void setBounds(Ogre::AxisAlignedBox const &box)
	{ _bounds = box; }

	Ogre::Real getBoundingSphereRadius(void) const
	{ return _bound_radius; }

	void setBoundingSphereRadius(Ogre::Real radius)
	{ _bound_radius = radius; }

	void calculateBounds(void);

	void createVertexData(void)
	{ sharedVertexData = new VertexData; }

	/// ---------------------- Public Data ------------------------
	VertexData *sharedVertexData;

private :
	/// ---------------------- Private Data ------------------------
	std::string _name;

	SubMeshList _sub_meshes;
	Ogre::AxisAlignedBox _bounds;
	Ogre::Real _bound_radius;

};	// class Mesh

Ogre::MeshPtr create_ogre_mesh(std::string const &name, vl::MeshRefPtr mesh);

void convert_ogre_geometry(vl::VertexData const *VertexData, Ogre::VertexData *og_vertexData);

void convert_ogre_submeshes(vl::Mesh const *mesh, Ogre::Mesh *og_mesh);

void convert_ogre_submesh(vl::SubMesh const *mesh, Ogre::SubMesh *og_sm);

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
ByteStream &operator<<(ByteStream &msg, vl::SubMesh const &mesh);

template<>
ByteStream &operator>>(ByteStream &msg, vl::SubMesh &mesh);

template<>
ByteStream &operator<<(ByteStream &msg, vl::Mesh const &mesh);

template<>
ByteStream &operator>>(ByteStream &msg, vl::Mesh &mesh);

}	// namespace cluster

}	// namespace vl

#endif	// HYDRA_MESH_HPP
