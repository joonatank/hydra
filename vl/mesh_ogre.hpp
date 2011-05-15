/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
*	@file mesh_ogre.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Ogre conversion from our Mesh structure, 
 *	in separate file so that the Ogre dependency is easily avoided (or can in future)
 */

#ifndef HYDRA_MESH_OGRE_HPP
#define HYDRA_MESH_OGRE_HPP

#include "mesh.hpp"

namespace vl
{

void convert_ogre_geometry(vl::VertexData const *VertexData, Ogre::VertexData *og_vertexData);

void convert_ogre_submeshes(vl::Mesh const *mesh, Ogre::Mesh *og_mesh);

void convert_ogre_submesh(vl::SubMesh const *mesh, Ogre::SubMesh *og_sm);

}	// namespace vl

#endif // HYDRA_MESH_OGRE_HPP