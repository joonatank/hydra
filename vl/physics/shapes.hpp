/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file physics/shapes.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PHYSICS_SHAPES_HPP
#define HYDRA_PHYSICS_SHAPES_HPP

#include "typedefs.hpp"

#include "math/math.hpp"
#include "math/conversion.hpp"

namespace vl
{

namespace physics
{

/// @brief abstract interface for collision shapes
/// To avoid collisions this can not have any concrete data as it's inherited multiple times.
class CollisionShape
{
public :
	virtual ~CollisionShape(void) {}

	virtual void setMargin(vl::scalar margin) = 0;

	virtual vl::scalar getMargin(void) const = 0;

	// @todo add default margin

};	// class CollisionShape

class HYDRA_API BoxShape : public CollisionShape
{
public :
	// Create a box shape around origin: with size [-bounds/2, bounds/2]
	static BoxShapeRefPtr create(Ogre::Vector3 const &bounds);

	virtual ~BoxShape(void) {}

	/// @brief Return the size of the cube.
	virtual Ogre::Vector3 getSize(void) = 0;

protected :
	BoxShape(void) {}

private :
	BoxShape(BoxShape const &);
	BoxShape &operator=(BoxShape const &);


};	// class BoxShape

class HYDRA_API SphereShape : public CollisionShape
{
public :
	static SphereShapeRefPtr create(vl::scalar radius);

	virtual ~SphereShape(void) {}

protected :
	SphereShape(void) {}

private :
	SphereShape(SphereShape const &other);
	SphereShape &operator=(SphereShape const &other);

};	// class SphereShape

class HYDRA_API StaticPlaneShape : public CollisionShape
{
public :
	// Plane constant for some reason affects also in the direction of plane normal
	// so usually you want to translate the plane by plane_constant*(-plane_normal)
	StaticPlaneShapeRefPtr create(Ogre::Vector3 const &normal, vl::scalar constant);

	virtual ~StaticPlaneShape(void) {}

protected :
	StaticPlaneShape(void) {}

private :
	StaticPlaneShape(StaticPlaneShape const &);
	StaticPlaneShape &operator=(StaticPlaneShape const &);


};	// class ConvexHullShape

class HYDRA_API StaticTriangleMeshShape : public CollisionShape
{
public :
	static StaticTriangleMeshShapeRefPtr create(vl::MeshRefPtr mesh);

	virtual ~StaticTriangleMeshShape(void) {}

protected :
	StaticTriangleMeshShape(void) {}

private :
	StaticTriangleMeshShape(StaticTriangleMeshShape const &);
	StaticTriangleMeshShape &operator=(StaticTriangleMeshShape const &);


};	// class StaticTriangleMeshShape

class HYDRA_API ConvexHullShape : public CollisionShape
{
public :
	static ConvexHullShapeRefPtr create(vl::MeshRefPtr mesh);

	virtual ~ConvexHullShape(void) {}

	/* Do we need these for some reason?
	void setLocalScaling(Ogre::Vector3 const &scale)
	{ _bt_shape->setLocalScaling(vl::math::convert_bt_vec(scale)); }

	Ogre::Vector3 getLocalScaling(void) const
	{ return vl::math::convert_vec(_bt_shape->getLocalScaling()); }
	*/

	vl::MeshRefPtr getMesh(void) const
	{ return _mesh; }

protected :
	ConvexHullShape(vl::MeshRefPtr mesh)
		: _mesh(mesh)
	{}

private :
	ConvexHullShape(ConvexHullShape const &);
	ConvexHullShape &operator=(ConvexHullShape const &);

	vl::MeshRefPtr _mesh;

};	// class ConvexHullShape

/// @todo not implemented
class HYDRA_API CylinderShape : public CollisionShape
{
public :
	static CylinderShapeRefPtr create(Ogre::Vector3 const &bounds);

	static CylinderShapeRefPtr create(vl::scalar radius, vl::scalar height);

protected :
	CylinderShape(void)
	{}

private :
	CylinderShape(CylinderShape const &);
	CylinderShape &operator=(CylinderShape const &);

};	// class CylinderShape

/// @todo not implemented
class HYDRA_API CapsuleShape : public CollisionShape
{
public :
	static CapsuleShapeRefPtr create(vl::scalar radius, vl::scalar height);

protected :
	CapsuleShape(void)
	{}

private:
	CapsuleShape(CapsuleShape const &);
	CapsuleShape operator=(CapsuleShape const &);

};	// class CapsuleShape

/// @todo not implemented
class HYDRA_API ConeShape : public CollisionShape
{
public :

private :
	ConeShape(void)
	{}

	ConeShape(ConeShape const &);
	ConeShape &operator=(ConeShape const &);

};	// class ConeShape


}	// namespace physics

}	// namepsace vl

#endif	// HYDRA_PHYSICS_SHAPES_HPP
