/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_node.hpp
 */

#ifndef VL_SCENE_NODE_HPP
#define VL_SCENE_NODE_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include "action.hpp"
#include "base/exceptions.hpp"
#include "typedefs.hpp"

// Base class
#include "distributed.hpp"

namespace vl
{

class SceneNode : public vl::Distributed
{
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

	Ogre::Vector3 const &getPosition( void ) const
	{ return _position; }

	void setPosition( Ogre::Vector3 const &v );

	Ogre::Quaternion const &getOrientation( void ) const
	{ return _orientation; }

	void setOrientation( Ogre::Quaternion const &q );

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

	bool getVisibility( void ) const
	{ return _visible; }

	void setVisibility( bool visible );

	void showBoundingBox( bool show );

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

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_POSITION = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_ORIENTATION = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_SCALE = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_VISIBILITY = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_BOUNDING_BOX = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_CHILDS = vl::Distributed::DIRTY_CUSTOM << 6,
		DIRY_ATTACHED = vl::Distributed::DIRTY_CUSTOM << 7,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 8,
	};

	Ogre::SceneNode *getNative(void) const
	{ return _ogre_node; }

	friend std::ostream &operator<<(  std::ostream &os, SceneNode const &a );

protected :

	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	void _createNative( void );

	std::string _name;

	Ogre::Vector3 _position;
	Ogre::Quaternion _orientation;
	Ogre::Vector3 _scale;

	bool _visible;

	bool _show_boundingbox;

	/// Keep track of the parent, so we can inform it when hierarchy is changed
	vl::SceneNodePtr _parent;

	std::vector<vl::SceneNodePtr> _childs;
	std::vector<vl::MovableObjectPtr> _objects;

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

#endif // VL_SCENE_NODE_HPP
