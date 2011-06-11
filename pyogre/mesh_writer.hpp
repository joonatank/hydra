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

	void allocateFaces(size_t n_faces)
	{
		_faces.resize(n_faces);
		_facet_normals.resize(n_faces);
	}

	size_t getNumFaces(void) const
	{ return _faces.size(); }

	void setFace(size_t face, int i1, int i2, int i3)
	{
		_faces.at(face) = boost::make_tuple(i1, i2, i3);
	}
	
	boost::tuple<int, int, int> const &getFace(size_t i) const
	{
		return _faces.at(i);
	}

	void setFaceNormal(size_t face, Ogre::Vector3 const &n)
	{
		_facet_normals.at(face) = n;
	}

	Ogre::Vector3 const &getFaceNormal(size_t i) const
	{ return _facet_normals.at(i); }

	bool hasFaceNormals(void) const
	{ return !_facet_normals.empty(); }

	/// @brief find the first face index based on a vertex index
	/// @return -1 if not found, valid index otherwise
	/// @todo not properly tested, and definitely not guarantied to work.
	int findFaceIndex(int vertex_index, size_t start_face) const;

private :
	std::string _name;
	std::string _material;
	std::vector< boost::tuple<int, int, int> > _faces;
	std::vector<Ogre::Vector3> _facet_normals;
};

inline
std::ostream &operator<<( std::ostream &os, SubMesh const &m )
{
	return os;
}

// @todo add more mesh tools from Nate Robins' GML
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

	/* Generates smooth vertex normals for a model.
	 * First builds a list of all the triangles each vertex is in.   Then
	 * loops through each vertex in the the list averaging all the facet
	 * normals of the triangles each vertex is in.   Finally, sets the
	 * normal index in the triangle for the vertex to the generated smooth
	 * normal.   If the dot product of a facet normal and the facet normal
	 * associated with the first triangle in the list of triangles the
	 * current vertex is in is greater than the cosine of the angle
	 * parameter to the function, that facet normal is not added into the
	 * average normal calculation and the corresponding vertex is given
	 * the facet normal.  This tends to preserve hard edges.  The angle to
	 * use depends on the model, but 90 degrees is usually a good start.
	 *
	 * angle - maximum angle to smooth across
	 *
	 * @todo not properly tested, and definitely not guarantied to work.
	 *
	 * Taken from glmVertexNormals by Nate Robins
	 */
	void smoothNormals(Ogre::Radian const &angle);

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
	void writeGeometry(vl::Mesh *mesh, Ogre::VertexData *vertexData, Ogre::Mesh *og_mesh);

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
