/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file shapes.hpp
 *
 *	This file is part of Hydra VR game engine.
 */

// Declaration
#include "shapes.hpp"

#include "mesh.hpp"
#include "mesh_bullet.hpp"

vl::physics::BoxShapeRefPtr 
vl::physics::BoxShape::create(Ogre::Vector3 const &bounds)
{
	BoxShapeRefPtr box(new BoxShape(bounds));
	return box;
}

vl::physics::BoxShape::BoxShape(Ogre::Vector3 const &bounds)
	: _bt_shape( new btBoxShape(vl::math::convert_bt_vec(bounds)) )
{}

vl::physics::SphereShapeRefPtr 
vl::physics::SphereShape::create(vl::scalar radius)
{
	SphereShapeRefPtr sphere(new SphereShape(radius));
	return sphere;
}

vl::physics::SphereShape::SphereShape(vl::scalar radius)
	: _bt_shape(new btSphereShape(radius))
{}

vl::physics::StaticPlaneShapeRefPtr
vl::physics::StaticPlaneShape::create(Ogre::Vector3 const &normal, vl::scalar constant)
{
	StaticPlaneShapeRefPtr plane(new StaticPlaneShape(normal, constant));
	return plane;
}

vl::physics::StaticPlaneShape::StaticPlaneShape(Ogre::Vector3 const &normal, vl::scalar constant)
	: _bt_shape( new btStaticPlaneShape( vl::math::convert_bt_vec(normal), constant ) )
{}

vl::physics::StaticTriangleMeshShapeRefPtr
vl::physics::StaticTriangleMeshShape::create(vl::MeshRefPtr mesh)
{
	StaticTriangleMeshShapeRefPtr shape(new StaticTriangleMeshShape(mesh));
	return shape;
}

vl::physics::StaticTriangleMeshShape::StaticTriangleMeshShape(vl::MeshRefPtr mesh)
{
	btTriangleIndexVertexArray *bt_mesh = new btTriangleIndexVertexArray;
	vl::convert_bullet_geometry(mesh.get(), bt_mesh);

	_bt_shape = new btBvhTriangleMeshShape(bt_mesh, false);
}

vl::physics::ConvexHullShapeRefPtr
vl::physics::ConvexHullShape::create(vl::MeshRefPtr mesh)
{
	ConvexHullShapeRefPtr shape(new ConvexHullShape(mesh));
	return shape;
}

vl::physics::ConvexHullShape::ConvexHullShape(vl::MeshRefPtr mesh)
{
	btTriangleIndexVertexArray *bt_mesh = new btTriangleIndexVertexArray;
	vl::convert_bullet_geometry(mesh.get(), bt_mesh);

	/// Optimisation we should have valid bounding boxes in the meshes already
	/// @todo crashes if we use the premade bounding boxes
	_bt_shape = new btConvexTriangleMeshShape(bt_mesh, true);
}
