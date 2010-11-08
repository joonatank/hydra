#ifndef EQ_OGRE_SCENE_NODE_HPP
#define EQ_OGRE_SCENE_NODE_HPP

#include <eq/fabric/serializable.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include "serialize_helpers.hpp"
#include "event.hpp"
#include "base/exceptions.hpp"

namespace eqOgre
{
class SceneNode;
typedef SceneNode * SceneNodePtr;

class SceneNode : public eq::fabric::Serializable
{
public :
	~SceneNode( void )
	{}

	// TODO this should really be ref counted
	static SceneNodePtr create( std::string const &name = std::string() );

	std::string const &getName( void ) const
	{
		return _name;
	}
	
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
		
	virtual void serialize( eq::net::DataOStream &os, const uint64_t dirtyBits );
    virtual void deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits );

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

class HideOperation : public Operation
{
public :
	HideOperation( void )
		: _node(0)
	{}

	void setSceneNode( SceneNodePtr node )
	{ _node = node; }

	SceneNodePtr getSceneNode( void )
	{ return _node; }

	virtual void execute( void )
	{
		if( !_node )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_node->setVisibility( false );
	}

	virtual std::string const &getTypeName( void ) const;

protected :
	SceneNodePtr _node;
};

class HideOperationFactory : public OperationFactory
{
public :
	virtual Operation *create( void )
	{ return new HideOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

inline std::string const &
HideOperation::getTypeName( void ) const
{ return HideOperationFactory::TYPENAME; }


class ShowOperation : public Operation
{
public :
	ShowOperation( void )
		: _node(0)
	{}

	void setSceneNode( SceneNodePtr node )
	{ _node = node; }

	SceneNodePtr getSceneNode( void )
	{ return _node; }

	virtual void execute( void )
	{
		if( !_node )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_node->setVisibility( true );
	}

	virtual std::string const &getTypeName( void ) const;

protected :
	SceneNodePtr _node;
};

class ShowOperationFactory : public OperationFactory
{
public :
	virtual Operation *create( void )
	{ return new ShowOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

inline std::string const &
ShowOperation::getTypeName( void ) const
{ return ShowOperationFactory::TYPENAME; }


}	// namespace eqOgre

#endif // EQ_OGRE_SCENE_NODE_HPP