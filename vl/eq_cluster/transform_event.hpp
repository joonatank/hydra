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

	TransformationEvent( SceneNode *node = 0 )
		: _node(node), _last_time( ::clock() ), _speed(1),
		  _angular_speed( Ogre::Degree(60) ),
		  _move_keys(), _rot_keys(),
		  _move_dir( Ogre::Vector3::ZERO ),
		  _rotation_axises( Ogre::Vector3::ZERO )
	{}

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
	void operator()( void )
	{
		clock_t time = ::clock();
		// Secs since last frame
		double t = ((double)( time - _last_time ))/CLOCKS_PER_SEC;

		// Check that we have an object and we are moving
		// TODO should check the Vector3 with a delta zero
		if( _node && _move_dir != Ogre::Vector3::ZERO )
		{
			Ogre::Vector3 pos = _node->getPosition();
			pos += _speed*t*_move_dir.normalisedCopy();
			_node->setPosition( pos );
		}

		if( _node && _rotation_axises != Ogre::Vector3::ZERO )
		{
			Ogre::Quaternion orient = _node->getOrientation();
			// TODO check if the axis is zero
			Ogre::Quaternion qx( _angular_speed*t, _rotation_axises );
//			Ogre::Quaternion qy( _angular_speed*t, _rotation_axises.y );
//			Ogre::Quaternion qz( _angular_speed*t, _rotation_axises.z );
			_node->setOrientation( qx*orient );
		}

		_last_time = time;
	}

	/// Called from event handling
	/// If the key is mapped to this TransformationEvent the state of the
	/// event is changed.
	/// Returns true if processed
	bool keyPressed( OIS::KeyCode key )
	{
		bool retval = false;
		Ogre::Vector3 vec = _move_keys.findKey(key);
		if( vec != Ogre::Vector3::ZERO )
		{
			_move_dir += vec;
			retval = true;
		}

		vec = _rot_keys.findKey(key);
		if( vec != Ogre::Vector3::ZERO )
		{
			_rotation_axises += vec;
			retval = true;
		}

		return retval;
	}

	bool keyReleased( OIS::KeyCode key )
	{
		bool retval = false;
		Ogre::Vector3 vec = _move_keys.findKey(key);
		if( vec != Ogre::Vector3::ZERO )
		{
			_move_dir -= vec;
			retval = true;
		}
		
		vec = _rot_keys.findKey(key);
		if( vec != Ogre::Vector3::ZERO )
		{
			_rotation_axises -= vec;
			retval = true;
		}

		return retval;
	}

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

}	// namespace eqOgre

#endif // EQ_OGRE_TRANSFORM_EVENT_HPP