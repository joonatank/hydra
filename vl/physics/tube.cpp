/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/tube.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
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
#include "entity.hpp"

size_t vl::physics::Tube::n_tubes = 0;

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::Tube const &tube)
{
	// @todo we don't have an identifier for the tube
	os << "Tube";

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::TubeList const &tubes)
{
	os << "TubeList with " << tubes.size() << " tubes.";
	for(TubeList::const_iterator iter = tubes.begin(); iter != tubes.end(); ++iter)
	{
		// @todo we don't have an identifier for the tube
	}

	return os;
}

vl::physics::Tube::Tube(WorldPtr world, SceneManagerPtr sm, Tube::ConstructionInfo const &info)
	: _start_body(info.start_body)
	, _end_body(info.end_body)
	, _length(info.length)
	, _stiffness(info.stiffness)
	, _damping(info.damping)
	, _element_size(info.element_size)
	, _tube_radius(info.radius)
	, _mass(0)
	, _body_damping(info.body_damping)
	, _spring(info.spring)
	, _disable_collisions(info.disable_collisions)
	, _disable_internal_collisions(info.disable_internal_collisions)
	, _use_instancing(info.use_instancing)
	, _inertia(Vector3(0, 0, 0))
	, _lower_lim(info.lower_lim)
	, _upper_lim(info.upper_lim)
	, _fixing_lower_lim(info.fixing_lower_lim)
	, _fixing_upper_lim(info.fixing_upper_lim)
	, _material_name(info.material_name)
	, _start_body_frame(info.start_body_frame)
	, _end_body_frame(info.end_body_frame)
	, _world(world)
	, _scene(sm)
{
	assert(_world);

	/// Check for validity
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

	vl::scalar mass_per_meter = 1;
	// Using mass per meter instead of mass
	if(info.mass_per_meter > 0)
	{ mass_per_meter = info.mass_per_meter; }

	// calculate the mass
	_mass = info.mass_per_meter*_length;
	std::clog << "Calculated mass : " << _mass << std::endl;

	vl::scalar elem_mass = _mass/(_length/_element_size);
	std::clog << "Calculated element mass = " << elem_mass << std::endl;

	// calculate inertia for tube

	// tube is created towards y direction
	_inertia.x = 1.0/12.0*elem_mass*(3*_tube_radius*_tube_radius + _element_size*_element_size);
	_inertia.z = _inertia.x;
	_inertia.y = elem_mass*_tube_radius*_tube_radius/2.0;
	_inertia = info.inertia_factor*_inertia;
	std::clog << "Calculated inertia : for each element inertia = " << _inertia << std::endl;

	if(info.bending_radius > 0)
	{
		// using formula for circle sector
		// element is the sector, we need height to calculate the
		// angle using sin
		vl::scalar r = info.bending_radius;
		vl::scalar c = _element_size;
		vl::scalar height = r - std::sqrt(r*r - c*c/4);
		std::clog << "height = " << height << " hypotenusa = " << c << std::endl;
		vl::scalar alfa = std::asin(height/c);

		if(vl::isnormal(alfa))
		{
			_lower_lim = Ogre::Vector3(-alfa, -alfa, -0);
			// Upper and lower limits are same except for the sign
			_upper_lim = -_lower_lim;
			std::clog << "Calculated limits : lower = " << _lower_lim << " upper = " 
				<< _upper_lim 
				<< " : angle " << Ogre::Radian(alfa).valueDegrees() << " deg." << std::endl;
		}
		else
		{
			std::clog << "ERROR : "
				<< "Tryed to calculate the limits from bending angle but got invalid result." << std::endl;
		}
	}
}

vl::physics::Tube::~Tube(void)
{
	/// Remove constraints and bodies from world and destroy them
	removeFromWorld();
	_fixing_bodies.clear();
	_external_fixings.clear();
	_constraints.clear();
	_bodies.clear();

	/// @todo should remove and destroy all used SceneNodes
	/// and MovableObjects
	/// We need to keep a list of them for easy removal.
	for(EntityList::iterator iter = _entities.begin(); 
		iter != _entities.end(); ++iter)
	{
		_scene->destroyMovableObject(*iter);
	}
	_entities.clear();

	for(SceneNodeList::iterator iter = _nodes.begin(); 
		iter != _nodes.end(); ++iter)
	{
		_scene->destroySceneNode(*iter);
	}
	_nodes.clear();
}

void
vl::physics::Tube::removeFromWorld(void)
{
	std::clog << "vl::physics::Tube::removeFromWorld" << std::endl;
	// Can't do isInWorld() checking because this can be called from destructor
	// and the weak_ptr is not valid anymore

	// Fixing bodies are created outside of the Tube, so we don't remove them
	// _fixing_bodies
	
	// External fixings are constraints created by us so we destroy them
	// We can use the original lists here because they are not modified
	// here.
	for(ConstraintList::iterator iter = _external_fixings.begin(); 
		iter != _external_fixings.end(); ++iter)
	{
		_world->removeConstraint(*iter);
	}

	// Constraints are created by us
	for(ConstraintList::iterator iter = _constraints.begin(); 
		iter != _constraints.end(); ++iter)
	{
		_world->removeConstraint(*iter);
	}

	// Bodies are created by us
	for(RigidBodyList::iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
	{
		_world->removeRigidBody(*iter);
	}

	// @todo destroy SceneNodes and Entities
}

bool
vl::physics::Tube::isInWorld(void) const
{
	return _world->hasTube(shared_from_this());
}

void
vl::physics::Tube::setSpringStiffness(vl::scalar stiffness)
{
	if(_spring && _stiffness != stiffness)
	{
		for(ConstraintList::iterator iter = _constraints.begin()+1;
			iter != _constraints.end()-1; ++iter)
		{
			SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(*iter);
			
			for(uint16_t i = 3; i < 6; ++i)
			{ spring->setStiffness(i, stiffness); }
		}
	}

	_stiffness = stiffness;
}

void
vl::physics::Tube::setSpringDamping(vl::scalar damping)
{
	if(_spring && _damping != damping)
	{
		for(ConstraintList::iterator iter = _constraints.begin()+1;
			iter != _constraints.end()-1; ++iter)
		{
			SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(*iter);
			
			for(uint16_t i = 3; i < 6; ++i)
			{ spring->setDamping(i, damping); }
		}
	}

	_damping = damping;
}

void
vl::physics::Tube::setDamping(vl::scalar damping)
{
	if(_body_damping != damping)
	{
		for(RigidBodyList::iterator iter = _bodies.begin();
			iter != _bodies.end(); ++iter)
		{
			(*iter)->setDamping(damping, damping);
		}

		_body_damping = damping;
	}
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
	if(_lower_lim != lim)
	{
		_lower_lim = lim;

		for(ConstraintList::iterator iter = _constraints.begin()+1;
			iter != _constraints.end()-1; ++iter)
		{
			SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(*iter);
			spring->setAngularLowerLimit(lim);
		}
	}
}

void
vl::physics::Tube::setUpperLim(Ogre::Vector3 const &lim)
{
	if(_upper_lim != lim)
	{
		_upper_lim = lim;

		for(ConstraintList::iterator iter = _constraints.begin()+1;
			iter != _constraints.end()-1; ++iter)
		{
			SixDofConstraintRefPtr spring = boost::dynamic_pointer_cast<SixDofConstraint>(*iter);
			spring->setAngularUpperLimit(lim);
		}
	}
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
vl::physics::Tube::setShowBoundingBoxes(bool show)
{
	for(RigidBodyList::const_iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
	{
		MotionState *ms = (*iter)->getMotionState();
		assert(ms->getNode());
		ms->getNode()->setShowBoundingBox(show);
	}
}

bool
vl::physics::Tube::isShowBoundingBoxes(void) const
{
	if(_bodies.size() > 0 && _bodies.at(0)->getMotionState() && _bodies.at(0)->getMotionState()->getNode())
	{ return _bodies.at(0)->getMotionState()->getNode()->getShowBoundingBox(); }
	
	return false;
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

	if(_fixing_bodies.find(length) != _fixing_bodies.end())
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Already has a fixing at the point."));
	}

	_fixing_bodies[length] = body;


	// Just record the fixing poinsts as the tube has not yet been created
	if(!_bodies.empty())
	{
		_add_fixing_point(body, length);
	}
}

void
vl::physics::Tube::removeFixingPoint(RigidBodyRefPtr body)
{
	for(ConstraintList::iterator iter = _external_fixings.begin(); 
		iter != _external_fixings.end(); ++iter)
	{
		if((*iter)->getBodyA() == body || (*iter)->getBodyB() == body)
		{
			_world->removeConstraint(*iter);
			_external_fixings.erase(iter);
			return;
		}
	}

	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No such constraint."));
}

void
vl::physics::Tube::create(void)
{
	if(!_bodies.empty())
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Tube already created"));
	}

	/// General parameters that stay constant for the whole tube
	uint16_t n_elements = std::ceil(_length/_element_size);
	vl::scalar elem_length = _length/n_elements;
	// Create the tube along z-axis
	Ogre::Vector3 bounds(_tube_radius*2, _tube_radius*2, elem_length);
	_shape = BoxShape::create(bounds);
	vl::scalar elem_mass = _mass/n_elements;
	std::clog << "Creating a tube with " << n_elements << " elements." << std::endl
		<< " and with " << _fixing_bodies.size() << " fixing points." << std::endl;

	/// Set parameters to start and end bodies
	_start_body->setUserControlled(true);
	_start_body->setDamping(_body_damping, _body_damping);
	_end_body->setUserControlled(true);
	_end_body->setDamping(_body_damping, _body_damping);

	std::map<vl::scalar, RigidBodyRefPtr>::iterator fixing_iter = _fixing_bodies.begin();

	// Create the bodies
	// between the fixing points
	RigidBodyRefPtr body0 = _start_body;
	RigidBodyRefPtr body1 = _end_body;
	size_t count = 0;
	for(uint16_t i = 0; i < n_elements; ++i)
	{
		// Direction between the bodies
		// @todo change to use consecutive bodies
		// we need to calculate the current position based on last fixing and
		// the direction from that to the next fixing
		if(fixing_iter != _fixing_bodies.end())
		{
			// Still valid body
			if(fixing_iter->first > i*elem_length)
			{
				body1 = fixing_iter->second;
			}
			// Move to next fixing point
			else
			{
				// We assume that no fixing points are so close to each other 
				// that we would need to move the iterator multiple times.
				++fixing_iter;
				if(fixing_iter != _fixing_bodies.end())
				{ body1 = fixing_iter->second; }
				else
				{ body1 = _end_body; }
				count = 0;
			}
		}

		// Find the start and end transformations
		// start transformation is the transformation of the last body
		// end transformation is the transformation of the next fixing point 
		// or a helper if one is defined
		vl::Transform wt_start(body0->getWorldTransform());
		vl::Transform wt_end(body1->getWorldTransform());
		
		// Using the last body instead of calculating real direction vector
		// does better job at keeping the tubes from tangeling when they backtrack
		// real method would be to take the limits into account when calculating
		// the direction.
		// update
		Ogre::Vector3 length = wt_end.position - wt_start.position;
		Ogre::Vector3 dir = length;
		dir.normalise();
	
		std::stringstream name;
		name << "tube_" << n_tubes << "_element_" << i;

		// This seems to work fine for everything except when we are moving backwards
		// but the limits are constricting so that we need to make a u-turn.
		Ogre::Vector3 v = dir * elem_length;
		Ogre::Vector3 pos = wt_start.position;
		if(v.length() < length.length())
		{ pos += v; }
		else
		{ pos += length; }

		// @todo this should maybe be modified to combine both the direction 
		// and the orginal orientation
		Ogre::Quaternion orient = body0->getWorldTransform().quaternion;

		Transform ms_t(pos, orient);
		MotionState *ms = _world->createMotionState(ms_t);
		RigidBodyRefPtr body = _world->createRigidBody(name.str(), elem_mass, ms, _shape, _inertia);
		_bodies.push_back(body);
		
		// Needs to be here so that it will not be deactivated
		body->setUserControlled(true);
		body->setDamping(_body_damping, _body_damping);
		body->disableCollisions(_disable_collisions);
		body0 = body;

		// Some of the casting might go wrong 
		// so lets assert that we still have valid state
		assert(ms == body->getMotionState());
		assert(!body->getMotionState()->getNode());

		++count;
	}

	// For integration to our kinematics solver with collision detection
	// start and end bodies can't have collision detection enabled
	// real fix would be to automatically position them outside of collision models
	// or use the collision engine for it but alas not implemented at this point.
	_start_body->disableCollisions(true);
	_end_body->disableCollisions(true);

	if(body0->isCollisionsDisabled())
	{
		std::clog << "Rigid body collisions disabled." << std::endl;
	}

	// Create the constraints
	_createConstraints(_start_body_frame, _end_body_frame, elem_length);

	std::clog << "Created " << _bodies.size() << " rigid bodies and "
		<< _constraints.size() << " constraints for the tube." << std::endl;

	// Create the fixing point constraints
	for(std::map<vl::scalar, RigidBodyRefPtr>::iterator iter = _fixing_bodies.begin();
		iter != _fixing_bodies.end(); ++iter)
	{
		_add_fixing_point(iter->second, iter->first);
	}

	assert(_scene);
	// Create the mesh
	_createMesh(_scene->getMeshManager());


	// Increase the static counter
	++n_tubes;
}

vl::physics::SixDofConstraintRefPtr
vl::physics::Tube::getStartFixing(void)
{
	return boost::dynamic_pointer_cast<SixDofConstraint>(*_constraints.begin());
}

vl::physics::SixDofConstraintRefPtr
vl::physics::Tube::getEndFixing(void)
{
	return boost::dynamic_pointer_cast<SixDofConstraint>(*(_constraints.end()-1));
}

vl::physics::SixDofConstraintRefPtr
vl::physics::Tube::getFixing(size_t index)
{
	return boost::dynamic_pointer_cast<SixDofConstraint>(_external_fixings.at(index));
}

size_t
vl::physics::Tube::getNFixings(void) const
{
	return _external_fixings.size();
}

vl::physics::ConstraintList const &
vl::physics::Tube::getFixings(void) const
{
	return _external_fixings;
}

/// ---------------------------------- Private -------------------------------
void
vl::physics::Tube::_createConstraints(vl::Transform const &start_frame, vl::Transform const &end_frame, vl::scalar elem_length)
{
	/// Create spring-damper constraints
	/// Maximum number of constraint is N+1 where N is the number of elements
	vl::Transform frameA(Ogre::Vector3(0, 0, elem_length/2));
	vl::Transform frameB = -frameA;

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
	std::clog << "vl::physics::Tube::_createMesh" << std::endl;
	/// create the graphics objects
	/// we need access to the MeshManager and SceneManager for this
	/// for now using simple cubes for the graphics meshes, later
	/// bones and a generated tube mesh.
	
	// Create the box element reused for all the bodies
	std::stringstream mesh_name;
	mesh_name << "tube_" << n_tubes << "_element";
	std::clog << "Copying size from physics shape : size = " << _shape->getSize();
	mesh_manager->createCube(mesh_name.str(), _shape->getSize());

	/// Create the SceneNodes
	if(!_start_body->getMotionState() || !_start_body->getMotionState()->getNode())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Invalid Motion state or node.")); }

	size_t index = 0;
	for(RigidBodyList::const_iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
	{
		/// Create the SceneNode and Entity
		std::stringstream name;
		name << mesh_name.str() << "_" << index;

		MotionState *ms = (*iter)->getMotionState();
		// check that there the MotionState is still valid
		assert(!ms->getNode());

		// We need to use root node for the physics engine to work correctly
		SceneNodePtr node = _scene->getRootSceneNode()->createChildSceneNode(name.str());
		ms->setNode(node);
		_nodes.push_back(node);

		EntityPtr ent = _scene->createEntity(name.str(), mesh_name.str(), true);
		ent->setInstanced(_use_instancing);
		// Force instanced material
		if(_use_instancing)
		{ _material_name = "tube/instanced"; }
		ent->setMaterialName(_material_name);
		node->attachObject(ent);
		_entities.push_back(ent);

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


void
vl::physics::Tube::_add_fixing_point(RigidBodyRefPtr body, vl::scalar length)
{
	// Find the closest body
	RigidBodyRefPtr tube_body;
	
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
	_setConstraint(constraint, _fixing_lower_lim, _fixing_upper_lim);
	body->setUserControlled(true);
	body->setDamping(_body_damping, _body_damping);

	_external_fixings.push_back(constraint);
}
