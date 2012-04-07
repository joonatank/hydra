/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/animation.hpp
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

#ifndef HYDRA_ANIMATION_HPP
#define HYDRA_ANIMATION_HPP

#include <vector>

#include "math/transform.hpp"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace vl
{

namespace animation
{

// Forward declartions
class Link;
class Node;
class Graph;

// Using ref counted pointers for easy memory handling
// also these will scale to implementation where object can have multiple parents
typedef boost::shared_ptr<Link> LinkRefPtr;
typedef boost::shared_ptr<Node> NodeRefPtr;
typedef boost::shared_ptr<Graph> GraphRefPtr;

typedef boost::weak_ptr<Link> LinkWeakPtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::weak_ptr<Graph> GraphWeakPtr;

/// @todo most of the code is duplicated in Node and Link
/// with just few independent methods
/// really should use a common master class for both
/// that implements common transformation methods etc.

/// @class Node
/// @brief A node in the animation graph
class Node : public boost::enable_shared_from_this<Node>
{
public :
	typedef std::vector<LinkRefPtr> LinkList;
	typedef std::vector<LinkWeakPtr> LinkWeakList;

	/// @brief Constructor
	Node(vl::Transform const &initial_transform);

	/// @brief Destructor
	~Node(void);

	/// @brief collision detection helper
	/// when the graph is modified (parents changed) the node
	/// previous transformation reflects the one used when it was
	/// created or the one from last parent.
	/// How ever this is not what we want, we want the last transformation
	/// with the current parent.
	void setInitialState(void)
	{ _prev_transform = _transform; }

	/// @brief is this node a leaf in the graph i.e. contains no child nodes
	bool isLeaf(void) const;

	/// @brief is this the root of the graph i.e. contains no parent node
	bool isRoot(void) const;

	/// @brief get the link to a parent node
	/// @return valid link if this object is not the root otherwise NULL
	LinkRefPtr getParent(void) const;

	/// @brief get the link to a next child, increase the internal child counter
	/// @return valid link if there is a next child otherwise NULL
	/// @throw nothing
	/// Uses an internal counter for the current child that is increased by this function
	LinkRefPtr getNextChild(void);

	/// @brief get the child with index, never fails
	/// @return valid child link if such exists otherwise NULL
	/// @param i the index for the child
	/// Modifies the current child index used by getNextChild.
	LinkRefPtr getChild(size_t i = 0);

	/// @brief get the local transformation
	/// @return Transformation of the Node in local coordinates
	/// Only const version is provided because we need to manage the old
	/// transformations for collision detection.
	Transform const &getTransform(void) const;

	/// @brief set the local transformation
	/// @param t a new Transformation for the Node in local coordinates
	void setTransform(Transform const &t);

	/// @brief get the current world transformation
	/// @return Transformation of the Node in world coordinates
	Transform getWorldTransform(void) const;
	
	/// @brief set the current world transformation
	/// @param t a new Transformation for the Node in world coordinates
	void setWorldTransform(Transform const &t);

	/// @brief collision detection helper
	void popLastTransform(void);

	size_t length_to_root(void) const;

	void addAuxilaryParent(LinkRefPtr link);

	LinkList getAuxilaryParents(void) const;

	/// @brief set the parent link
	void setParent(LinkRefPtr link);
	
	/// @internal
	void _setParent(LinkRefPtr link);

	/// @internal
	/// @brief add a child link
	void _addChild(LinkRefPtr link);

	/// @internal
	/// @brief remove a child link
	void _removeChild(LinkRefPtr link);

	bool _hasChild(LinkRefPtr link);

private :

	LinkWeakPtr _parent;
	LinkList _childs;
	size_t _next_child;

	// Does not own parents
	LinkWeakList _aux_parents;

	Transform _transform;

	Transform _prev_transform;
	
};	// class Node

/// @class Link
/// @brief A link between two Nodes in the animation graph
/// All links are owned by their parent Nodes so when the parent node
/// is destroyed it destroys all the links owned by it also.
class Link : public boost::enable_shared_from_this<Link>
{
public :
	/// @brief constructor
	/// @param t Transformation where the link starts
	Link(Transform const &t = Transform());

	/// @brief destructor
	~Link(void);

	NodeRefPtr getParent(void) const;

	void setParent(NodeRefPtr parent);

	NodeRefPtr getChild(void) const;

	/// @param child the child to set child of this Link
	/// @param primary_parent if this link is the primary (or only) parent
	void setChild(NodeRefPtr child, bool primary_parent = true);

	/// @brief get the local transformation
	/// @return Transformation of the Link in local coordinates
	/// Only const version is provided because we need to manage the old
	/// transformations for collision detection.
	Transform const &getTransform(void) const;

	/// @brief set the local transformation
	/// @param t a new Transformation for the Link in local coordinates
	/// @param preserve_child_transforms modify the link only preserving the
	/// childs world coordinate frame.
	void setTransform(Transform const &t, bool preserve_child_transforms = false);

	void setPosition(Ogre::Vector3 const &v, bool preserve_child_transforms = false);

	/// @brief get the current world transformation
	/// @return Transformation of the Link in world coordinates
	Transform getWorldTransform(void) const;

	/// @brief set the current world transformation
	/// @param t a new Transformation for the Link in world coordinates
	void setWorldTransform(Transform const &t);

	/// @brief set the current orientation relative to the initial transformation
	/// preseres childs translation and only affects it's orientation
	void setOrientation(Quaternion const &);

	void rotate(Quaternion const &);

	/// @brief collision detection helper
	void popLastTransform(void);

	/// @brief get the initial state transformation
	/// Purposefully non-const method is not provided as the initial state 
	/// should be set and not modified independently.
	Transform const &getInitialTransform(void) const
	{ return _initial_transform; }

	/// @brief set the initial state of the link to current state
	void setInitialState(void);

	/// @brief resets the link to initial state
	void reset(void);

	/// @internal
	void _setParent(NodeRefPtr parent);

	/// @internal
	void _setChild(NodeRefPtr child);

private :
	NodeWeakPtr _parent;
	NodeRefPtr _child;

	Transform _transform;
	Transform _initial_transform;

	Transform _prev_transform;

};	// class Link

/// @class Graph
/// @brief A graph containing all the animation Nodes and Links
class Graph
{
public :
	/// @brief constructor
	Graph(void);

	/// @brief destructor
	~Graph(void);

	/// @brief get the start node i.e. root for the whole graph
	NodeRefPtr getRoot(void);

	/// @todo Transformation Stack is not in use for the moment

	/// @brief push a transform to the transformation stack
	void _pushTransform(Transform const &);

	/// @brief pop a transform from the transformation stack
	void _popTransform(void);

private :
	NodeRefPtr _root;

};	// class Graph


std::ostream &
operator<<(std::ostream &os, Node const &n);

std::ostream &
operator<<(std::ostream &os, Link const &l);

std::ostream &
operator<<(std::ostream &os, Graph const &g);

}	// namespace animation

}	// namespace vl

#endif	// HYDRA_ANIMATION_HPP
