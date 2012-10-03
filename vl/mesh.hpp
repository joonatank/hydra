/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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

	void addElement(unsigned short source, size_t offset, Ogre::VertexElementType theType, 
		Ogre::VertexElementSemantic semantic, unsigned short index = 0)
	{
		_elements.push_back(Ogre::VertexElement(source, offset, theType, semantic, index));
	}

	std::vector<Ogre::VertexElement> const &getElements(void) const
	{ return _elements; }

	std::vector<Ogre::VertexElement> &getElements(void)
	{ return _elements; }

	/// @return position and size pair
	std::pair<size_t, size_t> getSemanticPosition(Ogre::VertexElementSemantic type) const
	{
		size_t pos = 0;
		for(std::vector<Ogre::VertexElement>::const_iterator iter = _elements.begin();
			iter != _elements.end(); ++iter)
		{
			if(iter->getSemantic() == type)
			{
				return std::make_pair(pos, iter->getSize());
			}
			pos += iter->getSize();
		}

		return std::make_pair(0, 0);
	}

	bool hasSemantic(Ogre::VertexElementSemantic type) const
	{
		for(std::vector<Ogre::VertexElement>::const_iterator iter = _elements.begin();
			iter != _elements.end(); ++iter)
		{
			if(iter->getSemantic() == type)
			{ return true; }
		}
	}

	/// @todo not sure if these are correct
	static size_t getTypeSize(Ogre::VertexElementType type);

private :
	std::vector<Ogre::VertexElement> _elements;

};

/**	@class VertexBuffer
 *
 *	@brief Data structure to hold the actual vertex information of a mesh
 *	Does not allow resizing of the buffer it needs to be recreated instead.
 *	This is so marginal case that it was deemed unnecessary overhead.
 *
 *	@todo add stream operator for VertexBuffer
 *	We need to lock and unlock the buffer to receive an input/output stream
 *	that counts the current position.
 **/
class VertexBuffer
{
public :
	VertexBuffer(size_t vertex_size, size_t n_vertices)
		: _buffer(0)
		, _n_vertices(n_vertices)
		, _vertex_size(vertex_size)
	{
		// Forbid empty buffers, any buffer that has less than 3 vertices
		// can not represent a triangle
		// it could represent a line so we keep the limit to at least 2 vertices.
		assert(_n_vertices > 1 && _vertex_size > 0);
		_buffer = new char[_n_vertices * _vertex_size];
	}

	void read(size_t offset, void *data, size_t size_) const
	{
		assert(offset + size_ <= this->size());

		memcpy(data, _buffer + offset, size_);
	}

	/// @brief write data
	/// You must ensure that the buffer is long enough
	void write(size_t offset, void const *data, size_t size_)
	{
		assert(offset + size_ <= this->size());

		memcpy(_buffer + offset, data, size_);
	}

	template<typename T>
	void write(size_t offset, T const &data)
	{
		write(offset, &data, sizeof(data));
	}

	template<typename T>
	void read(size_t offset, T &data) const
	{
		read(offset, &data, sizeof(data));
	}

	/// Copy data from this buffer to another char buffer
	/// Ensure that the dest buffer is long enough to hold the data
	void copy(char *dest)
	{
		memcpy(dest, _buffer, size());
	}

	size_t getNVertices(void) const
	{ return _n_vertices; }

	size_t getVertexSize(void) const
	{ return _vertex_size; }

	size_t size(void) const
	{ return _n_vertices*_vertex_size; }

	// @todo cleaner design the bit buffer should be private
	// no getters and no friends
	char *_buffer;

private :
	size_t _n_vertices;
	size_t _vertex_size;
};

template<typename T>
class VertexIterator
{
public :
	VertexIterator(VertexBuffer *buf, size_t offset)
		: _buffer(buf), _offset(offset), _position(0)
	{}

	// Invalid iterator constructor
	VertexIterator(void)
		: _buffer(0), _offset(0), _position(0)
	{}

	T* value(void)
	{	
		T *a = reinterpret_cast<T *>(_buffer->_buffer + _buffer_position());
		return a;
	}

	T& operator*(void)
	{
		return *value();
	}

	T* operator->(void)
	{
		return value();
	}
	
	VertexIterator &operator++(void)
	{
		++_position;
		return *this;
	}

	VertexIterator operator++(int)
	{
		VertexIterator iter(*this);
		++_position;
		return iter;
	}

	VertexIterator &operator--(void)
	{
		--_position;
		return *this;
	}

	VertexIterator operator--(int)
	{
		VertexIterator iter(*this);
		--_position;
		return iter;
	}

	bool operator==(VertexIterator const &i) const
	{
		return _buffer == i._buffer 
			&& _offset == i._offset 
			&& _position == i._position;
	}

	bool operator!=(VertexIterator const &i) const
	{ return !(*this == i); }
	
	bool end(void) const
	{ return _buffer && ((_buffer->size()-1) <= _buffer_position()); }

	/// @internal
	size_t _buffer_position(void) const
	{
		return _buffer->getVertexSize()*_position + _offset;
	}

private :
	VertexBuffer *_buffer;
	// static offset between values in the buffer
	const size_t _offset;
	// current position in the buffer
	size_t _position;
};

typedef VertexIterator<Ogre::Vector3> PositionIterator;
typedef VertexIterator<Ogre::Vector3> NormalIterator;
typedef VertexIterator<Ogre::Vector2> UVIterator;

/// @class VertexData
/// @todo Replace Vertex structure with a buffer where VertexElememt arrays
/// are continous memory segments.
/// for example [positions | normals | uvs | tangents]
/// For now we can use a structure where every VertexElement has a
/// different array, so it's easier to resize it.
struct VertexData
{
	VertexData(void)
		: buffer(0)
	{}

	PositionIterator getPositionIterator(void)
	{
		if(vertexDeclaration.hasSemantic(Ogre::VES_POSITION))
		{
			std::pair<size_t, size_t> pos = vertexDeclaration.getSemanticPosition(Ogre::VES_POSITION);
			// Semantic size needs to be some as the return type size
			assert(sizeof(Ogre::Vector3) == pos.second);

			return PositionIterator(buffer, pos.first);
		}
		else
		{
			std::clog << "VertexData has no Position semantics" << std::endl;
			throw;
		}
	}

	NormalIterator getNormalIterator(void)
	{
		if(vertexDeclaration.hasSemantic(Ogre::VES_NORMAL))
		{
			std::pair<size_t, size_t> pos = vertexDeclaration.getSemanticPosition(Ogre::VES_NORMAL);
			// Semantic size needs to be some as the return type size
			assert(sizeof(Ogre::Vector3) == pos.second);

			return NormalIterator(buffer, pos.first);
		}
		else
		{
			std::clog << "VertexData has no Normal semantics" << std::endl;
			throw;
		}		
	}

	VertexBuffer *buffer;

	VertexDeclaration vertexDeclaration;

};	// struct VertexData

/// Stub
struct VertexBoneAssignment
{
};

class IndexBuffer
{
public :
	IndexBuffer(void) {}

	~IndexBuffer(void) {}
	
	uint32_t const *getBuffer(void) const
	{ return &_buffer[0]; }

	uint32_t *getBuffer(void)
	{ return &_buffer[0]; }

	std::vector<uint32_t> const &getVec(void) const
	{ return _buffer; }

	std::vector<uint32_t> &getVec(void)
	{ return _buffer; }

	uint32_t &operator[](size_t i)
	{ return _buffer[i]; }

	uint32_t const &operator[](size_t i) const
	{ return _buffer[i]; }

	/// @todo this should change the current buffer automatically to 16-bit or 32-bit
	/// depending on the count
	void setIndexCount(size_t count)
	{ _buffer.resize(count); }

	size_t indexCount(void) const
	{ return _buffer.size(); }

	// @brief convenience function that works like std::vector::push_back
	// do not use if you are concerned with performance
	void push_back(uint32_t index)
	{ _buffer.push_back(index); }

	void set(size_t i, uint32_t index);

private :
	void _resize_buffer(size_t size);

	std::vector<uint32_t> _buffer;

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
ByteStream &operator<<(ByteStream &msg, Ogre::VertexElement const &elem);

template<>
ByteStream &operator>>(ByteStream &msg, Ogre::VertexElement &elem);


template<>
ByteStream &operator<<(ByteStream &msg, vl::VertexBuffer const &vbuf);

template<>
ByteStream &operator>>(ByteStream &msg, vl::VertexBuffer &vbuf);

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
