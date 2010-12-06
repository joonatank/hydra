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

#include <vector>
#include <typeinfo>
#include <iostream>

#include <eq/base/clock.h>

#include "trigger.hpp"
#include "action.hpp"

namespace vl
{

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
