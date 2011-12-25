/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/animation.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*
 *	Internal Kinematic animation implementation. Defined in a separate namespace.
 */
#include "animation.hpp"

/// ---------------------------------- Global --------------------------------
std::ostream &
vl::animation::operator<<(std::ostream &os, vl::animation::Node const &n)
{
	os << "Animation Node : length to root = " << n.length_to_root() << "\n"
		<< " transformation = " << n.getTransform() << "\n"
		<< " world transformation = " << n.getWorldTransform() << std::endl;

	return os;
}

std::ostream &
vl::animation::operator<<(std::ostream &os, vl::animation::Link const &l)
{
	os << "Animation Link : "
		<< " transformation = " << l.getTransform() << "\n"
		<< " world transformation = " << l.getWorldTransform() << std::endl;

	return os;
}

std::ostream &
vl::animation::operator<<(std::ostream &os, vl::animation::Graph const &g)
{
	// Not that many attributes we can print. Might add size of the graph
	// the longest path and so on.
	os << "Animation graph : ";

	return os;
}


/// ---------------------------------- Node ----------------------------------

vl::animation::Node::Node(vl::Transform const &initial_transform)
	: _parent()
	, _next_child(0)
	, _transform(initial_transform)
	, _prev_transform(initial_transform)
{}

vl::animation::Node::~Node(void)
{}

bool
vl::animation::Node::isLeaf(void) const
{ return _childs.empty(); }

bool
vl::animation::Node::isRoot(void) const
{ return _parent.expired(); }

vl::animation::LinkRefPtr
vl::animation::Node::getParent(void) const
{ return _parent.lock(); }

vl::animation::LinkRefPtr
vl::animation::Node::getNextChild(void)
{
	if(_next_child < _childs.size())
	{
		++_next_child;
		return _childs.at(_next_child-1);
	}

	return LinkRefPtr();
}

vl::animation::LinkRefPtr
vl::animation::Node::getChild(size_t i)
{
	if(i < _childs.size())
	{
		_next_child = i+1;
		return _childs.at(i);
	}
	
	return LinkRefPtr();
}

vl::Transform const &
vl::animation::Node::getTransform(void) const
{ return _transform; }

void
vl::animation::Node::setTransform(Transform const &t)
{
	_prev_transform = _transform;
	_transform = t;
}

vl::Transform
vl::animation::Node::getWorldTransform(void) const
{
	// Nodes don't need to have parents
	if(_parent.lock())
	{ return _parent.lock()->getWorldTransform()*_transform; }
	
	return _transform;
}

void
vl::animation::Node::setWorldTransform(Transform const &t)
{
	Transform wt;
	// Nodes don't need to have parents
	if(!_parent.expired())
	{
		wt = _parent.lock()->getWorldTransform();
		wt.invert();
	}

	setTransform(wt*t);
}

void
vl::animation::Node::popLastTransform(void)
{
	_transform = _prev_transform;
	// Update parents
	if(_parent.lock())
	{ _parent.lock()->popLastTransform(); }
}

size_t
vl::animation::Node::length_to_root(void) const
{
	// Count the number of links from this node upwards
	// the first node not having a link is the root node.

	size_t count = 0;
	animation::LinkRefPtr l(getParent());
	while(l)
	{
		++count;
		// shouldn't be necessary as all links have parents
		// nodes don't necessarily
		if(!l->getParent())
		{ break; }

		l = l->getParent()->getParent();
	}
	
	return count;
}

void
vl::animation::Node::addAuxilaryParent(vl::animation::LinkRefPtr link)
{
	// Never add same link twice
	for(std::vector<LinkWeakPtr>::iterator iter =_aux_parents.begin();
		iter != _aux_parents.end(); ++iter)
	{
		// @todo remove expired
		if(!iter->expired())
		{
			if(iter->lock() == link)
			{ return; }
		}
	}

	_aux_parents.push_back(link);
}

vl::animation::Node::LinkList
vl::animation::Node::getAuxilaryParents(void) const
{
	// @todo should we store ref pointers still?
	// this makes for a bit akward code and also probably inefficient.
	vl::animation::Node::LinkList parents;
	for(size_t i = 0; i < _aux_parents.size(); ++i)
	{
		// @todo should remove expired
		if(!_aux_parents.at(i).expired())
		{ parents.push_back(_aux_parents.at(i).lock()); }
	}

	return parents;
}

void
vl::animation::Node::setParent(LinkRefPtr link)
{
	// Check that the link has our already
	if(link)
	{
		link->_setChild(shared_from_this());
	}

	_setParent(link);
}

void
vl::animation::Node::_setParent(LinkRefPtr link)
{
	// Keep the transformation
	Transform wt(getWorldTransform());
	_parent = link;
	setWorldTransform(wt);
}

void
vl::animation::Node::_addChild(LinkRefPtr link)
{
	if(!_hasChild(link))
	{ _childs.push_back(link); }
}

void
vl::animation::Node::_removeChild(LinkRefPtr link)
{
	for(std::vector<LinkRefPtr>::iterator iter = _childs.begin(); 
		iter != _childs.end(); ++iter)
	{
		if(*iter == link)
		{
			// The link can only exist once in the node so we can safely
			// do an erase here and return.
			_childs.erase(iter);
			return;
		}
	}
}

bool
vl::animation::Node::_hasChild(LinkRefPtr link)
{
	if(std::find(_childs.begin(), _childs.end(), link) == _childs.end())
	{ return false; }

	return true;
}

/// ---------------------------------- Link ----------------------------------
vl::animation::Link::Link(Transform const &t)
	: _transform(t)
	, _initial_transform(t)
{}

vl::animation::Link::~Link(void)
{}

vl::animation::NodeRefPtr
vl::animation::Link::getParent(void) const
{ return _parent.lock(); }

void
vl::animation::Link::setParent(NodeRefPtr parent)
{
	if(!_parent.expired())
	{
		_parent.lock()->_removeChild(shared_from_this());
	}

	if(parent)
	{
		parent->_addChild(shared_from_this());
	}

	_setParent(parent);
}

void
vl::animation::Link::_setParent(NodeRefPtr parent)
{
	// Keep the transformation
	Transform wt(getWorldTransform());

	_parent = parent;

	setWorldTransform(wt);
}

vl::animation::NodeRefPtr
vl::animation::Link::getChild(void) const
{ return _child; }

void
vl::animation::Link::setChild(NodeRefPtr child, bool primary_parent)
{
	if(child && primary_parent)
	{
		child->setParent(shared_from_this());
	}
	
	_setChild(child);
}

void
vl::animation::Link::_setChild(NodeRefPtr child)
{
	_child = child;
}

vl::Transform const &
vl::animation::Link::getTransform(void) const
{ return _transform; }

void
vl::animation::Link::setTransform(Transform const &t, bool preserve_child_transforms)
{
	// save the last state
	_prev_transform = _transform;

	if(preserve_child_transforms && _child)
	{
		/// Save the old transformation before resetting link
		/// this is because changing the link will change the world transformation
		/// of the child node.
		Transform wt(_child->getWorldTransform());
		_transform = t;
		_child->setWorldTransform(wt);
	}
	else
	{ _transform = t; }
}

vl::Transform
vl::animation::Link::getWorldTransform(void) const
{
	if(!_parent.expired())
	{ return _parent.lock()->getWorldTransform()*_transform; }

	return _transform;
}

void
vl::animation::Link::setWorldTransform(Transform const &t)
{
	// Links can be without parent when they are created
	Transform wt;
	if(!_parent.expired())
	{
		wt = _parent.lock()->getWorldTransform();
		wt.invert();
	}

	setTransform(wt*t);
}

void
vl::animation::Link::setOrientation(Quaternion const &q)
{
	/// this messes up the axis of rotation, but that's easy to fix
	/// per constraint using python.
	/// preserves the translation correctly and center of orientation.
	setTransform(Transform(q*_initial_transform.quaternion, _initial_transform.position));
}

void
vl::animation::Link::popLastTransform(void)
{
	_transform = _prev_transform;
	// Update parents
	if(_parent.lock())
	{ _parent.lock()->popLastTransform(); }
}

void 
vl::animation::Link::setInitialState(void)
{
	_initial_transform = _transform;
	_prev_transform = _transform;
}

void
vl::animation::Link::reset(void)
{
	setTransform(_initial_transform);
}

/// ---------------------------------- Graph ---------------------------------
vl::animation::Graph::Graph(void)
	: _root(new Node(Transform()))
{}

vl::animation::Graph::~Graph(void)
{}

vl::animation::NodeRefPtr
vl::animation::Graph::getRoot(void)
{ return _root; }

/// @brief push a transform to the transformation stack
void
vl::animation::Graph::_pushTransform(vl::Transform const &t)
{
}

/// @brief pop a transform from the transformation stack
void
vl::animation::Graph::_popTransform(void)
{
}
