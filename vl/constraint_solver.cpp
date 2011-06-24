/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file constraint_solver.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Constraint solver.
 */

#include "constraint_solver.hpp"

#include "scene_node.hpp"

#include "logger.hpp"

/// ------------------------------ Global ------------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::ConstraintSolver const &solver)
{
	os << "Solver has " << solver._constraints.size() << " constraints."
		<< " Node map has " << solver._node_map.size() << " nodes." 
		<< std::endl;

	return os;
}

/// ------------------------------ ConstraintSolver --------------------------
vl::ConstraintSolver::ConstraintSolver(void)
	: _graph(new animation::Graph)
{}

vl::ConstraintSolver::~ConstraintSolver(void)
{}

void
vl::ConstraintSolver::addConstraint(vl::ConstraintRefPtr constraint)
{
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
	AnimationNodeMap::iterator iter = _node_map.find(constraint->getBodyA());
	if(iter != _node_map.end())
	{
		parent = iter->second;
	}
		
	iter = _node_map.find(constraint->getBodyB());
	if(iter != _node_map.end())
	{ child = iter->second; }

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
		}
	}

	if(!parent)
	{
		// Reusing the root would cause incorrect transformations 
		// because the every ScenNode has different transformation 
		// but the root can't have more than one.
		parent.reset(new animation::Node);
		animation::LinkRefPtr link(new animation::Link);
		link->setParent(_graph->getRoot());
		link->setChild(parent);
		_node_map[constraint->getBodyA()] = parent;
	}

	if(!child)
	{
		child.reset(new animation::Node);
		_node_map[constraint->getBodyB()] = child;
	}

	// The correct transformation is set by the constraint
	animation::LinkRefPtr link(new animation::Link);
	link->setParent(parent);
	link->setChild(child);

	constraint->_setLink(link);

	_constraints.push_back(constraint);
}

void
vl::ConstraintSolver::removeConstraint(vl::ConstraintRefPtr constraint)
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
vl::ConstraintSolver::hasConstraint(vl::ConstraintRefPtr constraint) const
{
	for(std::vector<vl::ConstraintRefPtr>::const_iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{
		if(*iter == constraint)
		{
			return true;
		}
	}

	return false;
}

void
vl::ConstraintSolver::step(vl::time const &t)
{
	/// Progress the constraints
	for(std::vector<vl::ConstraintRefPtr>::iterator iter = _constraints.begin();
		iter != _constraints.end(); ++iter)
	{ 
		(*iter)->_proggress(t);
	}

	for(AnimationNodeMap::iterator iter = _node_map.begin();
		iter != _node_map.end(); ++iter )
	{
		iter->first->setWorldTransform(iter->second->getWorldTransform());
	}
}
