#ifndef EQ_OGRE_SCENE_NODE_HPP
#define EQ_OGRE_SCENE_NODE_HPP

#include <eq/fabric/serializable.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include "serialize_helpers.hpp"
#include "action.hpp"
#include "base/exceptions.hpp"

namespace eqOgre
{
class SceneNode;
typedef SceneNode * SceneNodePtr;

class SceneNode : public eq::fabric::Serializable
{
public :
	virtual ~SceneNode( void ) {}

	// TODO this should really be ref counted
	/// SceneNode has a protected Constructor so this method is the only way
	/// to create one.
	static SceneNodePtr create( std::string const &name = std::string() );

	std::string const &getName( void ) const
	{ return _name; }

	void setName( std::string const &name )
	{
		setDirty( DIRTY_NAME );
		_name = name;
	}

	/// Find the SceneNode with the same name from the Scene graph provided
	/// Returns true if found, false otherwise
	/// After this deserialization will transform the Ogre SceneNode
	bool findNode( Ogre::SceneManager *man );

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

	void setVisibility( bool visible )
	{
		setDirty( DIRTY_VISIBILITY );
		_visible = visible;
	}

	enum DirtyBits
	{
		DIRTY_NAME = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
		DIRTY_POSITION = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_ORIENTATION = eq::fabric::Serializable::DIRTY_CUSTOM << 2,
		DIRTY_VISIBILITY = eq::fabric::Serializable::DIRTY_CUSTOM << 3
	};

protected :
	SceneNode( std::string const &name = std::string() );

	virtual void serialize( co::DataOStream &os, const uint64_t dirtyBits );
	virtual void deserialize( co::DataIStream &is, const uint64_t dirtyBits );

private :
	std::string _name;

	Ogre::Vector3 _position;
	Ogre::Quaternion _orientation;

	// These are here for the time beign as the SceneGraph is only read on the
	// slave instances. The master does not have correct initial positions.
	// Can be removed when the master reads the dotscene file also.
	Ogre::Vector3 _initial_position;
	Ogre::Quaternion _initial_orientation;

	bool _visible;

	Ogre::SceneNode *_ogre_node;

};	// class SceneNode

inline std::ostream & operator<<(  std::ostream &os, SceneNode const &a )
{
	os << "SceneNode = " << a.getName() << " with ID = " << a.getID()
		<< " with position " << a.getPosition()
		<< " and orientation " << a.getOrientation();

	return os;
}



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


}	// namespace eqOgre

#endif // EQ_OGRE_SCENE_NODE_HPP