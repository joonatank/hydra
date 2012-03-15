/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/tube.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *
 */

#ifndef HYDRA_PHYSICS_TUBE_HPP
#define HYDRA_PHYSICS_TUBE_HPP

// Necessary for vl::scalar
#include "math/types.hpp"
// Necessary for Transform
#include "math/transform.hpp"
// Necessary for pointer types and forward declarations
#include "typedefs.hpp"

namespace vl
{

namespace physics
{

/**	@class Tube
 *	@brief Simulates a single hydraulic tube with two endpoints
 */
class Tube
{
public :
	struct ConstructionInfo
	{
		/// @todo should we either add parameters for things that we can not
		/// guess like length, radius and mass
		/// or should we make them invalid (0) and check when the structure 
		/// is used that the user has made them valid.
		ConstructionInfo(void)
			: length(10)
			, radius(0.1)
			, mass_per_meter(1)
			, stiffness(0)
			, damping(1.0)
			, material_name("BaseWhite")
			, element_size(0.6)
			, lower_lim(Vector3::ZERO)
			, upper_lim(Vector3(-1, -1, -1))
			, fixing_lower_lim(Vector3::ZERO)
			, fixing_upper_lim(Vector3(-1, -1, -1))
			, spring(true)
			, disable_collisions(false)
			, inertia_factor(1)
			, body_damping(0)
			, bending_radius(-1)
			, use_instancing(false)
		{}

		RigidBodyRefPtr start_body;
		RigidBodyRefPtr end_body;
		Transform start_body_frame;
		Transform end_body_frame;

		vl::scalar length;
		vl::scalar radius;

		// automatically calculates the mass and discard s the mass parameter
		vl::scalar mass_per_meter;
		vl::scalar stiffness;
		vl::scalar damping;
		vl::scalar element_size;

		std::string material_name;

		// Angular limits
		Ogre::Vector3 lower_lim;
		Ogre::Vector3 upper_lim;
		Ogre::Vector3 fixing_lower_lim;
		Ogre::Vector3 fixing_upper_lim;

		bool spring;
		bool disable_collisions;

		// Factor to multiply the inertia calculated for the tube shapes
		vl::scalar inertia_factor;

		vl::scalar body_damping;

		// If greater than zero automatically calculates the joint limits
		// discarding the predefined ones.
		vl::scalar bending_radius;

		// rendering optimisation, still experimental
		bool use_instancing;
	};

	/**	@brief Constructor
	 *	@param world where to add the tube
	 *	@param start_body the body where the one end of the tube is tied to can be NULL
	 *	@param end_body the point where the other end of the tube is tied to can be NULL
	 *	@param length the length of the tube
	 *	@param radius the radius of the tube
	 *	@param mass the mass of the tube
	 *	@todo how to integrate this in to our animation system with out rigid body
	 *	simulation
	 */
	Tube(WorldPtr world, SceneManagerPtr sm, ConstructionInfo const &info);

	/// @brief Destructor
	~Tube(void);

	/// @brief
	void setSpringStiffness(vl::scalar stiffness);

	vl::scalar getSpringStiffness(void) const
	{ return _stiffness; }

	void setSpringDamping(vl::scalar damping);

	vl::scalar getSpringDamping(void) const
	{ return _damping; }

	void setDamping(vl::scalar damping);

	vl::scalar getDamping(void) const
	{ return _body_damping; }

	/// @brief get the size of a single simulated rigid body
	vl::scalar getElementSize(void) const
	{ return _element_size; }

	/// @brief get the radius of the tube
	vl::scalar getRadius(void) const
	{ return _tube_radius; }

	vl::scalar getLength(void) const
	{ return _length; }

	vl::scalar getMass(void) const
	{ return _mass; }

	void setMass(vl::scalar mass);

	std::string const &getMaterial(void) const
	{ return _material_name; }

	/// @todo not implemented
	void setMaterial(std::string const &material);

	Ogre::Vector3 const &getLowerLim(void) const
	{ return _lower_lim; }

	void setLowerLim(Ogre::Vector3 const &lim);

	Ogre::Vector3 const &getUpperLim(void) const
	{ return _upper_lim; }

	void setUpperLim(Ogre::Vector3 const &lim);

	void hide(void);

	void show(void);

	void setShowBoundingBoxes(bool show);

	bool isShowBoundingBoxes(void) const;

	void setEquilibrium(void);

	/// @brief add a point where the tube is fixed to a body
	/// @param body the body where to fix the tube
	/// @param legnth the length of the tube where the fixing should be 
	/// if negative uses the position of body
	void addFixingPoint(RigidBodyRefPtr body, vl::scalar length = -1);

	void removeFixingPoint(RigidBodyRefPtr body);

	void create(void);

	/// These are valid only after create has been called
	/// will throw if they are called before the tube is created

	SixDofConstraintRefPtr getStartFixing(void);

	SixDofConstraintRefPtr getEndFixing(void);

	/// @brief get one of the extra fixing points
	/// throws if there is no such such fixing point
	SixDofConstraintRefPtr getFixing(size_t index);

	/// @brief get the number of extra fixing points
	size_t getNFixings(void) const;

	/// @brief get all fixing points except the start and end point
	ConstraintList const &getFixings(void) const;

private :
	/// Used for naming purposes
	static size_t n_tubes;

	void _createConstraints(vl::Transform const &start_frame, vl::Transform const &end_frame, vl::scalar elem_length);

	/// Helper methods for Graphics engine so we don't need to pass 
	/// all this information to the constructor, which would be pretty hard
	/// as we would need to pass it also to the World.
	void _createMesh(MeshManagerRefPtr mesh_manager);

	void _setConstraint(ConstraintRefPtr constraint, Ogre::Vector3 const &lower, Ogre::Vector3 const &upper);

	void _add_fixing_point(RigidBodyRefPtr body, vl::scalar length);

	RigidBodyRefPtr _findBodyByLength(vl::scalar length);
	RigidBodyRefPtr _findBodyByPosition(Ogre::Vector3 const &pos);

	RigidBodyRefPtr _start_body;
	RigidBodyRefPtr _end_body;

	BoxShapeRefPtr _shape;

	// Uses multiple bodies and constraints to build the tube
	// @todo should this rather use a map that maps the constraint
	// to a body or a custom structure?
	RigidBodyList _bodies;
	ConstraintList _constraints;
	
	std::map<vl::scalar, RigidBodyRefPtr> _fixing_bodies;
	ConstraintList _external_fixings;

	vl::scalar _length;
	vl::scalar _stiffness;
	vl::scalar _damping;
	vl::scalar _element_size;
	vl::scalar _tube_radius;
	vl::scalar _mass;
	vl::scalar _body_damping;
	bool _spring;
	bool _disable_internal_collisions;
	bool _use_instancing;

	Ogre::Vector3 _inertia;

	// Angular limits
	Ogre::Vector3 _lower_lim;
	Ogre::Vector3 _upper_lim;
	Ogre::Vector3 _fixing_lower_lim;
	Ogre::Vector3 _fixing_upper_lim;

	std::string _material_name;

	vl::Transform _start_body_frame;
	vl::Transform _end_body_frame;

	WorldPtr _world;
	SceneManagerPtr _scene;

};	// class Tube

std::ostream &operator<<(std::ostream &os, Tube const &tube);

std::ostream &operator<<(std::ostream &os, TubeList const &tube);

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_TUBE_HPP
