/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file shapes.hpp
 */

#ifndef HYDRA_PHYSICS_SHAPES_HPP
#define HYDRA_PHYSICS_SHAPES_HPP

#include "typedefs.hpp"

#include "bullet/btBulletCollisionCommon.h"

#include "mesh.hpp"

namespace vl
{

namespace physics
{

/// @brief abstract interface for collision shapes
class CollisionShape
{
public :
	virtual btCollisionShape *getNative(void) = 0;

};	// class CollisionShape

class BoxShape : public CollisionShape
{
public :

	// Create a box shape around origin: with size [-bounds, bounds]
	static BoxShapeRefPtr create(Ogre::Vector3 const &bounds)
	{
		BoxShapeRefPtr box(new BoxShape(bounds));
		return box;
	}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	BoxShape(Ogre::Vector3 const &bounds)
		: _bt_shape( new btBoxShape(vl::math::convert_bt_vec(bounds)) )
	{}

	BoxShape(BoxShape const &);
	BoxShape &operator=(BoxShape const &);

	btBoxShape *_bt_shape;

};	// class BoxShape

class SphereShape : public CollisionShape
{
public :

	static SphereShapeRefPtr create(vl::scalar radius)
	{
		SphereShapeRefPtr sphere(new SphereShape(radius));
		return sphere;
	}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	SphereShape(vl::scalar radius)
		: _bt_shape(new btSphereShape(radius))
	{}

	SphereShape(SphereShape const &other);
	SphereShape &operator=(SphereShape const &other);

	btSphereShape *_bt_shape;

};	// class SphereShape

class StaticPlaneShape : public CollisionShape
{
public :
	// Plane constant for some reason affects also in the direction of plane normal
	// so usually you want to translate the plane by plane_constant*(-plane_normal)
	StaticPlaneShapeRefPtr create(Ogre::Vector3 const &normal, vl::scalar constant)
	{
		StaticPlaneShapeRefPtr plane(new StaticPlaneShape(normal, constant));
		return plane;
	}

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	StaticPlaneShape(Ogre::Vector3 const &normal, vl::scalar constant)
		: _bt_shape( new btStaticPlaneShape( vl::math::convert_bt_vec(normal), constant ) )
	{}

	StaticPlaneShape(StaticPlaneShape const &);
	StaticPlaneShape &operator=(StaticPlaneShape const &);

	btStaticPlaneShape *_bt_shape;

};	// class ConvexHullShape

class StaticTriangleMeshShape : public CollisionShape
{
public :
	
	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	static StaticTriangleMeshShapeRefPtr create(vl::Mesh mesh)
	{
		StaticTriangleMeshShapeRefPtr shape(new StaticTriangleMeshShape(mesh));
		return shape;
	}

private :
	/// @todo real implementation
	StaticTriangleMeshShape(vl::Mesh mesh)
	{}

	StaticTriangleMeshShape(StaticTriangleMeshShape const &);
	StaticTriangleMeshShape &operator=(StaticTriangleMeshShape const &);

	btBvhTriangleMeshShape *_bt_shape;

};	// class StaticTriangleMeshShape

class ConvexHullShape : public CollisionShape
{
public :
	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	static ConvexHullShapeRefPtr create(vl::Mesh mesh)
	{
		ConvexHullShapeRefPtr shape(new ConvexHullShape(mesh));
		return shape;
	}

private :
	/// @todo real implementation
	ConvexHullShape(vl::Mesh mesh)
	{}

	ConvexHullShape(ConvexHullShape const &);
	ConvexHullShape &operator=(ConvexHullShape const &);

	btConvexHullShape *_bt_shape;

};	// class ConvexHullShape

class CylinderShape : public CollisionShape
{
public :

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	CylinderShape(void)
	{}

	CylinderShape(CylinderShape const &);
	CylinderShape &operator=(CylinderShape const &);

	btCylinderShape *_bt_shape;

};	// class CylinderShape

class CapsuleShape : public CollisionShape
{
public :

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	CapsuleShape(void)
	{}

	CapsuleShape(CapsuleShape const &);
	CapsuleShape operator=(CapsuleShape const &);

	btCapsuleShape *_bt_shape;

};	// class CapsuleShape

class ConeShape : public CollisionShape
{
public :

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	ConeShape(void)
	{}

	ConeShape(ConeShape const &);
	ConeShape &operator=(ConeShape const &);

	btConeShape *_bt_shape;

};	// class ConeShape


}	// namespace physics

}	// namepsace vl

#endif	// HYDRA_PHYSICS_SHAPES_HPP
