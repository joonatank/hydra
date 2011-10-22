/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_mesh_importer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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

namespace vl
{

namespace dae
{

typedef std::vector< std::pair<vl::SubMesh *, COLLADAFW::MaterialId> > SubMeshMaterialIDMap;

class MeshImporter
{
public:

    /** Constructor. */
	MeshImporter(MeshManagerRefPtr mesh_manager, COLLADAFW::Mesh const *mesh, bool flat_shading = false);

    /** Destructor. */
	~MeshImporter();

	bool write();

	SubMeshMaterialIDMap const &getSubMeshMaterialMap(void)
	{ return _submeshes; }

/// Data
private:
	COLLADAFW::Mesh const *_collada_mesh;
	MeshManagerRefPtr _mesh_manager;
	bool _flat_shading;

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

	void _calculate_vertex_normals(std::vector<uint32_t> const &normalIndices, vl::IndexBuffer *ibf);
	void _calculate_vertex_uvs(std::vector<uint32_t> const &uvIndices, vl::IndexBuffer *ibf);
	
	size_t _calculate_index_buffer_size(COLLADAFW::MeshPrimitive *meshPrimitive) const;
};


}	// namespace dae

}	// namespace vl

#endif	// HYDRA_COLLADA_DAE_MESH_IMPORTER_HPP
