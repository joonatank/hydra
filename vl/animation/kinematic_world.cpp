/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_world.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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

vl::KinematicWorld::KinematicWorld(GameManager *man)
	: _collision_detection_on(true)
	, _graph(new animation::Graph)
	, _game(man)
{
}

vl::KinematicWorld::~KinematicWorld(void)
{
}

void
vl::KinematicWorld::step(vl::time const &t)
{
	_progress_constraints(t);

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

		if(_collision_detection_on && sn->getName().substr(0, 3) == "cb_")
		{
			std::clog << "Auto creating a collision model for " << sn->getName() << std::endl;
			try {
				assert(_game);
				/// enable physics if not already
				/// Create a kinematic rigid body and add it physics world
				/// use same MotionStates for both kinematic and rigid body
				_game->enablePhysics(true);
				// We assume that the mesh name is the same as the SceneNode
				vl::MeshRefPtr mesh = _game->getMeshManager()->loadMesh(sn->getName());
				physics::ConvexHullShapeRefPtr shape = physics::ConvexHullShape::create(mesh);
				physics::RigidBody::ConstructionInfo info(sn->getName(), 0, ms, shape, Ogre::Vector3(0, 0, 0), true);
				physics::RigidBodyRefPtr physics_body = _game->getPhysicsWorld()->createRigidBodyEx(info);
				// necessary to add callback so the kinematic object updates 
				// the the collision model.
				body->addListener(boost::bind(&physics::RigidBody::setWorldTransform, physics_body, _1));
				// Used by the collision detection to pop last transformation.
				physics_body->setUserData(body.get());
			}
			catch(vl::exception const &e)
			{
				std::clog << "Exception thrown when creating collision model for " << sn->getName() << std::endl;
			}
		}
	}

	return body;
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

	std::string type_name(type);
	vl::to_lower(type_name);

	vl::ConstraintRefPtr c;
	if(type_name == "slider")
	{
		c = SliderConstraint::create(body0, body1, trans);
	}
	else if(type_name == "hinge")
	{
		c = HingeConstraint::create(body0, body1, trans);
	}
	else if(type_name == "fixed")
	{
		c = FixedConstraint::create(body0, body1, trans);
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
			break;
		}
	}
}

bool
vl::KinematicWorld::hasConstraint(vl::ConstraintRefPtr constraint) const
{
	for(ConstraintList::const_iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		if(*iter == constraint)
		{
			return true;
		}
	}

	return false;
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
	if(iter != getBodies().end())
	{
		// Does this work or should we use Kinematic Body here?
		parent = (*iter)->getAnimationNode();
	}
	else
	{
		std::stringstream err_msg;
		err_msg << "No Kinematic node \"" << constraint->getBodyA()->getName()
			<< "\" exists : should never happen.";
		std::cout << vl::CRITICAL << err_msg.str() << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg.str()));
	}

	iter = std::find(_bodies.begin(), _bodies.end(), constraint->getBodyB());
	if(iter != getBodies().end())
	{ child = (*iter)->getAnimationNode(); }
	else
	{
		std::stringstream err_msg;
		err_msg << "No Kinematic node \"" << constraint->getBodyB()->getName()
			<< "\" exists : should never happen.";
		std::cout << vl::CRITICAL << err_msg.str() << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg.str()));
	}

	animation::LinkRefPtr link;
	if(child)
	{
		// If bodyB has already a parent it can't be added
		if(child->getParent())
		{

			if(child->getParent()->getParent() != _graph->getRoot())
			{
				std::cout << vl::CRITICAL << constraint->getBodyB()->getName() 
					<< " already has a parent and it's not root so the constraint to "
					<< constraint->getBodyA()->getName() << " can't be added."
					<< std::endl;

				return;
			}
			else
			{
				link = child->getParent();
			}
		}
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
