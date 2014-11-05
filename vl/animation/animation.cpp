/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/animation.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/*
 *	Internal Kinematic animation implementation. Defined in a separate namespace.
 */
#include "animation.hpp"

#include "base/exceptions.hpp"

/// ---------------------------------- Global --------------------------------
std::ostream &
vl::animation::operator<<(std::ostream &os, vl::animation::Node const &n)
{
	os << "Animation Node : length to root = " << n.length_to_root() << "\n"
		<< " transformation = " << n.getTransform() << "\n"
		<< " world transformation = " << n.getWorldTransform() << "\n"
		<< " " << n.getNChildren() << " children" << std::endl;

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
	os << "Animation graph : size " << g.size() << "\n"
		<< " root : \n" << *g.getRoot();

	return os;
}


/// ---------------------------------- Node ----------------------------------

vl::animation::Node::Node(vl::Transform const &initial_transform)
	: _parent()
	, _next_child(0)
	, _transform(initial_transform)
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

vl::animation::LinkRefPtr
vl::animation::Node::getChild(size_t i) const
{
	if(i < _childs.size())
	{
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
	// @todo should we check if we need to update transform?
	_transformation_dirty = true;
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
	// Update parent
	if(_parent.lock())
	{ _parent.lock()->popLastTransform(); }
}

size_t
vl::animation::Node::length_to_root(void) const
{
	// Count the number of links from this node upwards
	// the first node not having a link is the root node.

	// @todo this function should be implemented as recursive
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
vl::animation::Node::setParent(LinkRefPtr link)
{
	// Keep the transformation
	Transform wt(getWorldTransform());

	if(!_parent.expired())
	{
		_parent.lock()->_setChild(NodeRefPtr());
	}

	if(link)
	{
		link->_setChild(shared_from_this());
	}

	_parent = link;
	setWorldTransform(wt);
}

bool
vl::animation::Node::hasChild(LinkRefPtr link) const
{
	return( std::find(_childs.begin(), _childs.end(), link) != _childs.end() );
}

void
vl::animation::Node::removeChildren(void)
{
	// We need to use a copy because the removal will modify the original
	LinkList childs(_childs);
	for(LinkList::iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		removeChild(*iter);
	}
}

void
vl::animation::Node::removeChild(vl::animation::LinkRefPtr link)
{
	if(link && hasChild(link))
	{
		link->setParent(NodeRefPtr());
	}
}

size_t
vl::animation::Node::size(void) const
{
	size_t n = 1;
	for(LinkList::const_iterator iter = _childs.begin(); iter != _childs.end(); ++iter)
	{
		n += (*iter)->size();
	}
	
	return n;
}

void
vl::animation::Node::_addChild(LinkRefPtr link)
{
	if(!hasChild(link))
	{ _childs.push_back(link); }
}

void
vl::animation::Node::_removeChild(LinkRefPtr link)
{
	std::vector<LinkRefPtr>::iterator iter = std::find(_childs.begin(), _childs.end(), link);
	
	if(iter != _childs.end())
	{ _childs.erase(iter); }
	else
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No such child.")); }
}

void
vl::animation::Node::_updateCachedTransform(vl::Transform const &parent_t, bool parent_dirty)
{
	bool dirty = (parent_dirty || _transformation_dirty);
	if(dirty)
	{
		_wt_cached = parent_t * _transform;

		_transformation_dirty = false;
	}

	for(LinkList::iterator iter = _childs.begin(); iter != _childs.end(); ++iter)
	{
		(*iter)->_updateCachedTransform(_wt_cached, dirty);
	}
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
	// Keep the transformation
	Transform wt(getWorldTransform());

	if(!_parent.expired())
	{
		_parent.lock()->_removeChild(shared_from_this());
	}

	if(parent)
	{
		parent->_addChild(shared_from_this());
	}

	_parent = parent;

	setWorldTransform(wt);
}

vl::animation::NodeRefPtr
vl::animation::Link::getChild(void) const
{ return _child; }

void
vl::animation::Link::setChild(NodeRefPtr child, bool primary_parent)
{
	if(_child)
	{
		_child->setParent(LinkRefPtr());
	}

	if(child)
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
	// @todo should we check if we need to update Transform or not?
	_transformation_dirty = true;

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

void
vl::animation::Link::setPosition(Ogre::Vector3 const &v, bool preserve_child_transforms)
{
	vl::Transform t(getTransform());
	t.position = v;
	setTransform(t, preserve_child_transforms);
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
	setTransform(Transform(q, _initial_transform.position));
}

void
vl::animation::Link::rotate(Quaternion const &q)
{
	setOrientation(getTransform().quaternion*q);
}

void
vl::animation::Link::popLastTransform(void)
{
	// @todo should we check if we need to update transform?
	_transformation_dirty = true;
	_transform = _prev_transform;

	// Update parents
	if(_parent.lock())
	{ _parent.lock()->popLastTransform(); }
}

void 
vl::animation::Link::setInitialState(void)
{
	_transformation_dirty = true;
	_initial_transform = _transform;
	_prev_transform = _transform;
}

void
vl::animation::Link::reset(void)
{
	setTransform(_initial_transform);
}

size_t
vl::animation::Link::size(void) const
{
	return _child ? _child->size() : 0;
}

void
vl::animation::Link::_updateCachedTransform(vl::Transform const &parent_t, bool parent_dirty)
{
	bool dirty = (parent_dirty || _transformation_dirty);
	if(dirty)
	{
		// @todo add collision flag based selection of either
		// _transform or _prev_transform
		_wt_cached = parent_t * _transform;

		_transformation_dirty = false;
	}

	if(_child)
	{ _child->_updateCachedTransform(_wt_cached, dirty); }
}


/// ---------------------------------- Graph ---------------------------------
vl::animation::Graph::Graph(void)
	: _root(new Node(Transform()))
{}

vl::animation::Graph::~Graph(void)
{}

size_t
vl::animation::Graph::size(void) const
{
	assert(_root);

	// traverse the graph calculating the size
	return _root->size();
}

void
vl::animation::Graph::_update(void)
{
	assert(_root);
	/// Update the cached transformations of all children
	_root->_updateCachedTransform(vl::Transform(), false);
}

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
