/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_node.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef HYDRA_SCENE_NODE_HPP
#define HYDRA_SCENE_NODE_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

// Used for user callbacks
#include <boost/signal.hpp>

#include "action.hpp"
#include "base/exceptions.hpp"
#include "typedefs.hpp"

#include "math/transform.hpp"

// Base class
#include "distributed.hpp"

namespace vl
{

enum TransformSpace
{
	TS_LOCAL,
	TS_PARENT,
	TS_WORLD,
};

/// @todo add getTransform and getWorldTransform using vl::Transform
class SceneNode : public vl::Distributed
{
	typedef boost::signal<void (vl::Transform const &)> TransformedCB;

public :
	SceneNode( std::string const &name, vl::SceneManager *creator );

	virtual ~SceneNode( void ) {}

	std::string const &getName( void ) const
	{ return _name; }

	void setName( std::string const &name )
	{
		setDirty( DIRTY_NAME );
		_name = name;
	}

	/// @brief transform the node by a matrix, includes the scale part
	/// @param m matrix to use for transformation
	void transform(Ogre::Matrix4 const &m);

	/// @brief set the transformation of the node by a matrix, includes the scale part
	/// @param m matrix to replace the transformation
	void setTransform(Ogre::Matrix4 const &m);

	/// @brief transform the node, does not include the scale part
	void transform(vl::Transform const &trans);

	/// @brief set the transformation, does not include the scale part
	void setTransform(vl::Transform const &trans);

	/// @brief set the transformation in the space of an another object
	/// @param trans Transformation in reference space
	/// @param reference the space to use for Transformation
	/// @todo not testes, and probably does not work correctly
	void setTransform(vl::Transform const &trans, SceneNodePtr reference);

	vl::Transform const &getTransform(void) const
	{ return _transform; }

	/// @brief get transformation in the space of an another object
	/// @param reference the space the Transformation is returned
	/// @return Transformation in reference space
	/// @todo not testes, and probably does not work correctly
	vl::Transform getTransform(SceneNodePtr reference) const;

	/// @brief set the transformation in the world space
	/// @param Transformation in world space
	/// @todo not testes, and probably does not work correctly
	void setWorldTransform(vl::Transform const &trans);

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	vl::Transform getWorldTransform(void) const;
	
	/// @brief get the position in object space
	Ogre::Vector3 const &getPosition(void) const
	{ return _transform.position; }

	/// @brief set the position in object space
	void setPosition( Ogre::Vector3 const &v );

	/// @brief translate the SceneNode
	/// @param v how much to translate
	/// @param reference the coordinate system to use for translation
	void translate(Ogre::Vector3 const &v, vl::SceneNodePtr reference);

	/// @brief translate the SceneNode
	/// @param v how much to translate
	/// @param which coordinate system to use
	void translate(Ogre::Vector3 const &v, vl::TransformSpace space);

	/// @brief translate the SceneNode in local coordinate system
	/// defined separately because easier to expose for python
	void translate(Ogre::Vector3 const &v);
	void translate(Ogre::Real x, Ogre::Real y, Ogre::Real z)
	{ translate(Ogre::Vector3(x, y, z)); }

	/// @brief get the orientation in object space
	Ogre::Quaternion const &getOrientation( void ) const
	{ return _transform.quaternion; }

	/// @brief set the orientation in object space
	void setOrientation( Ogre::Quaternion const &q );

	/// @brief rotates the SceneNode around reference
	/// @param q how much to rotate
	/// @param reference the coordinate system to use for translation
	/// @todo seems like this doesn't not work as expected for some rotations
	/// @todo should this be renamed to rotateAround, because this is only usefull for special cases
	void rotate(Ogre::Quaternion const &q, vl::SceneNodePtr reference);

	/// @brief rotates the SceneNode in transform space
	/// @param q how much to rotate
	/// @param which coordinate system to use
	void rotate(Ogre::Quaternion const &q, vl::TransformSpace space);

	/// @brief rotates the SceneNode in local coordinate system
	/// defined separately because easier to expose for python
	void rotate(Ogre::Quaternion const &q);
	/// @brief helper overloads
	void rotate(Ogre::Vector3 const &axis, Ogre::Radian const &angle)
	{ rotate(Ogre::Quaternion(angle, axis)); }
	void rotate(Ogre::Radian const &angle, Ogre::Vector3 const &axis)
	{ rotate(Ogre::Quaternion(angle, axis)); }
	void rotate(Ogre::Vector3 const &axis, Ogre::Degree const &angle)
	{ rotate(Ogre::Quaternion(Ogre::Radian(angle), axis)); }
	void rotate(Ogre::Degree const &angle, Ogre::Vector3 const &axis)
	{ rotate(Ogre::Quaternion(Ogre::Radian(angle), axis)); }

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

	bool getVisible( void ) const
	{ return _visible; }

	/// @brief set the visibility of the object
	/// @param visible
	void setVisible(bool visible);

	void show(void)
	{ setVisible(true); }
	bool isShown(void) const
	{ return getVisible() == true; }

	void hide(void)
	{ setVisible(false); }
	bool isHidden(void) const
	{ return getVisible() == false; }

	/// @brief show the bounding box of the object
	/// Shows a combined bounding box of all childs of this node
	void setShowBoundingBox(bool show);

	bool getShowBoundingBox(void) const
	{ return _show_boundingbox; }

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

	/// --------------- MovableObjects ----------------------
	void attachObject(vl::MovableObjectPtr obj);

	void detachObject(vl::MovableObjectPtr obj);

	bool hasObject(vl::MovableObjectPtr obj) const;

	/// --------------- Childs ------------------------
	vl::SceneNodePtr createChildSceneNode(std::string const &name);

	void addChild(vl::SceneNodePtr child);

	void removeChild(vl::SceneNodePtr child);

	bool hasChild(vl::SceneNodePtr child) const;

	vl::SceneNodePtr getParent(void) const
	{ return _parent; }

	SceneManagerPtr getCreator(void) const
	{ return _creator; }


	// -------------------- Callbacks -----------------------
	int addListener(TransformedCB::slot_type const &slot)
	{ _transformed_cb.connect(slot); return 1; }

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_TRANSFORM = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_SCALE = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_VISIBILITY = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_BOUNDING_BOX = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_CHILDS = vl::Distributed::DIRTY_CUSTOM << 6,
		DIRY_ATTACHED = vl::Distributed::DIRTY_CUSTOM << 7,
		DIRTY_INHERIT_SCALE = vl::Distributed::DIRTY_CUSTOM << 8,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 9,
	};

	Ogre::SceneNode *getNative(void) const
	{ return _ogre_node; }

	friend std::ostream &operator<<(  std::ostream &os, SceneNode const &a );

protected :

	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	vl::SceneNodePtr doClone(std::string const &append_to_name, vl::SceneNodePtr parent) const;

private :
	// Disallow copying use clone instead
	SceneNode(SceneNode const &);
	SceneNode & operator=(SceneNode const &);

	std::string _name;

	vl::Transform _transform;
	Ogre::Vector3 _scale;

	// @todo combine the boolean attributes
	bool _visible;

	bool _show_boundingbox;

	bool _inherit_scale;

	/// Keep track of the parent, so we can inform it when hierarchy is changed
	vl::SceneNodePtr _parent;

	std::vector<vl::SceneNodePtr> _childs;
	std::vector<vl::MovableObjectPtr> _objects;

	// Callbacks
	TransformedCB _transformed_cb;

	Ogre::SceneNode *_ogre_node;

	vl::SceneManager *_creator;

};	// class SceneNode

std::ostream &
operator<<(std::ostream &os, SceneNode const &a);


/// ---------------------- SceneNode Actions -------------------------

/// Concrete class used to store and act as an interface for Actions needing
/// SceneNode access
/// Is not an action because doesn't do anything.
class SceneNodeActionBase
{
public :
	SceneNodeActionBase( void )
		: _node(0)
	{}

	void setSceneNode( SceneNodePtr node )
	{ _node = node; }

	SceneNodePtr getSceneNode( void )
	{ return _node; }

protected :
	SceneNodePtr _node;
};



class HideAction : public SceneNodeActionBase, public vl::BasicAction
{
public :
	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "HideAction"; }

	static HideAction *create( void )
	{ return new HideAction; }
};


class ShowAction : public SceneNodeActionBase, public  vl::BasicAction
{
public :
	virtual void execute( void );

	virtual std::string getTypeName( void ) const
	{ return "ShowAction"; }

	static ShowAction *create( void )
	{ return new ShowAction; }
};


/// Set a new transformation to a SceneNode
class SetTransformation : public SceneNodeActionBase, public vl::TransformAction
{
public :
	virtual std::string getTypeName( void ) const
	{ return "SetTransformation"; }

	virtual void execute( vl::Transform const &trans );

	static SetTransformation *create( void )
	{ return new SetTransformation; }
};


}	// namespace vl

#endif	// HYDRA_SCENE_NODE_HPP
