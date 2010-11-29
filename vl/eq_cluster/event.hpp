/**	Joonatan Kuosa
 *	2010-11
 *
 *	Event Handling is divided to Triggers, Events and Actions
 *	Triggers are processed in Config which passes all expired triggers
 *	to all Events which in turn will invoke correct Actions
 *
 *	Triggers can be added to an Event using appropriate methods
 *	(depends on the Event)
 *	Same for Actions
 */

#ifndef VL_EVENT_HPP
#define VL_EVENT_HPP

// TODO these should be replaced
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include <vector>
#include <typeinfo>
#include <iostream>

#include <eq/base/clock.h>

#include "keycode.hpp"

namespace vl
{
// TODO add str conversions to python
// TODO add event to print all the events (do this in python)


/// Base class for all Triggers
/// Abstract
class Trigger
{
public :
	bool operator==( Trigger const &other ) const
	{
		return ((typeid(*this) == typeid(other)) && isEqual(other));
	}

	/// Returns wether Triggers are similar enough
	/// Specifics depend on the derived class and how they handle this
	/// but basicly this returns true if the this is a specialisation of other
	/// or they are equal.
	bool isSimilar( Trigger const *other ) const
	{
		return (*this == *other || isSpecialisation( other ) );
	}

	/// Returns true if is a specialisation of other
	virtual bool isSpecialisation( Trigger const *other ) const = 0;

	virtual double value( void ) const = 0;

	virtual bool isEqual( Trigger const &other ) const = 0;

	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << "Trigger : " << getTypeName() << std::endl;
		return os;
	}
};

inline std::ostream &operator<<( std::ostream &os, vl::Trigger const &t )
{ return t.print( os ); }


/// Trigger Factory class for creating new Triggers
class TriggerFactory
{
public :
	virtual Trigger *create( void ) = 0;
	virtual std::string const &getTypeName( void ) const = 0;
};

class KeyTrigger : public Trigger
{
public :
	KeyTrigger( void );

	void setKey( OIS::KeyCode key )
	{ _key = key; }

	OIS::KeyCode getKey( void ) const
	{ return _key; }

	virtual double value( void ) const
	{ return 1; }

	virtual bool isSpecialisation( Trigger const *other ) const;

	virtual bool isEqual( Trigger const &other ) const;

	virtual std::string const &getTypeName( void ) const;

	virtual std::ostream & print( std::ostream & os ) const;

private :
	OIS::KeyCode _key;
};


class KeyTriggerFactory : public TriggerFactory
{
public :
	virtual Trigger *create( void )
	{ return new KeyTrigger; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


/// Specialization of KeyTrigger that checks if that the key is pressed
/// All triggers generated by keyEvents are of these specializations
class KeyPressedTrigger : public KeyTrigger
{
	virtual std::string const &getTypeName( void ) const;

	/// Pressed always returns 1 (positive)
	virtual double value( void ) const
	{ return 1; }

	virtual bool isSpecialisation( Trigger const *other ) const;
};

class KeyPressedTriggerFactory : public TriggerFactory
{
public :
	virtual Trigger *create( void )
	{ return new KeyPressedTrigger; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

/// Specialization of KeyTrigger that checks if that the key is released
/// All triggers generated by keyEvents are of these specializations
class KeyReleasedTrigger : public KeyTrigger
{
	virtual std::string const &getTypeName( void ) const;

	/// Released always returns -1 (negative)
	virtual double value( void ) const
	{ return -1; }

	virtual bool isSpecialisation( Trigger const *other ) const;

};

class KeyReleasedTriggerFactory : public TriggerFactory
{
public :
	virtual Trigger *create( void )
	{ return new KeyReleasedTrigger; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

/// Trigger that is triggered when a frame is rendered
class FrameTrigger : public Trigger
{
public :
	FrameTrigger( void )
		: _delta_time(0)
	{}

	virtual bool isEqual( Trigger const &other ) const
	{
		return true;
	}

	virtual std::string const &getTypeName( void ) const;

	void setDeltaTime( double delta_time )
	{ _delta_time = delta_time; }

	virtual double value( void ) const
	{ return _delta_time; }

	virtual bool isSpecialisation( Trigger const *other ) const
	{ return false; }

protected :
	double _delta_time;

};

class FrameTriggerFactory : public TriggerFactory
{
public :
	virtual Trigger *create( void )
	{ return new KeyTrigger; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};



struct Transform
{
	Transform( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO,
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: position( pos ), quaternion( rot )
	{}

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

inline std::ostream &operator<<( std::ostream &os, Transform const &d )
{
	os << "Position = " << d.position << " : Orientation = " << d.quaternion;

	return os;
}


/// Actions

/// Action is divided into different Action types depending on the input parameters for execute
/// Simple ones with no parameters
/// and more complex like transformation operations which need the Transformation
class Action
{
public :
	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << "Operation of type = " << getTypeName() << std::endl;
		return os;
	}

};

typedef Action * ActionPtr;

inline std::ostream &operator<<( std::ostream &os, vl::Action const &a )
{ return a.print(os); }

/// Action Factory class for creating new Actions
class ActionFactory
{
public :
	virtual ActionPtr create( void ) = 0;
	virtual std::string const &getTypeName( void ) const = 0;
};

/// The most basic action, takes no parameters
class BasicAction : public Action
{
public :
	virtual void execute( void ) = 0;
};

typedef BasicAction * BasicActionPtr;

/// Callback Action class designed for Trackers
/// Could be expanded for use with anything that sets the object transformation
// For now the Tracker Triggers are the test case
class TransformAction : public Action
{
public :
	/// Callback function for TrackerTrigger
	/// Called when new data is received from the tracker
	virtual void execute( Transform const &data ) = 0;

};

typedef TransformAction * TransformActionPtr;




/**	Base class for all Events
	Abstract interface
	Event has multiple Triggers
	But only one Action
	All Triggers trigger the same operation
	*/
// TODO methods not common to all events should be moved to respective
// classes
class Event
{
public :
	Event( void );

	virtual ~Event( void ) {}
	// Goes through the triggers in this event and if the trigger passed here
	// is present the appropriate Operation is performed.
	// Returns true if the Trigger triggered something
	// false otherwise.
	virtual bool processTrigger( Trigger *trig );

	bool removeTrigger( Trigger *trig );

	bool addTrigger( Trigger *trig );

	void setAction( BasicActionPtr action );

	BasicActionPtr getAction( void )
	{ return _action; }

	/// Set the time limit between the same actions beign triggered
	/// parameter time_limit in seconds
	void setTimeLimit( double time_limit )
	{ _time_limit = time_limit*1000; }

	/// Get the time limit between actions beign triggered
	/// returns the time limit in seconds
	double getTimeLimit( void ) const
	{ return _time_limit/1000; }

	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const;

protected :
	std::vector<Trigger *>::iterator _findTrigger( Trigger *trig );

	std::vector< Trigger *> _triggers;
	BasicActionPtr _action;


	eq::base::Clock _clock;

	// Time limit in milliseconds, easier to compare to clock output
	double _time_limit;

};	// class Event

inline std::ostream &operator<<( std::ostream &os, vl::Event const &e )
{
	return e.print(os);
}

/// Trigger Factory class for creating new Triggers
class EventFactory
{
public :
	virtual Event *create( void ) = 0;
	virtual std::string const &getTypeName( void ) const = 0;
};


/// Basic Event implementation used for Trigger Events
class BasicEvent : public Event
{
public :
	BasicEvent( void )
	{}

	virtual std::string const &getTypeName( void ) const;

};

class BasicEventFactory : public EventFactory
{
public :
	virtual Event *create( void )
	{ return new BasicEvent; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};



class ToggleEvent : public Event
{
public :
	// TODO this needs functions to set all parameters
	// Constuctor needs to cleaned as the factory can not set the parameters
	ToggleEvent( void );

	void setToggleOn( BasicActionPtr toggleOn )
	{ _toggleOn = toggleOn; }

	BasicActionPtr getToggleOn( void )
	{ return _toggleOn; }

	void setToggleOff( BasicActionPtr toggleOff )
	{ _toggleOff = toggleOff; }

	BasicActionPtr getToggleOff( void )
	{ return _toggleOff; }

	void setToggleState( bool state )
	{ _state = state; }

	bool getToggleState( void )
	{ return _state; }

	/// Toggle Operations need the operation to be divided into two distinct ones
	/// This function handles the state management
	/// And calls the overridable toggleOn and toggleOff functions
	virtual bool processTrigger( Trigger *trig );

	virtual std::string const &getTypeName( void ) const;

	// TODO override the Event::print

private :
	// Forbid some parent methods
	void setAction( BasicActionPtr action ) {}

	BasicActionPtr getAction( void ) { return 0; }

	/// The real function called when the Operation changes to toggled state
	void toggleOn( void );

	/// The real function caleed when the Operation changes from toggled state
	void toggleOff( void );

	/// Toggle value, should not be exposed to inherited classes
	bool _state;

	BasicActionPtr _toggleOn;
	BasicActionPtr _toggleOff;

};	// ToggleEvent

class ToggleEventFactory : public EventFactory
{
public :
	virtual Event *create( void )
	{ return new ToggleEvent; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

}	// namespace vl

#endif // VL_EVENT_HPP
