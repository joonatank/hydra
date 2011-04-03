/**	Joonatan Kuosa
 *	2010-12 initial implementation
 *
 *	2011-01 Removed equalizer dependencies
 */

#ifndef VL_ACTIONS_TRANSFORM_HPP
#define VL_ACTIONS_TRANSFORM_HPP

#include "action.hpp"
#include "typedefs.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "base/timer.hpp"

namespace vl
{

/// Simple Action class that does transformation based on key events
/// Keeps track of the state of the object (moving, stopped)
/// Transforms a SceneNode provided
/// All parameters that need units are in SI units (e.g. speed is m/s)
class MoveAction : public BasicAction
{
public :
	MoveAction( void );

	// TODO this should be executed with a double parameter (delta time) from
	// FrameTrigger
	/// Purposefully not virtual override the private virtual move
	/// which is called from this
	void execute( void );

	/// Parameters
	void setSpeed( double speed )
	{ _speed = speed; }

	double getSpeed( void ) const
	{ return _speed; }

	void setLocal( bool local )
	{ _local = local; }

	bool getLocal( void ) const
	{ return _local; }

	/// Set the angular speed of the object in radians per second
	void setAngularSpeed( Ogre::Radian const &speed )
	{ _angular_speed = speed; }

	Ogre::Radian const &getAngularSpeed( void ) const
	{ return _angular_speed; }

	void setMoveDir( Ogre::Vector3 const &mov_dir )
	{ _move_dir = mov_dir; }

	void setRotDir( Ogre::Vector3 const &rot_dir )
	{ _rot_dir = rot_dir; }

/// Private virtual methods
private :
	virtual void move(Ogre::Vector3 const &v, bool local) = 0;
	virtual void rotate(Ogre::Quaternion const &v, bool local) = 0;

/// Data
protected :
	Ogre::Vector3 _move_dir;

	Ogre::Vector3 _rot_dir;

	double _speed;
	Ogre::Radian _angular_speed;

	bool _local;

	vl::timer _clock;
};

class MoveNodeAction : public MoveAction
{
public :
	MoveNodeAction( void );

	void setSceneNode( vl::SceneNodePtr node )
	{ _node = node; }

	vl::SceneNodePtr getSceneNode( void )
	{ return _node; }

	static MoveNodeAction *create( void )
	{ return new MoveNodeAction; }

	std::string getTypeName( void ) const
	{ return "MoveNodeAction"; }

/// Private virtual methods
private :
	virtual void move(Ogre::Vector3 const &v, bool local);
	virtual void rotate(Ogre::Quaternion const &q, bool local);

/// Data
private :
	vl::SceneNodePtr _node;

};

class MoveSelectionAction : public MoveAction
{
public :
	MoveSelectionAction( void );

	void setSceneManager( vl::SceneManagerPtr man )
	{ _scene = man; }

	vl::SceneManagerPtr getSceneManager( void )
	{ return _scene; }

	static MoveSelectionAction *create( void )
	{ return new MoveSelectionAction; }

	std::string getTypeName( void ) const
	{ return "MoveSelectionAction"; }

/// Private virtual methods
private :
	virtual void move(Ogre::Vector3 const &v, bool local);
	virtual void rotate(Ogre::Quaternion const &q, bool local);

/// Data
private :
	vl::SceneManagerPtr _scene;

};	// class MoveSelectionAction

class MoveActionProxy : public VectorAction
{
public :
	MoveActionProxy( void )
		: _action(0), _rotation(false), _translation( false ),
		  _value( Ogre::Vector3::ZERO )
	{}

	virtual void execute( Ogre::Vector3 const &data )
	{
		_value = _value+data;

		if( _action && _translation )
		{_action->setMoveDir(_value); }

		if( _action && _rotation )
		{_action->setRotDir(_value); }
	}

	void enableRotation( void )
	{ _rotation = true; }

	void disableRotation( void )
	{ _rotation = false; }

	void enableTranslation( void )
	{ _translation = true; }

	void disableTranslation( void )
	{ _translation = false; }

	void setAction( MoveAction *action )
	{ _action = action; }

	MoveAction *getAction( void )
	{ return _action; }

	static MoveActionProxy *create( void )
	{ return new MoveActionProxy; }

	std::string getTypeName( void ) const
	{ return "MoveActionProxy"; }

private :
	MoveAction *_action;

	bool _rotation;
	bool _translation;

	Ogre::Vector3 _value;

};	// class MoveActionProxy

};	// namespace vl

#endif // VL_ACTIONS_TRANSFORM_HPP
