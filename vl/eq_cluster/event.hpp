/* Joonatan Kuosa
 * 2010-11
 *
 * Event Handling is divided to Triggers, Events and Operations
 * Triggers are processed in Config which passes all expired triggers
 * to all Events which in turn will invoke correct Operation
 *
 * Triggers can be added to an Event using appropriate methods
 * (depends on the Event)
 * Same for Operations
 */

#ifndef EQ_OGRE_EVENT_HPP
#define EQ_OGRE_EVENT_HPP

#include <vector>
#include <typeinfo>
#include <iostream>

#include "keycode.hpp"

namespace eqOgre
{

// Base class for all Triggers
class Trigger
{
public :
	bool operator==( Trigger const &other ) const
	{
		return ((typeid(*this) == typeid(other)) && isEqual(other));
	}

	virtual bool isEqual( Trigger const &other ) const = 0;

	virtual std::string const &getTypeName( void ) const = 0;
};

class KeyTrigger : public Trigger
{
public :
	KeyTrigger( void );

	void setKey( OIS::KeyCode key )
	{ _key = key; }
	
	void setReleased( bool released )
	{ _released = released; }

	virtual bool isEqual( Trigger const &other ) const;

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

private :
	OIS::KeyCode _key;
	bool _released;
};

/// Trigger that is triggered when a frame is rendered
class FrameTrigger : public Trigger
{
public :
	virtual bool isEqual( Trigger const &other ) const
	{
		return true;
	}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


class Operation
{
public :
	virtual void execute( void ) = 0;
//	virtual void operator()( void ) = 0;

	virtual std::string const &getTypeName( void ) const = 0;
};

// Some test operations
// NOTE Don't try to use function pointers
// Just create a separate operation for all classes

/**	Base class for all Events
	Abstract interface
	Event has multiple Triggers
	But only one Operation
	All Triggers trigger the same operation
	*/
class Event
{
public :
	Event( Operation *oper = 0, Trigger *trig = 0, double time_limit = 0 );

	virtual ~Event( void ) {}
	// Goes through the triggers in this event and if the trigger passed here
	// is present the appropriate Operation is performed.
	// Returns true if the Trigger triggered something
	// false otherwise.
	virtual bool processTrigger( Trigger *trig );

	bool removeTrigger( Trigger *trig );

	bool addTrigger( Trigger *trig );

	void setOperation( Operation *oper );

	void setTimeLimit( double time_limit )
	{ _time_limit = time_limit; }

	double getTimeLimit( void ) const
	{ return _time_limit; }

	virtual std::string const &getTypeName( void ) const = 0;

protected :
	std::vector<Trigger *>::iterator _findTrigger( Trigger *trig );
	
	std::vector< Trigger *> _triggers;
	Operation *_operation;

	::clock_t _last_time;
	double _time_limit;

};	// class Event

/// Basic Event implementation used for Trigger Events
class BasicEvent : public Event
{
public :
	BasicEvent( Operation *oper = 0, Trigger *trig = 0, double time_limit = 0 )
		: Event( oper, trig, time_limit )
	{}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }
	
	static const std::string TYPENAME;
};

class ToggleEvent : public Event
{
public :
	// TODO this needs functions to set all parameters
	// Constuctor needs to cleaned as the factory can not set the parameters
	ToggleEvent( bool toggle_state = false,
				 Operation *toggleOn = 0,
				 Operation *toggleOff = 0,
				 Trigger *trig = 0 );

	/// Toggle Operations need the operation to be divided into two distinct ones
	/// This function handles the state management
	/// And calls the overridable toggleOn and toggleOff functions
	virtual bool processTrigger( Trigger *trig );

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

private :

	/// The real function called when the Operation changes to toggled state
	void toggleOn( void );

	/// The real function caleed when the Operation changes from toggled state
	void toggleOff( void );

	/// Toggle value, should not be exposed to inherited classes
	bool _toggle;

	Operation *_toggleOn;
	Operation *_toggleOff;

};	// ToggleEvent

/**	EventHandler
	Has multiple Events
	*/

}	// namespace eqOgre

#endif // EQ_OGRE_EVENT_HPP