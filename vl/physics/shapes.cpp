/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file physics/shapes.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

// Declaration
#include "shapes.hpp"

/// Concrete implementation
#include "shapes_bullet.hpp"

vl::physics::BoxShapeRefPtr
vl::physics::BoxShape::create(Ogre::Vector3 const &bounds)
{
	BoxShapeRefPtr box(new BulletBoxShape(bounds));
	return box;
}


vl::physics::SphereShapeRefPtr
vl::physics::SphereShape::create(vl::scalar radius)
{
	SphereShapeRefPtr sphere(new BulletSphereShape(radius));
	return sphere;
}

vl::physics::StaticPlaneShapeRefPtr
vl::physics::StaticPlaneShape::create(Ogre::Vector3 const &normal, vl::scalar constant)
{
	StaticPlaneShapeRefPtr plane(new BulletStaticPlaneShape(normal, constant));
	return plane;
}

vl::physics::StaticTriangleMeshShapeRefPtr
vl::physics::StaticTriangleMeshShape::create(vl::MeshRefPtr mesh)
{
	StaticTriangleMeshShapeRefPtr shape(new BulletStaticTriangleMeshShape(mesh));
	return shape;
}

vl::physics::ConvexHullShapeRefPtr
vl::physics::ConvexHullShape::create(vl::MeshRefPtr mesh)
{
	ConvexHullShapeRefPtr shape(new BulletConvexHullShape(mesh));
	return shape;
}

vl::physics::ConcaveHullShapeRefPtr
vl::physics::ConcaveHullShape::create(vl::MeshRefPtr mesh)
{
	ConcaveHullShapeRefPtr shape(new BulletConcaveHullShape(mesh));
	return shape;
}

vl::physics::CompoundShapeRefPtr
vl::physics::CompoundShape::create(bool dynamicTree)
{
	CompoundShapeRefPtr shape(new BulletCompoundShape(dynamicTree));
	return shape;
}

vl::physics::CylinderShapeRefPtr
vl::physics::CylinderShape::create(Ogre::Vector3 const &bounds)
{
	CylinderShapeRefPtr shape(new BulletCylinderShape(bounds));
	return shape;
}

vl::physics::CylinderShapeRefPtr
vl::physics::CylinderShape::create(vl::scalar radius, vl::scalar height)
{
	// convert to bounds
	CylinderShapeRefPtr shape(new BulletCylinderShape(Ogre::Vector3(radius*2, height, radius)));
	return shape;
}

vl::physics::CapsuleShapeRefPtr
vl::physics::CapsuleShape::create(vl::scalar radius, vl::scalar height)
{
	CapsuleShapeRefPtr shape(new BulletCapsuleShape(radius, height));
	return shape;
}
