/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/animation.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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
///
/// Nodes don't support collision detection only Links do.
/// Supports dirties in Transformations and caching of the World transformation.
///
/// @todo add cache of Collision detection to the node
/// Best would probably to use a transformation list and indices
/// when a transformation is popped we only increase the index
/// and mark World transformation dirty.
/// More trivial implementation would be to use two variables
/// and a boolean for collision. When transformation is popped
/// the boolean goes true if boolean value was changed mark as dirty.
/// When transformation is set mark the boolean false (and always mark as dirty).
///
/// @todo add methods for determining if the Node is in Graph or not
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
	{ /*_prev_transform = _transform;*/ }

	/// @brief is this node a leaf in the graph i.e. contains no child nodes
	bool isLeaf(void) const;

	/// @brief is this the root of the graph i.e. contains no parent node
	/// @todo this is not necessary true though, in theory we allow free nodes
	/// that are not part of the graph (or are not yet linked) which would
	/// return true here even though they are not root nodes.
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

	LinkRefPtr getChild(size_t i = 0) const;

	size_t getNChildren(void) const
	{ return _childs.size(); }

	/// @brief get the local transformation
	/// @return Transformation of the Node in local coordinates
	/// Only const version is provided because we need to manage the old
	/// transformations for collision detection.
	Transform const &getTransform(void) const;

	/// @brief set the local transformation
	/// @param t a new Transformation for the Node in local coordinates
	/// @todo add a variable to keep child transformations
	void setTransform(Transform const &t);

	/// @brief get the current world transformation
	/// @return Transformation of the Node in world coordinates
	Transform getWorldTransform(void) const;
	
	/// @brief set the current world transformation
	/// @param t a new Transformation for the Node in world coordinates
	/// @todo add a variable to keep child transformations
	void setWorldTransform(Transform const &t);

	/// @brief collision detection helper
	void popLastTransform(void);

	size_t length_to_root(void) const;

	/// @brief Returns how many direct and indirect children this node has.
	/// Calculates only Nodes not Links
	size_t size(void) const;

	/// @brief set the parent link
	void setParent(LinkRefPtr link);

	bool hasChild(LinkRefPtr link) const;

	void removeChildren(void);

	void removeChild(LinkRefPtr link);

	/// @internal
	/// @brief add a child link
	void _addChild(LinkRefPtr link);

	/// @internal
	/// @brief remove a child link
	void _removeChild(LinkRefPtr link);

	/// @internal
	/// @brief Update the cached transformation
	void _updateCachedTransform(vl::Transform const &parent_t, bool parent_dirty);

	/// @internal
	/// Get the last cached transformation
	vl::Transform const &_getCachedWorldTransform(void) const
	{ return _wt_cached; }

private :

	LinkWeakPtr _parent;
	LinkList _childs;
	size_t _next_child;

	Transform _transform;
	
	/// Internal states
	bool _transformation_dirty;

	vl::Transform _wt_cached;

};	// class Node

/// @class Link
/// @brief A link between two Nodes in the animation graph
/// All links are owned by their parent Nodes so when the parent node
/// is destroyed it destroys all the links owned by it also.
///
/// @todo add methods for determining if the Link is in Graph or not
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

	/// @brief Returns how many direct and indirect children this node has.
	/// Calculates only Nodes not Links
	size_t size(void) const;

	/// @internal
	void _setChild(NodeRefPtr child);

	/// @internal
	void _updateCachedTransform(vl::Transform const &parent_t, bool parent_dirty);

	/// @internal
	/// Get the last cached transformation
	vl::Transform const &_getCachedWorldTransform(void) const
	{ return _wt_cached; }

private :
	NodeWeakPtr _parent;
	NodeRefPtr _child;

	Transform _transform;
	Transform _initial_transform;

	Transform _prev_transform;

	/// Internal states
	bool _transformation_dirty;

	vl::Transform _wt_cached;

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
	/// @todo const is wrong here, we should return const Node pointer
	/// it's added for compatibility with const methods, but
	/// can be abused because it returns non-const pointer.
	NodeRefPtr getRoot(void) const
	{ return _root; }

	/// @brief calculate the size of the graph
	/// Number of Nodes connected to the Graph (including root node)
	/// Minimum size is one because root node exists always.
	size_t size(void) const;

	void _update(void);

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
