/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file shapes.hpp
 *
 *	This file is part of Hydra VR game engine.
 */

#ifndef HYDRA_PHYSICS_SHAPES_HPP
#define HYDRA_PHYSICS_SHAPES_HPP

#include "typedefs.hpp"

#include "bullet/btBulletCollisionCommon.h"

#include "math/math.hpp"
#include "math/conversion.hpp"

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
	static BoxShapeRefPtr create(Ogre::Vector3 const &bounds);

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	BoxShape(Ogre::Vector3 const &bounds);

	BoxShape(BoxShape const &);
	BoxShape &operator=(BoxShape const &);

	btBoxShape *_bt_shape;

};	// class BoxShape

class SphereShape : public CollisionShape
{
public :

	static SphereShapeRefPtr create(vl::scalar radius);

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	SphereShape(vl::scalar radius);

	SphereShape(SphereShape const &other);
	SphereShape &operator=(SphereShape const &other);

	btSphereShape *_bt_shape;

};	// class SphereShape

class StaticPlaneShape : public CollisionShape
{
public :
	// Plane constant for some reason affects also in the direction of plane normal
	// so usually you want to translate the plane by plane_constant*(-plane_normal)
	StaticPlaneShapeRefPtr create(Ogre::Vector3 const &normal, vl::scalar constant);

	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

private :
	StaticPlaneShape(Ogre::Vector3 const &normal, vl::scalar constant);

	StaticPlaneShape(StaticPlaneShape const &);
	StaticPlaneShape &operator=(StaticPlaneShape const &);

	btStaticPlaneShape *_bt_shape;

};	// class ConvexHullShape

class StaticTriangleMeshShape : public CollisionShape
{
public :
	
	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	static StaticTriangleMeshShapeRefPtr create(vl::MeshRefPtr mesh);

private :
	/// @todo real implementation
	StaticTriangleMeshShape(vl::MeshRefPtr mesh);

	StaticTriangleMeshShape(StaticTriangleMeshShape const &);
	StaticTriangleMeshShape &operator=(StaticTriangleMeshShape const &);

	btBvhTriangleMeshShape *_bt_shape;

};	// class StaticTriangleMeshShape

class ConvexHullShape : public CollisionShape
{
public :
	virtual btCollisionShape *getNative(void)
	{ return _bt_shape; }

	static ConvexHullShapeRefPtr create(vl::MeshRefPtr mesh);

	void setLocalScaling(Ogre::Vector3 const &scale)
	{ _bt_shape->setLocalScaling(vl::math::convert_bt_vec(scale)); }

	Ogre::Vector3 getLocalScaling(void) const
	{ return vl::math::convert_vec(_bt_shape->getLocalScaling()); }

private :
	/// @todo real implementation
	ConvexHullShape(vl::MeshRefPtr mesh);

	ConvexHullShape(ConvexHullShape const &);
	ConvexHullShape &operator=(ConvexHullShape const &);

	/// @todo move to using btConvexHull for this, better performance
	/// needs a separate conversion algorithm
	btConvexTriangleMeshShape *_bt_shape;

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
