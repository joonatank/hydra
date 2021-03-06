/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file physics/mesh_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Bullet conversion from our Mesh structure, 
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