/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_mesh_importer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_COLLADA_DAE_MESH_IMPORTER_HPP
#define HYDRA_COLLADA_DAE_MESH_IMPORTER_HPP

#include <COLLADAFramework/COLLADAFWMeshPrimitive.h>
#include <COLLADAFramework/COLLADAFWArrayPrimitiveType.h>
#include <COLLADAFramework/COLLADAFWMeshVertexData.h>
#include <COLLADAFramework/COLLADAFWMesh.h>

#include <map>

#include "typedefs.hpp"

/// Necessary for mesh schematics
#include "mesh.hpp"

#include "dae_common.hpp"

namespace vl
{

namespace dae
{

/// @class VertexNormal
/// A collection made from Face normals
/// We can implement multiple methods for calculating the resulting vertex normal
class VertexNormal
{
public :
	// uses huge amounts of memory but should never be necessary to resize
	// @todo what is the necessary size of the array?
	VertexNormal(void)
		: _index(0)
		, _size(32)
	{}

	/// @brief calculate a sum and normalise
	Ogre::Vector3 avarage(void) const
	{
		if(_index == 0)
		{ return Ogre::Vector3::ZERO; }

		Ogre::Vector3 s = Ogre::Vector3::ZERO;
		for(size_t i = 0; i < _index; ++i)
		{ s += _normals[i]; }

		s.normalise();

		return s;
	}

	Ogre::Vector3 sum(void) const
	{
		Ogre::Vector3 s = Ogre::Vector3::ZERO;
		for(size_t i = 0; i < _index; ++i)
		{
			s += _normals[i];
		}

		return s;
	}

	size_t size() const
	{ return _index; }

	void add(Ogre::Vector3 const &v)
	{
		assert(_index < _size);
		_normals[_index] = v;
		++_index;
	}

private :
	Ogre::Vector3 _normals[32];
	size_t _index;
	size_t _size;
};

class MeshImporter
{
public:

    /** Constructor. */
	MeshImporter(MeshManagerRefPtr mesh_manager);

    /** Destructor. */
	~MeshImporter();

	bool read(COLLADAFW::Mesh const *mesh, ImporterSettings const &settings);

	SubMeshMaterialIDMap const &getSubMeshMaterialMap(void)
	{ return _submeshes; }

/// Data
private:
	MeshManagerRefPtr _mesh_manager;

	std::vector<Ogre::Vector3> _normals;
	std::vector<Ogre::Vector2> _uvs;

	// Necessary because we need to reference this when we create the index buffer
	vl::MeshRefPtr _mesh;

	// Necessary for mapping materials when the whole mesh is imported
	SubMeshMaterialIDMap _submeshes;

/// Methods
private:

    /** Disable default copy ctor. */
	MeshImporter(MeshImporter const &);

    /** Disable default assignment operator. */
	MeshImporter const &operator=(MeshImporter const &);

	void handleSubMesh(COLLADAFW::MeshPrimitive* meshPrimitive, COLLADABU::Math::Matrix4 const &matrix, vl::SubMesh *submesh);

	void handleVertexBuffer(COLLADAFW::Mesh const *mesh, vl::VertexData *vbf);
	void handleIndexBuffer(COLLADAFW::MeshPrimitive* meshPrimitive,
		const COLLADABU::Math::Matrix4& matrix, vl::IndexBuffer *ibf);

	/// Normals in Collada files are face normals (aka proper normals)
	/// we need vertex normals for meshes, so we calculate those here.
	/// You can confirm that they are face normals by importing a cube mesh
	/// and checking that it has 8 vertices and 6 normals (6 faces).
	void _calculate_vertex_normals(std::vector<uint32_t> const &normalIndices, vl::IndexBuffer *ibf);
	void _calculate_vertex_uvs(std::vector<uint32_t> const &uvIndices, vl::IndexBuffer *ibf);
	
	size_t _calculate_index_buffer_size(COLLADAFW::MeshPrimitive *meshPrimitive) const;
};


}	// namespace dae

}	// namespace vl

#endif	// HYDRA_COLLADA_DAE_MESH_IMPORTER_HPP
