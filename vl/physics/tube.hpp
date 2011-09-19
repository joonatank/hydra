/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file physics/tube.hpp
 *
 *	This file is part of Hydra a VR game engine.
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
			, mass(50.0)
			, stiffness(0.9)
			, damping(0.1)
			, element_size(0.6)
		{}

//		ConstructionInfo(RigidBodyRefPtr body1, RigidBodyRefPtr body2, Transform frame1, Transform frame2, 

		RigidBodyRefPtr start_body;
		RigidBodyRefPtr end_body;
		Transform start_body_frame;
		Transform end_body_frame;

		vl::scalar length;
		vl::scalar radius;
		vl::scalar mass;
		vl::scalar stiffness;
		vl::scalar damping;
		vl::scalar element_size;

		std::string material_name;

		// Angular limits
		Ogre::Vector3 lower_lim;
		Ogre::Vector3 upper_lim;
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
	Tube(WorldPtr world, ConstructionInfo const &info);

	/// @brief Destructor
	~Tube(void);

	/// @brief
	void setStiffness(vl::scalar stiffness);

	vl::scalar getStiffness(void) const
	{ return _stiffness; }

	void setDamping(vl::scalar damping);

	vl::scalar getDamping(void) const
	{ return _damping; }

	/// @brief get the size of a single simulated rigid body
	/// hard coded for now.
	vl::scalar getElementSize(void) const
	{ return _element_size; }

	/// @brief get the radius of the tube
	/// hard coded for now
	vl::scalar getRadius(void) const
	{ return _tube_radius; }

	vl::scalar getLength(void) const
	{ return _length; }

	vl::scalar getMass(void) const
	{ return _mass; }

	std::string const &getMaterial(void) const
	{ return _material_name; }

	/// @todo does not work yet as it does not update the entities.
	void setMaterial(std::string const &material);

	void hide(void);

	void show(void);

private :
	static size_t n_tubes;

	void _createConstraints(vl::Transform const &start_frame, vl::Transform const &end_frame, vl::scalar elem_length);

	/// Helper methods for Graphics engine so we don't need to pass 
	/// all this information to the constructor, which would be pretty hard
	/// as we would need to pass it also to the World.
	void _createMesh(MeshManagerRefPtr mesh_manager);

	RigidBodyRefPtr _start_body;
	RigidBodyRefPtr _end_body;

	// Uses multiple bodies and constraints to build the tube
	// @todo should this rather use a map that maps the constraint
	// to a body or a custom structure?
	RigidBodyList _bodies;
	ConstraintList _constraints;
	
	vl::scalar _length;
	vl::scalar _stiffness;
	vl::scalar _damping;
	vl::scalar _element_size;
	vl::scalar _tube_radius;
	vl::scalar _mass;

	std::string _material_name;

	WorldPtr _world;

	bool _mesh_created;

};	// class Tube

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_TUBE_HPP
