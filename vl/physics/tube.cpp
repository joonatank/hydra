/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file physics/tube.cpp
 *
 *	This file is part of Hydra a VR game engine.
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
	, _world(world)
	, _material_name(info.material_name)
	, _mesh_created(false)
{
	assert(_world);

	/// @todo create the tube bodies using a user given length
	/// this is usually different than the length between start and end bodies
	/// but should never be less than.
	
	/// @todo check the distance between the two bodies
	uint16_t n_elements = std::ceil(_length/_element_size);
	vl::scalar elem_length = _length/n_elements;
	Ogre::Vector3 bounds(_tube_radius, elem_length/2, _tube_radius);
	BoxShapeRefPtr shape = BoxShape::create(bounds);
	Ogre::Vector3 inertia(1, 1, 1);
	vl::scalar elem_mass = _mass/n_elements;
	std::clog << "Creating a tube with " << n_elements << " elements." << std::endl;
	for(uint16_t i = 0; i < n_elements; ++i)
	{
		std::stringstream name;
		name << "tube_" << n_tubes << "_element_" << i;
		// @todo fix the transformations for the bodies now they are on top of each other
		// Motions state transformation is correct, tested without mass it creates a nice string
		// to the world zero that goes upwards.
		// Of course we should have the string going in the z-axis
		// Of course we should have the bodies created between start and end body
		Transform ms_t(Ogre::Vector3(0, 2, (i-n_elements/2)*elem_length), Ogre::Quaternion(0.7071, 0.7071, 0, 0));
		MotionState *ms = _world->createMotionState(ms_t);
		// @todo should check what axis is up and so on, set mass and inertia to zero for testing
		RigidBodyRefPtr body = _world->createRigidBody(name.str(), elem_mass, ms, shape, inertia);
		_bodies.push_back(body);
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
}

void
vl::physics::Tube::setStiffness(vl::scalar stiffness)
{
	// @todo needs to reconfigure the constraints
	_stiffness = stiffness;
}

void
vl::physics::Tube::setDamping(vl::scalar damping)
{
	// @todo needs to reconfigure the constraints
	_damping = damping;
}

void
vl::physics::Tube::setMaterial(std::string const &material)
{
	_material_name = material;
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
		_constraints.push_back(constraint);
	}

	for(size_t i = 1; i < _bodies.size(); ++i)
	{	
		ConstraintRefPtr constraint;

		// first element
		constraint = SixDofConstraint::create(_bodies.at(i-1), _bodies.at(i), frameA, frameB, true);
		_constraints.push_back(constraint);
	}
	
	// Last element
	if(_end_body && _bodies.size() > 0)
	{
		ConstraintRefPtr constraint = SixDofConstraint::create(_bodies.back(), _end_body, frameA, end_frame, true);
		_constraints.push_back(constraint);
	}

	/// Configure all created constraints using same parameters
	for(ConstraintList::iterator iter = _constraints.begin(); iter != _constraints.end(); ++iter)
	{
		/// @todo set limits
		/// all translations should be fixed
		/// rotation around y-axis is allowed to a small extent
		/// the tube should have y-axis running through it in this scenario
		/// rotation around x and z axes are allowed to a large extent

		_world->addConstraint(*iter);
		
		SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(*iter);
		assert(spring);
		/// @todo set limits
		spring->setLinearLowerLimit(Ogre::Vector3(0, 0, 0));
		spring->setLinearUpperLimit(Ogre::Vector3(0, 0, 0));
		//spring->setAngularLowerLimit(Ogre::Vector3(0, 0, 0));
		//spring->setAngularUpperLimit(Ogre::Vector3(0, 0, 0));

		/// @todo enable spring damper
		/// @todo set stiffness and damping
		// Enable spring for rotations around x, y and z
		for(uint16_t i = 3; i < 6; ++i)
		{
			spring->enableSpring(i, true);
			spring->setStiffness(i, _stiffness);
			spring->setDamping(i, _damping);
		}
		//spring->setEquilibriumPoint();
	}
}

void
vl::physics::Tube::_createMesh(MeshManagerRefPtr mesh_manager)
{
	if(_mesh_created)
	{ return; }

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

	_mesh_created = true;
}
