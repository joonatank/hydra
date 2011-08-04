/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file physics/tube.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef HYDRA_PHYSICS_TUBE_HPP
#define HYDRA_PHYSICS_TUBE_HPP

#include "math/types.hpp"

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
	Tube(WorldPtr world, RigidBodyRefPtr start_body, RigidBodyRefPtr end_body,
		vl::scalar length, vl::scalar radius, vl::scalar mass);

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

private :
	void _createConstraints(vl::scalar elem_length);

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

	WorldPtr _world;

	bool _mesh_created;

};	// class Tube

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_TUBE_HPP
