/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file physics/shapes_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 *	Concrete implementation for bullet physics engine.
 */

#ifndef HYDRA_PHYSICS_SHAPES_BULLET_HPP
#define HYDRA_PHYSICS_SHAPES_BULLET_HPP

// Base class
#include "shapes.hpp"

/// Necesary for loading from mesh
#include "mesh.hpp"
#include "mesh_bullet.hpp"

// Concrete physics engine implementation
#include "bullet/btBulletCollisionCommon.h"

namespace vl
{

namespace physics
{

class BulletCollisionShape : public vl::physics::CollisionShape
{
public :
	virtual btCollisionShape *getNative(void) = 0;

};

typedef boost::shared_ptr<BulletCollisionShape> BulletCollisionShapeRefPtr;

class BulletBoxShape : public BulletCollisionShape, public vl::physics::BoxShape
{
public :
	BulletBoxShape(Ogre::Vector3 const &bounds)
		: BoxShape()
		, _bt_shape( new btBoxShape(vl::math::convert_bt_vec(bounds)) )
	{}

	virtual ~BulletBoxShape(void) {}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	btBoxShape *_bt_shape;
};

class BulletSphereShape : public BulletCollisionShape, public vl::physics::SphereShape
{
public :		
	BulletSphereShape(vl::scalar radius)
		: SphereShape()
		, _bt_shape(new btSphereShape(radius))
	{}

	virtual ~BulletSphereShape(void) {}
		
	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	btSphereShape *_bt_shape;
};

class BulletStaticPlaneShape : public BulletCollisionShape, public vl::physics::StaticPlaneShape
{
public :
	
	BulletStaticPlaneShape(Ogre::Vector3 const &normal, vl::scalar constant)
		: StaticPlaneShape()
		, _bt_shape( new btStaticPlaneShape( vl::math::convert_bt_vec(normal), constant ) )
	{}

	virtual ~BulletStaticPlaneShape(void) {}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }
	
private :
	btStaticPlaneShape *_bt_shape;

};

class BulletStaticTriangleMeshShape : public BulletCollisionShape, public vl::physics::StaticTriangleMeshShape
{
public :
	BulletStaticTriangleMeshShape(vl::MeshRefPtr mesh)
		: StaticTriangleMeshShape(), _bt_shape(0)
	{
		btTriangleIndexVertexArray *bt_mesh = new btTriangleIndexVertexArray;
		vl::convert_bullet_geometry(mesh.get(), bt_mesh);

		_bt_shape = new btBvhTriangleMeshShape(bt_mesh, false);
	}

	virtual ~BulletStaticTriangleMeshShape(void) {}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	btBvhTriangleMeshShape *_bt_shape;
};

class BulletConvexHullShape : public BulletCollisionShape, public vl::physics::ConvexHullShape
{
public :
	BulletConvexHullShape(vl::MeshRefPtr mesh)
		: ConvexHullShape(), _bt_shape(0)
	{
		btTriangleIndexVertexArray *bt_mesh = new btTriangleIndexVertexArray;
		vl::convert_bullet_geometry(mesh.get(), bt_mesh);

		/// Optimisation we should have valid bounding boxes in the meshes already
		/// @todo crashes if we use the premade bounding boxes
		_bt_shape = new btConvexTriangleMeshShape(bt_mesh, true);
	}

	virtual ~BulletConvexHullShape(void) {}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	/// @todo move to using btConvexHull for this, better performance
	/// needs a separate conversion algorithm
	btConvexTriangleMeshShape *_bt_shape;
};

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_SHAPES_BULLET_HPP
