#ifndef EQ_OGRE_TRANSFORM_EVENT_HPP
#define EQ_OGRE_TRANSFORM_EVENT_HPP

#include "scene_node.hpp"
#include "keycode.hpp"
#include "base/exceptions.hpp"
#include "event.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

namespace eqOgre
{

/// Set a new transformation to a SceneNode
class SetTransformationOperation : public Operation
{
public :
	SetTransformationOperation( eqOgre::SceneNode *node,
								Ogre::Quaternion const &q = Ogre::Quaternion::IDENTITY,
								Ogre::Vector3 const &v = Ogre::Vector3::ZERO )
		: _node(node), _orientation(q), _position(v)
	{}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

	void setOrientation( Ogre::Quaternion const &q )
	{
		_orientation = q;
	}

	void setPosition( Ogre::Vector3 const &v )
	{
		_position = v;
	}

	virtual void operator()( void )
	{
		_node->setOrientation(_orientation);
		_node->setPosition(_position);
	}

protected :
	SceneNode *_node;

	Ogre::Quaternion _orientation;
	Ogre::Vector3 _position;
};

/// Modify existing transformation in a SceneNode
class TransformOperation : public Operation
{
public :
	TransformOperation( eqOgre::SceneNode *node,
						Ogre::Quaternion const &q = Ogre::Quaternion::IDENTITY,
						Ogre::Vector3 const &v = Ogre::Vector3::ZERO )
		: _node(node), _rotation(q), _translation(v)
	{
		if( _node )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
	}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

	void rotate( Ogre::Quaternion const &q )
	{
		_rotation = _rotation * q;
	}

	void translate( Ogre::Vector3 const &v )
	{
		_translation += v;
	}

	virtual void operator()( void )
	{
		_node->setOrientation( _node->getOrientation()*_rotation );
		_node->setPosition( _node->getPosition()+_translation );

		_translation = Ogre::Vector3::ZERO;
		_rotation = Ogre::Quaternion::IDENTITY;
	}

protected :
	SceneNode *_node;
	
	Ogre::Quaternion _rotation;
	Ogre::Vector3 _translation;
};

class MoveOperation : public Operation
{
public :
	MoveOperation( eqOgre::SceneNode *node )
		: _node(node), _move_dir(Ogre::Vector3::ZERO), _rot_dir(Ogre::Vector3::ZERO)
	{
		if( _node )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
	}

	virtual void execute( double time )
	{
		// Check that we have an object and we are moving
		if( _node && !_move_dir.isZeroLength())
		{
			Ogre::Vector3 pos = _node->getPosition();
			pos += _speed*time*_move_dir.normalisedCopy();
			_node->setPosition( pos );
		}

		if( _node && _rot_dir.isZeroLength() )
		{
			Ogre::Quaternion orient = _node->getOrientation();
			Ogre::Quaternion qx( _angular_speed*time, _rot_dir );
			_node->setOrientation( qx*orient );
		}
	}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

    static const std::string TYPENAME;

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

	void addMove( Ogre::Vector3 const &v )
	{
		_move_dir += v;
	}

	void addRotation( Ogre::Vector3 const &v )
	{
		_rot_dir += v;
	}

protected :
	SceneNode *_node;

	double _speed;
	Ogre::Radian _angular_speed;
	Ogre::Vector3 _move_dir;
	Ogre::Vector3 _rot_dir;

private :
	virtual void execute( void ) {}
};

// TODO really an EventHandler not an Event
// contains multiple events.
/// Simple Event class that does transformation based on key events
/// Keeps track of the state of the object (moving, stopped)
/// Transforms a SceneNode provided
/// All parameters that need units are in SI units (e.g. speed is m/s)
class TransformationEvent : public Event
{
public :
/*
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
*/
	/// Constructor
	// FIXME parameters need to be set with functions, factory creation
	TransformationEvent( SceneNode *node = 0 );

	/// Destructor
	virtual ~TransformationEvent( void )
	{}

	class TriggerPair
	{
	public :
		TriggerPair( Trigger *trig1 = 0, Trigger *trig2 = 0 )
			: _trig1(trig1), _trig2(trig2)
		{}

		Trigger * _trig1;
		Trigger * _trig2;
	};

    virtual std::string const &getTypeName( void ) const
    { return TYPENAME; }

    static const std::string TYPENAME;

/*
	void setSceneNode( SceneNode *node )
	{ _node = node; }

	SceneNode *getSceneNode( void )
	{ return _node; }

	SceneNode const *getSceneNode( void ) const
	{ return _node; }
*/
	/// Execute operator
	/// Transforms the SceneNode if it's in moving state
	void operator()( void );

	/// Called from event handling
	/// If the key is mapped to this TransformationEvent the state of the
	/// event is changed.
	/// Returns true if processed
//	bool keyPressed( OIS::KeyCode key );

//	bool keyReleased( OIS::KeyCode key );

	bool processTrigger(Trigger* trig);

	/// Parameters
	void setSpeed( double speed )
	{ _operation->setSpeed(speed); }

	double getSpeed( void ) const
	{ return _operation->getSpeed(); }

	/// Set the angular speed of the object in radians per second
	void setAngularSpeed( Ogre::Radian const &speed )
	{ _operation->setAngularSpeed(speed); }

	Ogre::Radian const &getAngularSpeed( void ) const
	{ return _operation->getAngularSpeed(); }


	/// These triggers will change the movement direction of the object
	/// they will not really move the object
	/// Translation Triggers
	void setTransXtrigger( Trigger *trig_pos, Trigger *trig_neg )
	{
		_trans_triggers.at(0) = TriggerPair( trig_pos, trig_neg );
	}

	void setTransYtrigger( Trigger *trig_pos, Trigger *trig_neg )
	{
		_trans_triggers.at(1) = TriggerPair( trig_pos, trig_neg );
	}

	void setTransZtrigger( Trigger *trig_pos, Trigger *trig_neg )
	{
		_trans_triggers.at(2) = TriggerPair( trig_pos, trig_neg );
	}



	/// Rotation Triggers
	void setRotXtrigger( Trigger *trig_pos, Trigger *trig_neg )
	{
		_rot_triggers.at(0) = TriggerPair( trig_pos, trig_neg );
	}

	void setRotYtrigger( Trigger *trig_pos, Trigger *trig_neg )
	{
		_rot_triggers.at(1) = TriggerPair( trig_pos, trig_neg );
	}

	void setRotZtrigger( Trigger *trig_pos, Trigger *trig_neg )
	{
		_rot_triggers.at(2) = TriggerPair( trig_pos, trig_neg );
	}


	friend bool operator==( TransformationEvent const &a, TransformationEvent const &b );
	friend std::ostream & operator<<( std::ostream &os, TransformationEvent const &a );

protected :
	/// Copy constructor
	/// Copies the prototype but does not copy any state information
	/// assumption is that the state information is only useful in the context
	/// where it has been setted.
	TransformationEvent( TransformationEvent const &a );

	TransformationEvent &operator=( TransformationEvent const &a );

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
//	KeyPairVec _move_keys;

	// Mapped keys to rotate around axises
//	KeyPairVec _rot_keys;

	// Direction where we are moving, is not unit length
	// For key events there is three possible values for each axis
	// -1, 0, 1
//	Ogre::Vector3 _move_dir;

//	Ogre::Vector3 _rotation_axises;
	MoveOperation *_operation;

	std::vector< TriggerPair > _trans_triggers;

	std::vector< TriggerPair >_rot_triggers;
};

/*
inline bool operator==( TransformationEvent::KeyPair const &a, TransformationEvent::KeyPair const &b )
{
	return( a.neg_key == b.neg_key && a.pos_key == b.pos_key );
}

inline bool operator==( TransformationEvent::KeyPairVec const &a, TransformationEvent::KeyPairVec const &b )
{
	return( a.x == b.x && a.y == b.y && a.z == b.z );
}
*/

/// Chech that the prototypes are equal but does not test the state information.
/// Assumption is that the state information is only useful in the context
/// where it has been setted.
// FIXME this is not completed
inline bool operator==( TransformationEvent const &a, TransformationEvent const &b )
{
	return( a._node == b._node
		&& a._speed == b._speed
		&& a._angular_speed == b._angular_speed
//		&& a._move_keys == b._move_keys
//		&& a._rot_keys == b._rot_keys
		);
}

/*
inline std::ostream & operator<<( std::ostream &os , TransformationEvent::KeyPair const &a )
{
	// TODO would be nice if the keys would be converted to human readable form
	os << "KeyPair : neg = " << getKeyName(a.neg_key) << " pos = "
		<< getKeyName(a.pos_key);

	return os;
}
*/

/*
inline std::ostream & operator<<(  std::ostream &os, TransformationEvent::KeyPairVec const &a )
{
	os << "x = " << a.x << " y = " << a.y << " z = " << a.z;
	return os;
}
*/

inline std::ostream & operator<<(  std::ostream &os, TransformationEvent const &a )
{
/*
	std::string del("     ");

	if( a.getSceneNode() )
		os << "TransformationEvent for node = " << a.getSceneNode()->getName();
	else
		os << "TransformationEvent without node";
	os << std::endl
		<< del << "speed = " << a.getSpeed()
		<< " m/s : " << " angular speed = " << a.getAngularSpeed() << std::endl;
//		<< del << "move keys = " << a._move_keys << std::endl
//		<< del << "rot keys = " << a._rot_keys;
*/
	return os;
}

}	// namespace eqOgre

#endif // EQ_OGRE_TRANSFORM_EVENT_HPP