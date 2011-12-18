/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file physics/shapes.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Declaration
#include "shapes.hpp"

/// Concrete implementations
#ifdef USE_BULLET
#include "shapes_bullet.hpp"
#else if USE_NEWTON
#include "shapes_newton.hpp"
#endif

// @todo the create methods should return concrete bullet or newton objects
vl::physics::BoxShapeRefPtr
vl::physics::BoxShape::create(Ogre::Vector3 const &bounds)
{
	BoxShapeRefPtr box;
#ifdef USE_BULLET
	box.reset(new BulletBoxShape(bounds));
#else if USE_NEWTON
#endif
	return box;
}


vl::physics::SphereShapeRefPtr
vl::physics::SphereShape::create(vl::scalar radius)
{
	SphereShapeRefPtr sphere;
#ifdef USE_BULLET
	sphere.reset(new BulletSphereShape(radius));
#else if USE_NEWTON
#endif
	return sphere;
}

vl::physics::StaticPlaneShapeRefPtr
vl::physics::StaticPlaneShape::create(Ogre::Vector3 const &normal, vl::scalar constant)
{
	StaticPlaneShapeRefPtr plane;
#ifdef USE_BULLET
	plane.reset(new BulletStaticPlaneShape(normal, constant));
#else if USE_NEWTON
#endif
	return plane;
}

vl::physics::StaticTriangleMeshShapeRefPtr
vl::physics::StaticTriangleMeshShape::create(vl::MeshRefPtr mesh)
{
	StaticTriangleMeshShapeRefPtr shape;
#ifdef USE_BULLET
	shape.reset(new BulletStaticTriangleMeshShape(mesh));
#else if USE_NEWTON
#endif
	return shape;
}

vl::physics::ConvexHullShapeRefPtr
vl::physics::ConvexHullShape::create(vl::MeshRefPtr mesh)
{
	ConvexHullShapeRefPtr shape;
#ifdef USE_BULLET
	shape.reset(new BulletConvexHullShape(mesh));
#else if USE_NEWTON
#endif
	return shape;
}

vl::physics::CylinderShapeRefPtr
vl::physics::CylinderShape::create(Ogre::Vector3 const &bounds)
{
CylinderShapeRefPtr shape;
#ifdef USE_BULLET
	shape.reset(new BulletCylinderShape(bounds));
#else if USE_NEWTON
#endif
	return shape;
}

vl::physics::CylinderShapeRefPtr
vl::physics::CylinderShape::create(vl::scalar radius, vl::scalar height)
{
CylinderShapeRefPtr shape;
#ifdef USE_BULLET
	// convert to bounds
	shape.reset(new BulletCylinderShape(Ogre::Vector3(radius*2, height, radius)));
#else if USE_NEWTON
#endif
	return shape;
}

vl::physics::CapsuleShapeRefPtr
vl::physics::CapsuleShape::create(vl::scalar radius, vl::scalar height)
{
CapsuleShapeRefPtr shape;
#ifdef USE_BULLET
	shape.reset(new BulletCapsuleShape(radius, height));
#else if USE_NEWTON
#endif
	return shape;
}
