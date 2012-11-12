/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_world.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	World object that deals with Kinematic bodies and constraints.
 *	Provides the same interface as physics world but using our own
 *	kinematic solver.
 */

#include "kinematic_world.hpp"

#include "kinematic_body.hpp"

#include "scene_node.hpp"

#include "base/exceptions.hpp"

#include "constraints.hpp"

// Necessary for collision detection
#include "game_manager.hpp"
#include "physics/physics_world.hpp"
#include "physics/rigid_body.hpp"
#include "physics/shapes.hpp"
#include "mesh_manager.hpp"

// Log leves
#include "logger.hpp"

/// ---------------------------- Global --------------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::KinematicWorld const &world)

{
	os << "KinematicWorld with " << world.getBodies().size() << " bodies and " 
		<< world.getConstraints().size() << " constraints. " 
		<< *world._graph << std::endl;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::KinematicBodyList const &bodies)
{
	os << "Kinematic body list : length = " << bodies.size() << std::endl;

	return os;
}


/// ---------------------------- KinematicWorld ------------------------------
vl::KinematicWorld::KinematicWorld(GameManager *man)
	: _collision_detection_on(false)
	, _graph(new animation::Graph)
	, _game(man)
{
}

vl::KinematicWorld::~KinematicWorld(void)
{
	 _constraints.clear();
	 _bodies.clear();
	 _graph.reset();
}

void
vl::KinematicWorld::step(vl::time const &t)
{
	_progress_constraints(t);

	// Update all the nodes
	_graph->_update();

	/// Copy transformations to Motion states
	for(KinematicBodyList::iterator iter = _bodies.begin();
		iter != _bodies.end(); ++iter )
	{
		(*iter)->_update();
	}
}

void
vl::KinematicWorld::finalise(void)
{
	/// Copy back transformations from MotionStates after collision detection
	for(KinematicBodyList::iterator iter = _bodies.begin();
		iter != _bodies.end(); ++iter )
	{
		(*iter)->_update();
	}
}

void
vl::KinematicWorld::removeAll(void)
{
	std::clog << "vl::KinematicWorld::removeAll" << std::endl;
	// Constraints need to be removed first
	// This should make graph nodes dangling
	// so they can be added to the world again using different constraints.
	// Because these are ref counted we let the destructor handle them.
	//ConstraintList _constraints;
	_constraints.clear();

	// Removing bodies should remove the animation::Nodes completely
	// Because these are ref counted we let the destructor handle them.
	//KinematicBodyList _bodies;
	_bodies.clear();
	
	// We need to clean up the Graph because some links are not mapped to constraints.
	// This is because we create a Link for every Node and then
	// when constraint is created it will retrieve that Link so some Links
	// have not been mapped to constraints.
	_graph->getRoot()->removeChildren();

	// Just check that we don't have any links/nodes left but leave the
	// graph which can be reused.
	//animation::GraphRefPtr _graph;
	std::clog << *_graph << std::endl;
	// This assert fails always, why?
	// because the test project has collision detection which saves
	// the pointers to kinematic bodies for updates.
	//assert(_graph->getRoot()->isLeaf());

	// Reset parameters
	enableCollisionDetection(false);
}

vl::KinematicBodyRefPtr
vl::KinematicWorld::getKinematicBody(std::string const &name) const
{
	for(KinematicBodyList::const_iterator iter = _bodies.begin();
		iter != _bodies.end(); ++iter)
	{
		if((*iter)->getName() == name)
		{ return *iter; }
	}

	return KinematicBodyRefPtr();
}

vl::KinematicBodyRefPtr
vl::KinematicWorld::getKinematicBody(vl::SceneNodePtr sn) const
{
	if(!sn)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	for(KinematicBodyList::const_iterator iter = _bodies.begin();
		iter != _bodies.end(); ++iter)
	{
		if((*iter)->getName() == sn->getName())
		{ return *iter; }
	}

	return KinematicBodyRefPtr();
}

vl::KinematicBodyRefPtr
vl::KinematicWorld::createKinematicBody(vl::SceneNodePtr sn)
{
	if(!sn)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	KinematicBodyRefPtr body = getKinematicBody(sn);
	if(!body)
	{
		std::clog << "Creating kinematic body for : " << sn->getName() << std::endl;
		physics::MotionState *ms = physics::MotionState::create(sn->getWorldTransform(), sn);
		animation::NodeRefPtr node = _createNode(ms->getWorldTransform());
		body.reset(new KinematicBody(sn->getName(), this, node, ms));
		assert(body);
		_bodies.push_back(body);

		if(_collision_detection_on)
		{
			if( sn->getName().find("cb_") != std::string::npos)
			{
				std::clog << "Auto creating a collision model for " << sn->getName() << std::endl;
				_create_collision_body(body); 
			}
		}
	}

	return body;
}

void
vl::KinematicWorld::_create_collision_body(KinematicBodyRefPtr body)
{
	try {
		assert(_game);
		/// enable physics if not already
		/// Create a kinematic rigid body and add it physics world
		/// use same MotionStates for both kinematic and rigid body
		_game->enablePhysics(true);
		// We assume that the mesh name is the same as the SceneNode
		vl::MeshRefPtr mesh = _game->getMeshManager()->loadMesh(body->getName());
		physics::ConvexHullShapeRefPtr shape = physics::ConvexHullShape::create(mesh);
		physics::RigidBody::ConstructionInfo info(body->getName(), 0, body->getMotionState(), shape, Ogre::Vector3(0, 0, 0), true);
		physics::RigidBodyRefPtr physics_body = _game->getPhysicsWorld()->createRigidBodyEx(info);
		// necessary to add callback so the kinematic object updates 
		// the the collision model.
		body->addListener(boost::bind(&physics::RigidBody::setWorldTransform, physics_body, _1));
		// Used by the collision detection to pop last transformation.
		physics_body->setUserData(body.get());
	}
	catch(vl::exception const &e)
	{
		std::clog << "Exception thrown when creating collision model for " << body->getName() << std::endl;
	}
}

void
vl::KinematicWorld::removeKinematicBody(vl::KinematicBodyRefPtr body)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

vl::ConstraintRefPtr
vl::KinematicWorld::createConstraint(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, vl::Transform const &trans)
{
	if(body0 == body1)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Can't create constraint between object and itself.")); }
	if(!body0 || !body1)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Can't create constraint without second body.")); }

	vl::ConstraintRefPtr c;

	vl::Transform fA = body0->transformToLocal(trans);
	vl::Transform fB = body1->transformToLocal(trans);

	return createConstraint(type, body0, body1, fA, fB);
}

vl::ConstraintRefPtr
vl::KinematicWorld::createConstraint(std::string const &type, vl::KinematicBodyRefPtr body0, 
		vl::KinematicBodyRefPtr body1, vl::Transform const &frameInA, vl::Transform const &frameInB, std::string const &name)
{
	if(body0 == body1)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Can't create constraint between object and itself.")); }
	if(!body0 || !body1)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Can't create constraint without second body.")); }

	std::string type_name(type);
	vl::to_lower(type_name);

	vl::ConstraintRefPtr c;
	if(type_name == "slider")
	{
		c = SliderConstraint::create(name, body0, body1, frameInA, frameInB);
	}
	else if(type_name == "hinge")
	{
		c = HingeConstraint::create(name, body0, body1, frameInA, frameInB);
	}
	else if(type_name == "fixed")
	{
		c = FixedConstraint::create(name, body0, body1, frameInA, frameInB);
	}
	
	// Do not allow empties, should have some real exception types for it though
	if(!c)
	{
		std::cout << vl::CRITICAL << "Constraint type not valid." << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception());
	}

	_addConstraint(c);

	return c;
}


vl::ConstraintRefPtr
vl::KinematicWorld::createConstraint(std::string const &type, vl::KinematicBodyRefPtr body0, 
		vl::KinematicBodyRefPtr body1, vl::Transform const &frameInA, vl::Transform const &frameInB)
{
	return createConstraint(type, body0, body1, frameInA, frameInB, vl::generate_random_string());
}

void
vl::KinematicWorld::removeConstraint(ConstraintRefPtr constraint)
{
	for(std::vector<vl::ConstraintRefPtr>::iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		if(*iter == constraint)
		{
			_constraints.erase(iter);
			// @todo remove the link and Node also from the map and graph
			// at the moment they are in the destructor which is not good
			break;
		}
	}
}

bool
vl::KinematicWorld::hasConstraint(vl::ConstraintRefPtr constraint) const
{
	return( std::find(_constraints.begin(), _constraints.end(), constraint) != _constraints.end() );
}

bool
vl::KinematicWorld::hasConstraint(std::string const &name) const
{
	return (getConstraint(name) != ConstraintRefPtr());
}

vl::ConstraintRefPtr
vl::KinematicWorld::getConstraint(std::string const &name) const
{
	for(ConstraintList::const_iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		if((*iter)->getName() == name)
		{
			return *iter;
		}
	}

	return ConstraintRefPtr();
}

vl::ConstraintList const &
vl::KinematicWorld::getConstraints(void) const
{
	return _constraints;
}

vl::KinematicBodyList const &
vl::KinematicWorld::getBodies(void) const
{
	return _bodies;
}

void
vl::KinematicWorld::enableCollisionDetection(bool enable)
{
	if(_collision_detection_on != enable)
	{
		for(KinematicBodyList::iterator iter = _bodies.begin(); iter != _bodies.end(); ++iter)
		{
			(*iter)->enableCollisions(enable);
		}

		_collision_detection_on = enable;
	}	
}


void
vl::KinematicWorld::_addConstraint(vl::ConstraintRefPtr constraint)
{
	if(!constraint)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

	// checks
	if(hasConstraint(constraint))
	{ return; }

	// @todo this will add multiple constraints to same bodies which does not
	// work with just the FK solver
	// multiple parents, not yet working

	animation::NodeRefPtr parent;
	animation::NodeRefPtr child;

	// Using either already existing parent, for kinematic chains
	// or a new parent under root Node for new chains
	KinematicBodyList::iterator iter = std::find(_bodies.begin(), _bodies.end(), constraint->getBodyA());
	if(iter == getBodies().end())
	{
		std::stringstream err_msg;
		err_msg << "No Kinematic node \"" << constraint->getBodyA()->getName()
			<< "\" exists : should never happen.";
		std::cout << vl::CRITICAL << err_msg.str() << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg.str()));
	}
	// Does this work or should we use Kinematic Body here?
	parent = (*iter)->getAnimationNode();

	iter = std::find(_bodies.begin(), _bodies.end(), constraint->getBodyB());
	if(iter == getBodies().end())
	{
		std::stringstream err_msg;
		err_msg << "No Kinematic node \"" << constraint->getBodyB()->getName()
			<< "\" exists : should never happen.";
		std::cout << vl::CRITICAL << err_msg.str() << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg.str()));
	}
	child = (*iter)->getAnimationNode();
	assert(parent && child);

	animation::LinkRefPtr link = child->getParent();

	// If bodyB has already a parent it can't be added
	assert(link);

	if(link->getParent() != _graph->getRoot())
	{
		std::cout << vl::CRITICAL << constraint->getBodyB()->getName() 
			<< " already has a parent and it's not root so the constraint to "
			<< constraint->getBodyA()->getName() << " can't be added." 
			<< std::endl;

		return;
	}

	// @todo throw
	assert(parent && child);
	assert(link);

	link->setParent(parent);
	// The correct transformation is set by the constraint
	constraint->_setLink(link);

	// @todo Not the correct place for this but for testing
	// fixed issues with Node transformation beign returned to the one
	// used with a previous parent.
	// Now these should be moved to correct places in animation framework.
	parent->setInitialState();
	child->setInitialState();

	_constraints.push_back(constraint);
}

vl::animation::NodeRefPtr
vl::KinematicWorld::_createNode(vl::Transform const &initial_transform)
{
	animation::NodeRefPtr node(new animation::Node(initial_transform));
	animation::LinkRefPtr link(new animation::Link);
	link->setParent(_graph->getRoot());
	link->setChild(node);

	return node;
}

void
vl::KinematicWorld::_progress_constraints(vl::time const &t)
{
	// First phase
	/// Progress the constraints
	for(ConstraintList::iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{ 
		(*iter)->_solve(t);
	}

	/// Second phase needs an IK solver using the Jacobian to progress
	/// the constraints... not implemented yet.
}
