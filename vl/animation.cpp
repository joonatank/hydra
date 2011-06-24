/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file animation.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Internal Kinematic animation implementation. Defined in a separate namespace.
 */

#include "animation.hpp"

/// ---------------------------------- Node ----------------------------------

vl::animation::Node::Node(void)
	: _parent()
	, _next_child(0)
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
vl::animation::Node::getParent(void)
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

vl::Transform &
vl::animation::Node::getTransform(void)
{ return _transform; }

vl::Transform const &
vl::animation::Node::getTransform(void) const
{ return _transform; }

void
vl::animation::Node::setTransform(Transform const &t)
{ _transform = t; }

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

	_transform = wt*t;
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
			_childs.erase(iter);
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
	// @todo should preserve the world transformation
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
vl::animation::Link::setChild(NodeRefPtr child)
{
	if(_child)
	{
		_child->_setParent(LinkRefPtr());
	}

	if(child)
	{
		child->_setParent(shared_from_this());
	}

	_child = child;
}

vl::Transform &
vl::animation::Link::getTransform(void)
{ return _transform; }
vl::Transform const &
vl::animation::Link::getTransform(void) const
{ return _transform; }

void
vl::animation::Link::setTransform(Transform const &t)
{ _transform = t; }

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

	_transform = wt*t;
}

void 
vl::animation::Link::setInitialState(void)
{
	_initial_transform = _transform;
}

void
vl::animation::Link::reset(void)
{
	_transform = _initial_transform;
}

/// ---------------------------------- Graph ---------------------------------
vl::animation::Graph::Graph(void)
	: _root(new Node)
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
