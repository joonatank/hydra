/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
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

class MeshImporter
{
public:

    /** Constructor. */
	MeshImporter(MeshManagerRefPtr mesh_manager, COLLADAFW::Mesh const *mesh);

    /** Destructor. */
	~MeshImporter();

	bool write();

/// Data
private:
	const COLLADAFW::Mesh* _collada_mesh;
	MeshManagerRefPtr _mesh_manager;

/// Methods
private:

    /** Disable default copy ctor. */
	MeshImporter(MeshImporter const &);

    /** Disable default assignment operator. */
	MeshImporter const &operator=(MeshImporter const &);

	void handleSubMesh(COLLADAFW::MeshPrimitive* meshPrimitive, COLLADABU::Math::Matrix4 const &matrix, vl::SubMesh *submesh);

	void handleSubMeshOperation(COLLADAFW::MeshPrimitive::PrimitiveType primitiveType);

	void handleVertexBuffer(vl::VertexData *data);
	void handleIndexBuffer(COLLADAFW::MeshPrimitive* meshPrimitive,
		const COLLADABU::Math::Matrix4& matrix, vl::IndexBuffer *data);
};


}	// namespace dae

}	// namespace vl

#endif	// HYDRA_COLLADA_DAE_MESH_IMPORTER_HPP
