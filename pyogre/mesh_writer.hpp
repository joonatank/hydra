/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file mesh_writer.hpp
 */

#ifndef VL_MESH_WRITER_HPP
#define VL_MESH_WRITER_HPP

#include <OGRE/OgreMesh.h>
#include <OGRE/Ogre.h>
#include <OGRE/OgreDefaultHardwareBufferManager.h>

#include <string>

#include "boost/tuple/tuple.hpp"

namespace vl
{

struct Vertex
{
	Ogre::Vector3 position;
	Ogre::Vector3 normal;
	Ogre::ColourValue diffuse;
	Ogre::ColourValue specular;
	Ogre::Vector2 uv;
};

inline
std::ostream &operator<<( std::ostream &os, Vertex const &v )
{
	os << "Vertex : position " << v.position << " : normal " << v.normal;
	return os;
}

// Wrappers for Ogre::Mesh for writing them
class SubMesh
{
public :
	SubMesh(void) {}

	std::string const &getName(void)
	{ return _name; }

	void setName(std::string const &name)
	{ _name = name; }
	
	std::string const &getMaterial(void)
	{ return _material; }

	void setMaterial(std::string const &material)
	{ _material = material; }

	void addFace(int i1, int i2, int i3)
	{
		_faces.push_back( boost::make_tuple(i1, i2, i3) );
	}

	boost::tuple<int, int, int> const &getFace(size_t i)
	{
		return _faces.at(i);
	}

	size_t getNumFaces(void)
	{ return _faces.size(); }

private :
	std::string _name;
	std::string _material;
	std::vector< boost::tuple<int, int, int> > _faces;
};

inline
std::ostream &operator<<( std::ostream &os, SubMesh const &m )
{
	return os;
}

class Mesh
{
public :
	Mesh(void)
		: _bound_radius(0)
	{}

	~Mesh(void);

	void addVertex(Vertex const &vertex)
	{ _vertices.push_back(vertex); }

	Vertex const &getVertex(size_t i) const
	{ return _vertices.at(i); }

	size_t getNumVertices(void) const
	{ return _vertices.size(); }

	SubMesh *createSubMesh(void);

	SubMesh *getSubMesh(unsigned int size)
	{ return _sub_meshes.at(size); }

	unsigned int getNumSubMeshes(void) const
	{ return _sub_meshes.size(); }

	Ogre::AxisAlignedBox const &getBounds(void) const
	{ return _bounds; }

	void setBounds(Ogre::AxisAlignedBox const &box)
	{ _bounds = box; }

	Ogre::Real getBoundingSphereRadius(void) const
	{ return _bound_radius; }

	void setBoundingSphereRadius(Ogre::Real radius)
	{ _bound_radius = radius; }

private :
	std::vector<Vertex> _vertices;
	std::vector<SubMesh *> _sub_meshes;
	Ogre::AxisAlignedBox _bounds;
	Ogre::Real _bound_radius;
};

inline
std::ostream &operator<<( std::ostream &os, Mesh const &m )
{
	os << "Mesh : vertices = ";
	for( size_t i = 0; i < m.getNumVertices(); ++i )
	{
		os << m.getVertex(i).position << ", ";
	}
	os << std::endl;
	return os;
}


class MeshWriter
{
public :
	MeshWriter( void );

	~MeshWriter( void );

	vl::Mesh *createMesh(void);

	void writeMesh(vl::Mesh *mesh, std::string const &filename);

private :
	void writeGeometry(vl::Mesh *mesh, Ogre::VertexData *vertexData);

	void writeSubMeshes(vl::Mesh *mesh, Ogre::Mesh *og_mesh);

	void writeSubMesh(vl::SubMesh *mesh, Ogre::SubMesh *og_sm);

	Ogre::LogManager *_logMgr;
	Ogre::Math *_math;
	Ogre::LodStrategyManager *_lodMgr;
	Ogre::MaterialManager* _matMgr;
	Ogre::MeshSerializer* _meshSerializer;
	Ogre::DefaultHardwareBufferManager *_bufferMgr;
	Ogre::MeshManager* _meshMgr;
	Ogre::ResourceGroupManager* _resourcegm;
	Ogre::SkeletonManager *_skelMgr;
	Ogre::SkeletonSerializer *_skeletonSerializer;

};	// class MeshWriter

}	// namespace vl

#endif	// VL_MESH_WRITER_HPP