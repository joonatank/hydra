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

	void setPosition( Ogre::Vector3 const &v )
	{
		setDirty( DIRTY_POSITION );
		_position = v;
	}

	Ogre::Quaternion const &getOrientation( void ) const
	{ return _orientation; }

	void setOrientation( Ogre::Quaternion const &q )
	{
		setDirty( DIRTY_ORIENTATION );
		_orientation = q;
	}

	bool getVisibility( void ) const
	{ return _visible; }

	void setVisibility( bool visible );

	void showBoundingBox( bool show );

	void addEntity(vl::EntityPtr ent);

	void removeEntity(vl::EntityPtr ent);

	bool hasEntity(vl::EntityPtr ent) const;

	void addChild(vl::SceneNodePtr child);

	void removeChild(vl::SceneNodePtr child);

	bool hasChild(vl::SceneNodePtr child) const;

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_POSITION = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_ORIENTATION = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_VISIBILITY = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_BOUNDING_BOX = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_CHILDS = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_ENTITIES = vl::Distributed::DIRTY_CUSTOM << 6,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 7,
	};

	Ogre::SceneNode *getNative(void) const
	{ return _ogre_node; }

	friend std::ostream &operator<<(  std::ostream &os, SceneNode const &a );

protected :

	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	bool _findNode( void );

	std::string _name;

	Ogre::Vector3 _position;
	Ogre::Quaternion _orientation;

	bool _visible;

	bool _show_boundingbox;

	std::vector<vl::SceneNodePtr> _childs;
	std::vector<vl::EntityPtr> _entities;

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
