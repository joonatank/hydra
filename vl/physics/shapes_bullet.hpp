/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/shapes_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**
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
#include "bullet/BulletCollision/Gimpact/btGImpactShape.h"
#include "math/conversion.hpp"

namespace vl
{

namespace physics
{

class BulletCollisionShape : public vl::physics::CollisionShape
{
public :
	virtual btCollisionShape *getNative(void) = 0;

	virtual btCollisionShape const *getNative(void) const = 0;

};

typedef boost::shared_ptr<BulletCollisionShape> BulletCollisionShapeRefPtr;

class BulletBoxShape : public BulletCollisionShape, public vl::physics::BoxShape
{
public :
	BulletBoxShape(Ogre::Vector3 const &bounds)
		: BoxShape()
		// dividing by two because Bullet uses halfExtends and we use bounds
		, _bt_shape( new btBoxShape(vl::math::convert_bt_vec(bounds/2)) )
	{}

	virtual ~BulletBoxShape(void) {}

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual Ogre::Vector3 getSize(void)
	{
		btVector3 min, max;
		_bt_shape->getAabb(btTransform::getIdentity(), min, max);
		return vl::math::convert_vec(max - min);
	}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
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

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
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

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }
	
	virtual btCollisionShape const *getNative(void) const
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

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
	{ return _bt_shape; }

private :
	btBvhTriangleMeshShape *_bt_shape;
};

class BulletConvexHullShape : public BulletCollisionShape, public vl::physics::ConvexHullShape
{
public :
	BulletConvexHullShape(vl::MeshRefPtr mesh)
		: ConvexHullShape(mesh), _bt_shape(0)
	{
		btTriangleIndexVertexArray *bt_mesh = new btTriangleIndexVertexArray;
		vl::convert_bullet_geometry(mesh.get(), bt_mesh);

		/// Optimisation we should have valid bounding boxes in the meshes already
		/// @todo crashes if we use the premade bounding boxes
		_bt_shape = new btConvexTriangleMeshShape(bt_mesh, true);
	}

	virtual ~BulletConvexHullShape(void) {}

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
	{ return _bt_shape; }

private :
	/// @todo move to using btConvexHull for this, better performance
	/// needs a separate conversion algorithm
	btConvexTriangleMeshShape *_bt_shape;
};


class BulletConcaveHullShape : public BulletCollisionShape, public vl::physics::ConcaveHullShape
{
public :
	BulletConcaveHullShape(vl::MeshRefPtr mesh)
		: ConcaveHullShape(mesh), _bt_shape(0)
	{
		btTriangleIndexVertexArray *bt_mesh = new btTriangleIndexVertexArray;
		vl::convert_bullet_geometry(mesh.get(), bt_mesh);

		_bt_shape = new btGImpactMeshShape(bt_mesh);
		
		_bt_shape->updateBound();
	}

	virtual ~BulletConcaveHullShape(void) {}

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
	{ return _bt_shape; }

private :
	btGImpactMeshShape *_bt_shape;
};

class BulletCompoundShape : public BulletCollisionShape, public vl::physics::CompoundShape
{
public:
	BulletCompoundShape(bool dynamicTree)
		:_bt_shape(0)
	{
		_bt_shape = new btCompoundShape(dynamicTree);
	}

	virtual ~BulletCompoundShape(void)
	{}
	
	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
	{ return _bt_shape; }	
	
	virtual void addChildShape(vl::Transform const &localTrans, vl::physics::CollisionShapeRefPtr shape)
	{	
		//@warning: IS THIS ALLOWED?
		BulletCollisionShapeRefPtr shape_bullet( boost::dynamic_pointer_cast<BulletCollisionShape>(shape) );
		assert(shape_bullet);
		_bt_shape->addChildShape(vl::math::convert_bt_transform(localTrans), shape_bullet->getNative());
	}

	virtual void removeChildShapeByIndex(int idx)
	{
		//@warning: IS THIS ALLOWED?
		//BulletCollisionShapeRefPtr shape_bullet = boost::dynamic_pointer_cast<BulletCollisionShape>(shape);
		//assert(shape_bullet);
		_bt_shape->removeChildShapeByIndex(idx);
		//_bt_shape->removeChildShape(shape_bullet->getNative());
	}
	
private:
	btCompoundShape *_bt_shape;
};

class BulletCylinderShape : public BulletCollisionShape, public vl::physics::CylinderShape
{
public :

	BulletCylinderShape(Ogre::Vector3 const &bounds)
		// dividing by two because Bullet uses halfExtends and we use bounds
		: _bt_shape(new btCylinderShape(vl::math::convert_bt_vec(bounds/2)))
	{}
	
	virtual ~BulletCylinderShape(void) {}

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
	{ return _bt_shape; }

private :
	btCylinderShape *_bt_shape;
};

class BulletCapsuleShape : public BulletCollisionShape, public vl::physics::CapsuleShape
{
public :
	BulletCapsuleShape(vl::scalar radius, vl::scalar height)
		// divide by two because we use height to mean the actual height of
		// the capsule (as in bounding box size) not height from center.
		: _bt_shape(new btCapsuleShape(radius, height/2))
	{}
	
	virtual ~BulletCapsuleShape(void) {}

	virtual void setMargin(vl::scalar margin)
	{ _bt_shape->setMargin(margin); }

	virtual vl::scalar getMargin(void) const
	{ return _bt_shape->getMargin(); }

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	virtual btCollisionShape const *getNative(void) const
	{ return _bt_shape; }

private :
	btCapsuleShape *_bt_shape;
};

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_SHAPES_BULLET_HPP
