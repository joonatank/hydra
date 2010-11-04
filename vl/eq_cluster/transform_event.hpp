#ifndef EQ_OGRE_TRANSFORM_EVENT_HPP
#define EQ_OGRE_TRANSFORM_EVENT_HPP

#include "scene_node.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include <OIS/OISKeyboard.h>

namespace eqOgre
{

/// Simple Event class that does transformation based on key events
/// Keeps track of the state of the object (moving, stopped)
/// Transforms a SceneNode provided
/// All parameters that need units are in SI units (e.g. speed is m/s)
class TransformationEvent
{
public :
	struct KeyPair
	{
		// TODO does not  check if the keys are equal
		KeyPair( OIS::KeyCode npos_key = OIS::KC_UNASSIGNED,
					OIS::KeyCode nneg_key = OIS::KC_UNASSIGNED )
			: pos_key(npos_key), neg_key(nneg_key)
		{}

		int findKey( OIS::KeyCode const key ) const
		{
			if( key == OIS::KC_UNASSIGNED )
			{ return 0; }

			if( key == pos_key )
			{ return 1; }

			if( key == neg_key )
			{ return -1; }

			return 0;
		}

		OIS::KeyCode pos_key;
		OIS::KeyCode neg_key;
	};

	struct KeyPairVec
	{
		KeyPairVec( KeyPair nx = KeyPair(),
					KeyPair ny = KeyPair(),
					KeyPair nz = KeyPair() )
			: x(nx), y(ny), z(nz)
		{}

		KeyPair x;
		KeyPair y;
		KeyPair z;

		/// Find the key event in the structure
		/// returns a Vector of -1, 0, 1 based on wether it was found and if
		/// it was a positive or negative control key.
		/// Return value can be used as movement direction vector,
		/// or a delta to such a vector.
		Ogre::Vector3 findKey( OIS::KeyCode const key )
		{
			Ogre::Vector3 vec = Ogre::Vector3::ZERO;
			if( x.findKey(key) != 0 )
			{ vec.x = x.findKey(key); }
			if( y.findKey(key) != 0 )
			{ vec.y = y.findKey(key); }
			if( z.findKey(key) != 0 )
			{ vec.z = z.findKey(key); }

			return vec;
		}
	};

	/// Constructor
	TransformationEvent( SceneNode *node = 0 );

	/// Copy constructor
	/// Copies the prototype but does not copy any state information
	/// assumption is that the state information is only useful in the context
	/// where it has been setted.
	TransformationEvent( TransformationEvent const &a );

	TransformationEvent &operator=( TransformationEvent const &a );

	/// Destructor
	virtual ~TransformationEvent( void )
	{}

	void setSceneNode( SceneNode *node )
	{ _node = node; }

	SceneNode *getSceneNode( void )
	{ return _node; }

	SceneNode const *getSceneNode( void ) const
	{ return _node; }

	/// Execute operator
	/// Transforms the SceneNode if it's in moving state
	void operator()( void );

	/// Called from event handling
	/// If the key is mapped to this TransformationEvent the state of the
	/// event is changed.
	/// Returns true if processed
	bool keyPressed( OIS::KeyCode key );

	bool keyReleased( OIS::KeyCode key );

	/// Parameters
	void setSpeed( double speed )
	{ _speed = speed; }

	double getSpeed( void ) const
	{ return _speed; }

	/// Set the angular speed of the object in radians per second
	void setAngularSpeed( Ogre::Radian const &speed )
	{ _angular_speed = speed; }

	Ogre::Radian const &getAngularSpeed( void ) const
	{ return _angular_speed; }


	/// Translation keys
	void setTransXKeys( OIS::KeyCode pos_key, OIS::KeyCode neg_key )
	{ setTransXKeys( KeyPair(pos_key, neg_key) ); }

	void setTransXKeys( KeyPair key_pair )
	{ _move_keys.x = key_pair; }

	void setTransYKeys( OIS::KeyCode pos_key, OIS::KeyCode neg_key )
	{ setTransYKeys( KeyPair(pos_key, neg_key) ); }

	void setTransYKeys( KeyPair key_pair )
	{ _move_keys.y = key_pair; }

	void setTransZKeys( OIS::KeyCode pos_key, OIS::KeyCode neg_key )
	{ setTransZKeys( KeyPair(pos_key, neg_key) ); }

	void setTransZKeys( KeyPair key_pair )
	{ _move_keys.z = key_pair; }



	/// Rotation Keys
	void setRotXKeys( OIS::KeyCode pos_key, OIS::KeyCode neg_key )
	{ setRotXKeys( KeyPair(pos_key, neg_key) ); }

	void setRotXKeys( KeyPair key_pair )
	{ _rot_keys.x = key_pair; }

	void setRotYKeys( OIS::KeyCode pos_key, OIS::KeyCode neg_key )
	{ setRotYKeys( KeyPair(pos_key, neg_key) ); }

	void setRotYKeys( KeyPair key_pair )
	{ _rot_keys.y = key_pair; }

	void setRotZKeys( OIS::KeyCode pos_key, OIS::KeyCode neg_key )
	{ setRotZKeys( KeyPair(pos_key, neg_key) ); }

	void setRotZKeys( KeyPair key_pair )
	{ _rot_keys.z = key_pair; }

	friend bool operator==( TransformationEvent const &a, TransformationEvent const &b );
	friend std::ostream & operator<<( std::ostream &os, TransformationEvent const &a );

protected :
	/// Core
	// Node which this Event moves
	SceneNode *_node;

	// When last executed so that the movement has constant speed
	::clock_t _last_time;


	/// Parameters
	// Movement speed
	double _speed;

	// Rotation speed
	Ogre::Radian _angular_speed;

	/// Mappings
	// Mapped keys to movent along axises
	KeyPairVec _move_keys;

	// Mapped keys to rotate around axises
	KeyPairVec _rot_keys;

	// Direction where we are moving, is not unit length
	// For key events there is three possible values for each axis
	// -1, 0, 1
	Ogre::Vector3 _move_dir;

	Ogre::Vector3 _rotation_axises;
};


inline bool operator==( TransformationEvent::KeyPair const &a, TransformationEvent::KeyPair const &b )
{
	return( a.neg_key == b.neg_key && a.pos_key == b.pos_key );
}

inline bool operator==( TransformationEvent::KeyPairVec const &a, TransformationEvent::KeyPairVec const &b )
{
	return( a.x == b.x && a.y == b.y && a.z == b.z );
}

/// Chech that the prototypes are equal but does not test the state information.
/// Assumption is that the state information is only useful in the context
/// where it has been setted.
// FIXME this is not completed
inline bool operator==( TransformationEvent const &a, TransformationEvent const &b )
{
	return( a._node == b._node
		&& a._speed == b._speed
		&& a._angular_speed == b._angular_speed
		&& a._move_keys == b._move_keys
		&& a._rot_keys == b._rot_keys
		);
}

inline std::ostream & operator<<( std::ostream &os , TransformationEvent::KeyPair const &a )
{
	// TODO would be nice if the keys would be converted to human readable form
	os << "KeyPair : neg = " << a.neg_key << " pos = " << a.pos_key;
	return os;
}

inline std::ostream & operator<<(  std::ostream &os, TransformationEvent::KeyPairVec const &a )
{
	os << "x = " << a.x << " y = " << a.y << " z = " << a.z;
	return os;
}

inline std::ostream & operator<<(  std::ostream &os, TransformationEvent const &a )
{
	std::string del("     ");

	if( a.getSceneNode() )
		os << "TransformationEvent for node = " << a.getSceneNode()->getName();
	else
		os << "TransformationEvent without node";
	os << std::endl
		<< del << "speed = " << a.getSpeed()
		<< " m/s : " << " angular speed = " << a.getAngularSpeed() << std::endl
		<< del << "move keys = " << a._move_keys << std::endl
		<< del << "rot keys = " << a._rot_keys;

	return os;
}

}	// namespace eqOgre

#endif // EQ_OGRE_TRANSFORM_EVENT_HPP