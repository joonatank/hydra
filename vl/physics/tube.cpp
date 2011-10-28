/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/tube.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "tube.hpp"

/// Necessary for registering all the bodies and constraints
#include "physics/physics_world.hpp"
/// Necessary as we create new bodies for all the elements
#include "physics/rigid_body.hpp"
/// Necessary for the string-dampers
#include "physics/physics_constraints.hpp"
/// Necessary for setting the MotionState on created Bodies
#include "physics/motion_state.hpp"
/// Necessary for creating collision shapes
#include "physics/shapes.hpp"

/// Necessary for generating the mesh
#include "mesh_manager.hpp"
/// Necessary for attaching the generated mesh/meshes to the SceneGraph
#include "scene_node.hpp"
// Necessary for creating SceneNodes and retriving mesh manager
#include "scene_manager.hpp"

size_t vl::physics::Tube::n_tubes = 0;

vl::physics::Tube::Tube(WorldPtr world, Tube::ConstructionInfo const &info)
	: _start_body(info.start_body)
	, _end_body(info.end_body)
	, _length(info.length)
	, _stiffness(info.stiffness)
	, _damping(info.damping)
	, _element_size(info.element_size)
	, _tube_radius(info.radius)
	, _mass(info.mass)
	, _body_damping(info.body_damping)
	, _spring(info.spring)
	, _disable_internal_collisions(info.disable_collisions)
	, _lower_lim(info.lower_lim)
	, _upper_lim(info.upper_lim)
	, _fixing_lower_lim(info.fixing_lower_lim)
	, _fixing_upper_lim(info.fixing_upper_lim)
	, _world(world)
	, _material_name(info.material_name)
{
	assert(_world);

	/// Check for validity
	if(_mass < 0)
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Can't create RigidBodies with negative mass"));
	}
	else if(_mass == 0)
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Tube with zero mass is meaningless."));
	}
	if(_element_size <= 0)
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Tube element size can't be zero or negative."));
	}
	if(!_start_body)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Tube start body is mandatory."));
	}
	if(!_end_body)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Tube end body is mandatory."));
	}

	/// @todo create the tube bodies using a user given length
	/// this is usually different than the length between start and end bodies
	/// but should never be less than.
	
	/// @todo check the distance between the two bodies
	uint16_t n_elements = std::ceil(_length/_element_size);
	vl::scalar elem_length = _length/n_elements;
	Ogre::Vector3 bounds(_tube_radius, _tube_radius, elem_length/2);
	BoxShapeRefPtr shape = BoxShape::create(bounds);
	Ogre::Vector3 inertia(info.inertia);
	vl::scalar elem_mass = _mass/n_elements;
	std::clog << "Creating a tube with " << n_elements << " elements." << std::endl;

	Ogre::Vector3 from_start_to_end = _end_body->getWorldTransform().position - _start_body->getWorldTransform().position;
	from_start_to_end.normalise();
	std::clog << "start body to end body vector : " << from_start_to_end << std::endl;
	
	/// Set parameters to start and end bodies
	_start_body->setUserControlled(true);
	_start_body->setDamping(_body_damping, _body_damping);
	_end_body->setUserControlled(true);
	_end_body->setDamping(_body_damping, _body_damping);
	
	for(uint16_t i = 0; i < n_elements; ++i)
	{
		std::stringstream name;
		name << "tube_" << n_tubes << "_element_" << i;
		// @todo fix the transformations for the bodies now they are on top of each other
		// Motions state transformation is correct, tested without mass it creates a nice string
		// to the world zero that goes upwards.
		// Of course we should have the string going in the z-axis
		// Of course we should have the bodies created between start and end body
		Ogre::Vector3 pos = _start_body->getWorldTransform().position 
			+ from_start_to_end*((i-n_elements/2)*elem_length);
		Ogre::Quaternion orient(1, 0, 0, 0); //(0.7071, 0.7071, 0, 0);
		Transform ms_t(pos, orient);
		MotionState *ms = _world->createMotionState(ms_t);
		// @todo should check what axis is up and so on, set mass and inertia to zero for testing
		RigidBodyRefPtr body = _world->createRigidBody(name.str(), elem_mass, ms, shape, inertia);
		_bodies.push_back(body);
		
		// Needs to be here so that it will not be deactivated
		body->setUserControlled(true);
		body->setDamping(_body_damping, _body_damping);
	}

	_createConstraints(info.start_body_frame, info.end_body_frame, elem_length);

	std::clog << "Created " << _bodies.size() << " rigid bodies and "
		<< _constraints.size() << " constraints for the tube." << std::endl;

	/// Create the SceneNodes
	assert(_start_body->getMotionState() && _start_body->getMotionState()->getNode());
	_createMesh(_start_body->getMotionState()->getNode()->getCreator()->getMeshManager());
	
	++n_tubes;
}

vl::physics::Tube::~Tube(void)
{
	/// @todo should remove constraints and bodies from world
	/// and destroy them
	/// @todo should remove and destroy all used SceneNodes
}

void
vl::physics::Tube::setSpringStiffness(vl::scalar stiffness)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());

	// @todo needs to reconfigure the constraints
	_stiffness = stiffness;
}

void
vl::physics::Tube::setSpringDamping(vl::scalar damping)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());

	// @todo needs to reconfigure the constraints
	_damping = damping;
}

void
vl::physics::Tube::setDamping(vl::scalar damping)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());

	// @todo needs to reconfigure the constraints
	_body_damping = damping;
}

void
vl::physics::Tube::setMass(vl::scalar mass)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

void
vl::physics::Tube::setMaterial(std::string const &material)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());

	_material_name = material;
}

void
vl::physics::Tube::setLowerLim(Ogre::Vector3 const &lim)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	
	_lower_lim = lim;
}

void
vl::physics::Tube::setUpperLim(Ogre::Vector3 const &lim)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());

	_upper_lim = lim;
}

void
vl::physics::Tube::hide(void)
{
	for(RigidBodyList::const_iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
	{
		MotionState *ms = (*iter)->getMotionState();
		assert(ms->getNode());
		ms->getNode()->hide();
	}
}

void
vl::physics::Tube::show(void)
{
	for(RigidBodyList::const_iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
	{
		MotionState *ms = (*iter)->getMotionState();
		assert(ms->getNode());
		ms->getNode()->show();
	}
}

void
vl::physics::Tube::setEquilibrium(void)
{
	if(!_spring)
	{ return; }

	for(ConstraintList::iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(*iter);
		spring->setEquilibriumPoint();
	}
}

void
vl::physics::Tube::addFixingPoint(RigidBodyRefPtr body, vl::scalar length)
{
	std::clog << "vl::physics::Tube::addFixingPoint" << std::endl;

	if(!body)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Body where to fix is mandatory."));
	}

	RigidBodyRefPtr tube_body;

	// Find the closest body
	if(length < 0)
	{
		tube_body = _findBodyByPosition(body->getWorldTransform().position);
	}
	else
	{
		tube_body = _findBodyByLength(length);
	}

	if(!tube_body)
	{
		std::string msg = "Finding a RigidBody failed.";
		std::clog << "Tube::addFixingPoint : " << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc(msg));
	}

	// Create the fixed constraint
	// @todo fix the frames, we should have the option to pass either local or world frame here
	vl::Transform frameA, frameB;
	SixDofConstraintRefPtr constraint = SixDofConstraint::create(body, tube_body, frameA, frameB, true);

	// @todo this should be changed to use _setConstraint
	constraint->setLinearLowerLimit(Ogre::Vector3::ZERO);
	constraint->setLinearUpperLimit(Ogre::Vector3::ZERO);
	constraint->setAngularLowerLimit(_fixing_lower_lim);
	constraint->setAngularUpperLimit(_fixing_upper_lim);
	body->setUserControlled(true);
	body->setDamping(_body_damping, _body_damping);

	_world->addConstraint(constraint, _disable_internal_collisions);
	_external_fixings.push_back(constraint);
}

/// @todo even though this is implemented here physics::World::removeConstraint is not
/// so this method is completely untested and unusable.
void
vl::physics::Tube::removeFixingPoint(RigidBodyRefPtr body)
{
	std::clog << "vl::physics::Tube::removeFixingPoint" << std::endl;

	for(ConstraintList::iterator iter = _external_fixings.begin(); 
		iter != _external_fixings.end(); ++iter)
	{
		if((*iter)->getBodyA() == body || (*iter)->getBodyB() == body)
		{
			std::clog << "Found constraint and removing it." << std::endl;
			_world->removeConstraint(*iter);
			_external_fixings.erase(iter);
			return;
		}
	}

	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No such constraint."));
}


/// ---------------------------------- Private -------------------------------
void
vl::physics::Tube::_createConstraints(vl::Transform const &start_frame, vl::Transform const &end_frame, vl::scalar elem_length)
{
	/// Create spring-damper constraints
	/// Maximum number of constraint is N+1 where N is the number of elements
	vl::Transform frameA(Ogre::Vector3(0, elem_length/2, 0));
	vl::Transform frameB = -frameA;
//	frameA.quaternion = Ogre::Quaternion(0.7071, 0.7071, 0, 0);
//	frameB.quaternion = Ogre::Quaternion(0.7071, 0.7071, 0, 0);
	if(_start_body && _bodies.size() > 0)
	{
		ConstraintRefPtr constraint = SixDofConstraint::create(_start_body, _bodies.front(), start_frame, frameB, true);
		_setConstraint(constraint, _fixing_lower_lim, _fixing_upper_lim);
		_constraints.push_back(constraint);
	}

	for(size_t i = 1; i < _bodies.size(); ++i)
	{	
		ConstraintRefPtr constraint;

		// first element
		constraint = SixDofConstraint::create(_bodies.at(i-1), _bodies.at(i), frameA, frameB, true);
		_setConstraint(constraint, _lower_lim, _upper_lim);
		_constraints.push_back(constraint);
	}
	
	// Last element
	if(_end_body && _bodies.size() > 0)
	{
		ConstraintRefPtr constraint = SixDofConstraint::create(_bodies.back(), _end_body, frameA, end_frame, true);
		// Configure constraint
		_setConstraint(constraint, _fixing_lower_lim, _fixing_upper_lim);
		_constraints.push_back(constraint);
	}
}

void
vl::physics::Tube::_setConstraint(ConstraintRefPtr constraint, 
	Ogre::Vector3 const &lower, Ogre::Vector3 const &upper)
{
	/// @todo set limits
	/// all translations should be fixed
	/// rotation around y-axis is allowed to a small extent
	/// the tube should have y-axis running through it in this scenario
	/// rotation around x and z axes are allowed to a large extent

	_world->addConstraint(constraint, _disable_internal_collisions);
		
	SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(constraint);
	assert(spring);
	spring->setLinearLowerLimit(Ogre::Vector3(0, 0, 0));
	spring->setLinearUpperLimit(Ogre::Vector3(0, 0, 0));
	spring->setAngularLowerLimit(lower);
	spring->setAngularUpperLimit(upper);

	/// @todo enable spring damper
	/// @todo set stiffness and damping
	// Enable spring for rotations around x, y and z
	if(_spring)
	{
		for(uint16_t i = 3; i < 6; ++i)
		{
			spring->enableSpring(i, true);
			spring->setStiffness(i, _stiffness);
			spring->setDamping(i, _damping);
		}
	}
}


void
vl::physics::Tube::_createMesh(MeshManagerRefPtr mesh_manager)
{
	/// create the graphics objects
	/// we need access to the MeshManager and SceneManager for this
	/// for now using simple cubes for the graphics meshes, later
	/// bones and a generated tube mesh.
	
	// Create the box element reused for all the bodies
	std::stringstream mesh_name;
	mesh_name << "tube_" << n_tubes << "_element";
	mesh_manager->createCube(mesh_name.str(), Ogre::Vector3(_tube_radius*2, _tube_radius*2, _length/_bodies.size()));

	SceneNodePtr parent_node = _start_body->getMotionState()->getNode();
	assert(parent_node);
	SceneManagerPtr sm = parent_node->getCreator();
	assert(sm);
	// We need to use root node for the physics engine to work correctly
	parent_node = sm->getRootSceneNode();
	assert(parent_node);
	size_t index = 0;
	for(RigidBodyList::const_iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
	{
		/// Create the SceneNode and Entity
		std::stringstream name;
		name << mesh_name.str() << "_" << index;
		MotionState *ms = (*iter)->getMotionState();
		assert(!ms->getNode());
		SceneNodePtr node = parent_node->createChildSceneNode(name.str());
		ms->setNode(node);
		EntityPtr ent = sm->createEntity(name.str(), mesh_name.str(), true);
		ent->setMaterialName(_material_name);
		node->attachObject(ent);

		++index;
	}

	std::clog << "Created " << index << " graphics objects for the tube." << std::endl;

	/// Mesh and Scene managers can be accessed using the start and end bodies
	/// they need to have a MotionState attached to them and a SceneNode
	/// attached to that for this to work.
}

vl::physics::RigidBodyRefPtr
vl::physics::Tube::_findBodyByLength(vl::scalar length)
{
	if(length < 0)
	{ return RigidBodyRefPtr(); }
	else if(length < _element_size)
	{ return _start_body; }
	else if(length > _length - _element_size)
	{ return _end_body; }
	else
	{
		size_t index = (size_t)(length/_element_size);
		return _bodies.at(index);
	}
}

vl::physics::RigidBodyRefPtr
vl::physics::Tube::_findBodyByPosition(Ogre::Vector3 const &pos)
{
	RigidBodyRefPtr closest;
	Ogre::Vector3 pos_diff;
	
	// Initialise with start and end bodies
	closest = _start_body;
	pos_diff = _start_body->getWorldTransform().position - pos;

	if((_end_body->getWorldTransform().position - pos).length() < pos_diff.length())
	{ closest = _end_body; }

	for(RigidBodyList::iterator iter = _bodies.begin(); iter != _bodies.end();
		++iter)
	{
		Ogre::Vector3 comp_pos_diff = (*iter)->getWorldTransform().position - pos;
		if(comp_pos_diff.length() < pos_diff.length())
		{
			closest = *iter;
			pos_diff = comp_pos_diff;
		}
	}

	std::clog << "Found RigidBody with error : " << pos_diff.length() 
		<< "m from the requested position." << std::endl;

	return closest;
}
