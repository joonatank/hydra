/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file mesh_bullet.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Bullet conversion from our Mesh structure, 
 *	in separate file so that the Bullet dependency is easily avoided
 */

#ifndef HYDRA_MESH_BULLET_HPP
#define HYDRA_MESH_BULLET_HPP

#include "mesh.hpp"

#include <bullet/BulletCollision/btBulletCollisionCommon.h>

namespace vl
{

void convert_bullet_geometry(vl::Mesh const *mesh, btTriangleIndexVertexArray *bt_mesh);

}	// namespace vl

#endif // HYDRA_MESH_BULLET_HPP