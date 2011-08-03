/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file physics/tube.cpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#include "tube.hpp"

/// Necessary for registering all the bodies and constraints
#include "physics_world.hpp"
/// Necessary as we create new bodies for all the elements
#include "rigid_body.hpp"
/// Necessary for the string-dampers
#include "constraints.hpp"

/// Necessary for generating the mesh
#include "mesh_manager.hpp"
/// Necessary for attaching the generated mesh/meshes to the SceneGraph
#include "scene_node.hpp"
// Necessary for creating SceneNodes and retriving mesh manager
#include "scene_manager.hpp"

vl::physics::Tube::Tube(WorldPtr world, RigidBodyRefPtr start_body, RigidBodyRefPtr end_body,
		vl::scalar length, vl::scalar radius, vl::scalar mass)
	: _start_body(start_body)
	, _end_body(end_body)
	, _length(length)
	, _stiffness(0.9)
	, _element_size(0.3)
	, _tube_radius(radius)
	, _mass(mass)
	, _world(world)
	, _mesh_created(false)
{
	std::clog << "vl::physics::Tube::Tube" << std::endl;

	assert(_world);
	assert(_start_body && _end_body);

	/// @todo create the tube bodies using a user given length
	/// this is usually different than the length between start and end bodies
	/// but should never be less than.
	
	/// @todo check the distance between the two bodies
	uint16_t n_elements = std::ceil(_length/_element_size);

	Ogre::Vector3 bounds(_tube_radius*2, _tube_radius*2, _length/n_elements);
	BoxShapeRefPtr shape = BoxShape::create(bounds);
	Ogre::Vector3 inertia(1, 1, 1);
	std::clog << "Creating a tube with " << n_elements << " elements." << std::endl;
	for(uint16_t i = 0; i < n_elements; ++i)
	{
		std::stringstream name;
		name << "tube_element_" << i;
		// @todo fix the transformations for the bodies now they are on top of each other
		MotionState *ms = _world->createMotionState(vl::Transform(Ogre::Vector3(0, 1+i, 0)));

		RigidBodyRefPtr body = _world->createRigidBody(name.str(), _mass/n_elements, ms, shape, inertia);
		_bodies.push_back(body);
	}

	std::clog << "Created " << _bodies.size() << " rigid bodies for the tube." << std::endl;

	/// create string-damper constraints between the bodies
	
	/// Create the SceneNodes
	assert(start_body->getMotionState() && start_body->getMotionState()->getNode());
	_createMesh(start_body->getMotionState()->getNode()->getCreator()->getMeshManager());
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
vl::physics::Tube::_createMesh(MeshManagerRefPtr mesh_manager)
{
	std::clog << "vl::physics::Tube::_createMesh" << std::endl;

	if(_mesh_created)
	{ return; }

	/// create the graphics objects
	/// we need access to the MeshManager and SceneManager for this
	/// for now using simple cubes for the graphics meshes, later
	/// bones and a generated tube mesh.
	
	// Create the box element reused for all the bodies
	std::string const mesh_name("tube_element");
	mesh_manager->createCube(mesh_name, Ogre::Vector3(_tube_radius*2, _tube_radius*2, _length/_bodies.size()));

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
		name << mesh_name << "_" << index;
		MotionState *ms = (*iter)->getMotionState();
		assert(!ms->getNode());
		SceneNodePtr node = parent_node->createChildSceneNode(name.str());
		ms->setNode(node);
		EntityPtr ent = sm->createEntity(name.str(), mesh_name, true);
		node->attachObject(ent);

		++index;
	}

	std::clog << "Created " << index << " graphics objects for the tube." << std::endl;

	/// Mesh and Scene managers can be accessed using the start and end bodies
	/// they need to have a MotionState attached to them and a SceneNode
	/// attached to that for this to work.

	_mesh_created = true;
}
