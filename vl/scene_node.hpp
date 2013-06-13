/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file scene_node.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */


#ifndef HYDRA_SCENE_NODE_HPP
#define HYDRA_SCENE_NODE_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>


#include "base/exceptions.hpp"
#include "typedefs.hpp"

#include "math/transform.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"
// Base class
#include "cluster/distributed.hpp"
#include "object_interface.hpp"

// Debug helper
#include "ogre_axes.hpp"

namespace vl
{

class HYDRA_API SceneNode : public vl::Distributed, public vl::ObjectInterface
{
public :
	SceneNode(std::string const &name, vl::SceneManager *creator, bool is_dynamic);

	virtual ~SceneNode(void);

	std::string const &getName( void ) const
	{ return _name; }

	void setName( std::string const &name )
	{
		setDirty( DIRTY_NAME );
		_name = name;
	}

	/// Base virtual overrides for transformations
	/// @brief set the transformation, does not include the scale part
	void setTransform(vl::Transform const &trans);
	vl::Transform const &getTransform(void) const
	{ return _transform; }

	/// @brief set the transformation in the world space
	/// @param Transformation in world space
	virtual void setWorldTransform(Transform const &trans);

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	virtual Transform getWorldTransform(void) const;


	void scale(Ogre::Real s);
	void scale(Ogre::Vector3 const &s);

	void setScale(Ogre::Vector3 const &s);

	Ogre::Vector3 const &getScale(void) const
	{ return _scale; }


	/// Set the world size of the object
	/// This is effectively equal to scale(desired_size/getSize)
	/// @todo not implemented
	/// @todo size calculations and modifications need the mesh support
	/// because we can not use Ogre meshes in the master node
	//void setSize(Ogre::Vector3 const &s);

	virtual bool isVisible(void) const
	{ return _visible; }

	/// @brief set the visibility of the object
	/// @param visible
	/// @param cascade wether or not affect childs also
	virtual void setVisibility(bool visible)
	{ setVisibility(visible, true); }
	void setVisibility(bool visible, bool cascade);

	void show(void)
	{ setVisibility(true); }
	bool isShown(void) const
	{ return isVisible(); }

	void hide(void)
	{ setVisibility(false); }
	bool isHidden(void) const
	{ return !isVisible(); }

	/// @brief show the bounding box of the object
	/// Shows a combined bounding box of all childs of this node
	void setShowBoundingBox(bool show);

	bool getShowBoundingBox(void) const
	{ return _show_boundingbox; }

	void setShowDebugDisplay(bool show);

	bool isShowDebugDisplay(void) const
	{ return _show_debug_display; }

	void setShowAxes(bool show);

	bool isShowAxes(void) const
	{ return _show_axes; }

	void setAxesSize(vl::scalar size);

	vl::scalar getAxesSize(void) const
	{ return _axes_size; }

	/// @brief get if the node inherits scale from it's parent
	/// Default value is true, because this will mess up reflections also
	/// as they use negative scale along an axis to do the reflection
	bool getInheritScale(void) const
	{ return _inherit_scale; }

	/// @brief set if the node inherits scale from it's parent
	void setInheritScale(bool b);

	/// @brief make a deep copy of the SceneNode
	/// Shallow copies would not make much sense with SceneGraphs because you
	/// can not have multiple parents unlike DAGs.
	SceneNodePtr clone(void) const;
	SceneNodePtr clone(std::string const &append_to_name) const;

	SceneNodePtr cloneDynamic(void) const;
	SceneNodePtr cloneDynamic(std::string const &append_to_name) const;
	
	/// --------------- MovableObjects ----------------------
	void attachObject(vl::MovableObjectPtr obj);

	void detachObject(vl::MovableObjectPtr obj);

	bool hasObject(vl::MovableObjectPtr obj) const;

	vl::MovableObjectList const &getObjects(void) const
	{ return _objects; }

	/// --------------- Childs ------------------------
	vl::SceneNodePtr createChildSceneNode(std::string const &name);

	void addChild(vl::SceneNodePtr child);

	void removeChild(vl::SceneNodePtr child);

	bool hasChild(vl::SceneNodePtr child) const;

	/// @brief removes all children from this node and puts them under root
	void removeAllChildren(void);

	vl::SceneNodeList const &getChilds(void) const
	{ return _childs; }

	vl::SceneNodePtr getParent(void) const
	{ return _parent; }

	SceneManagerPtr getCreator(void) const
	{ return _creator; }

	bool isDynamic(void) const
	{ return _is_dynamic; }

	// -------------------- Callbacks -----------------------
	virtual int addListener(TransformedCB::slot_type const &slot)
	{ _transformed_cb.connect(slot); return 1; }

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_TRANSFORM = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_SCALE = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_VISIBILITY = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_CHILDS = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRY_ATTACHED = vl::Distributed::DIRTY_CUSTOM << 6,
		DIRTY_PARAMS = vl::Distributed::DIRTY_CUSTOM << 7,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 8,
	};

	Ogre::SceneNode *getNative(void) const
	{ return _ogre_node; }

	friend std::ostream &operator<<(  std::ostream &os, SceneNode const &a );

protected :

	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	vl::SceneNodePtr _do_clone(std::string const &append_to_name, vl::SceneNodePtr parent, bool dynamic) const;

private :
	// Disallow copying use clone instead
	SceneNode(SceneNode const &);
	SceneNode & operator=(SceneNode const &);

	std::string _name;

	vl::Transform _transform;
	Ogre::Vector3 _scale;

	// @todo combine the boolean attributes
	bool _visible;

	// @params
	bool _show_boundingbox;
	bool _inherit_scale;
	bool _show_debug_display;
	bool _show_axes;
	vl::scalar _axes_size;

	/// Keep track of the parent, so we can inform it when hierarchy is changed
	vl::SceneNodePtr _parent;

	std::vector<vl::SceneNodePtr> _childs;
	std::vector<vl::MovableObjectPtr> _objects;

	// Callbacks
	TransformedCB _transformed_cb;

	Ogre::SceneNode *_ogre_node;
	vl::ogre::Axes *_debug_axes;

	vl::SceneManager *_creator;

	bool _is_dynamic;

};	// class SceneNode

std::ostream &
operator<<(std::ostream &os, SceneNode const &a);


}	// namespace vl

#endif	// HYDRA_SCENE_NODE_HPP
