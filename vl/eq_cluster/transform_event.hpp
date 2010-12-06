#ifndef EQ_OGRE_TRANSFORM_EVENT_HPP
#define EQ_OGRE_TRANSFORM_EVENT_HPP

#include "scene_node.hpp"
#include "keycode.hpp"
#include "base/exceptions.hpp"
#include "event.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include <eq/base/clock.h>

namespace eqOgre
{

// TODO this is not really an Operation,
// no execute(void) function
// No Factory class
class MoveOperation : public vl::Action
{
public :
	MoveOperation( void );

	// TODO this should be divided to two different functions
	// one for setting delta time
	// and other for executing the Operation as with other Operation classes
	virtual void execute( double time );

	virtual std::string getTypeName( void ) const
	{ return TYPENAME; }

	// TODO should be moved to Factory
    static const std::string TYPENAME;

	/// Parameters
	void setSceneNode( SceneNode *node )
	{ _node = node; }

	SceneNode *getSceneNode( void )
	{ return _node; }

	SceneNode const *getSceneNode( void ) const
	{ return _node; }

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
	{ _move_dir += v; }

	void addRotation( Ogre::Vector3 const &v )
	{ _rot_dir += v; }

protected :
	SceneNode *_node;

	/// Parameters
	/// Movement speed
	double _speed;

	/// Rotation speed
	Ogre::Radian _angular_speed;

	// Current heading
	Ogre::Vector3 _move_dir;
	Ogre::Vector3 _rot_dir;

};

/// Simple Event class that does transformation based on key events
/// Keeps track of the state of the object (moving, stopped)
/// Transforms a SceneNode provided
/// All parameters that need units are in SI units (e.g. speed is m/s)
class TransformationEvent : public vl::Event
{
public :

	class TriggerPair
	{
	public :
		TriggerPair( vl::Trigger *trig1 = 0, vl::Trigger *trig2 = 0 )
			: _trig_pos(trig1), _trig_neg(trig2)
		{}

		double findTrigger( vl::Trigger const *trig ) const
		{
			if( !trig )
			{ return 0; }

			if( _trig_pos  && _trig_pos->isSimilar(trig) )
			{ return trig->value(); }

			if( _trig_neg  && _trig_neg->isSimilar(trig) )
			{ return -trig->value(); }

			return 0;
		}

		vl::Trigger * _trig_pos;
		vl::Trigger * _trig_neg;
	};

	struct TriggerPairVector
	{
		TriggerPairVector( TriggerPair nx = TriggerPair(),
						   TriggerPair ny = TriggerPair(),
						   TriggerPair nz = TriggerPair() )
			: x(nx), y(ny), z(nz)
		{}

		TriggerPair x;
		TriggerPair y;
		TriggerPair z;

		/// Find the key event in the structure
		/// returns a Vector of -1, 0, 1 based on wether it was found and if
		/// it was a positive or negative control key.
		/// Return value can be used as movement direction vector,
		/// or a delta to such a vector.
		Ogre::Vector3 findTrigger( vl::Trigger const *trig )
		{
			Ogre::Vector3 vec;
			vec.x = x.findTrigger(trig);
			vec.y = y.findTrigger(trig);
			vec.z = z.findTrigger(trig);

			return vec;
		}
	};

	/// Constructor
	TransformationEvent( void );

	/// Destructor
	virtual ~TransformationEvent( void )
	{}

	virtual std::string const &getTypeName( void ) const;

	void setSceneNode( SceneNode *node )
	{ _operation->setSceneNode(node);; }

	SceneNode *getSceneNode( void )
	{ return _operation->getSceneNode(); }

	// TODO if we need this one, we need to provide overloads to python
//	SceneNode const *getSceneNode( void ) const
//	{ return _operation->getSceneNode(); }

	/// Called from event handling
	/// If the trigger is mapped to this TransformationEvent the state of the
	/// event is changed.
	/// Returns true if processed
	bool processTrigger(vl::Trigger* trig);

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
	void setTransXtrigger( vl::Trigger *trig_pos, vl::Trigger *trig_neg )
	{
		_trans_triggers.x = TriggerPair( trig_pos, trig_neg );
	}

	void setTransYtrigger( vl::Trigger *trig_pos, vl::Trigger *trig_neg )
	{
		_trans_triggers.y = TriggerPair( trig_pos, trig_neg );
	}

	void setTransZtrigger( vl::Trigger *trig_pos, vl::Trigger *trig_neg )
	{
		_trans_triggers.z = TriggerPair( trig_pos, trig_neg );
	}



	/// Rotation Triggers
	void setRotXtrigger( vl::Trigger *trig_pos, vl::Trigger *trig_neg )
	{
		_rot_triggers.x = TriggerPair( trig_pos, trig_neg );
	}

	void setRotYtrigger( vl::Trigger *trig_pos, vl::Trigger *trig_neg )
	{
		_rot_triggers.y = TriggerPair( trig_pos, trig_neg );
	}

	void setRotZtrigger( vl::Trigger *trig_pos, vl::Trigger *trig_neg )
	{
		_rot_triggers.z = TriggerPair( trig_pos, trig_neg );
	}

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
	eq::base::Clock _clock;

	/// Move operation owned by this class
	// TODO should be made to use the Operation schematics and created by the
	// EventManager
	MoveOperation *_operation;

	/// Triggers used for translation and rotation
	TriggerPairVector _trans_triggers;
	TriggerPairVector _rot_triggers;

};	// class TransformationEvent

class TransformationEventFactory : public vl::EventFactory
{
public :
	virtual vl::Event *create( void )
	{ return new TransformationEvent; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

}	// namespace eqOgre

#endif // EQ_OGRE_TRANSFORM_EVENT_HPP