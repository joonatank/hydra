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
	bool operator==( Trigger const &other )
	{
		return ((typeid(*this) == typeid(other)) && isEqual(other));
	}

	virtual bool isEqual( Trigger const &other ) = 0;

};

class KeyTrigger : public Trigger
{
public :
	KeyTrigger( OIS::KeyCode key, bool released );

	virtual bool isEqual( Trigger const &other );


private :
	OIS::KeyCode _key;
	bool _released;
};

class Operation
{
public :
	virtual void operator()( void ) = 0;
};



// Some test operations
// NOTE Don't try to use function pointers
// Just create a separate operation for all classes

/**	Base class for all Events
	Event has multiple Triggers
	But only one Operation
	All Triggers trigger the same operation
	*/
class Event
{
public :
	Event( Operation *oper = 0, Trigger *trig = 0 );

	virtual ~Event( void ) {}
	// Goes through the triggers in this event and if the trigger passed here
	// is present the appropriate Operation is performed.
	// Returns true if the Trigger triggered something
	// false otherwise.
	virtual bool processTrigger( Trigger *trig );

	bool removeTrigger( Trigger *trig );

	bool addTrigger( Trigger *trig );

	void setOperation( Operation *oper );

protected :
	std::vector<Trigger *>::iterator _findTrigger( Trigger *trig );
	
	std::vector< Trigger *> _triggers;
	Operation *_operation;

};	// class Event


class ToggleEvent : public Event
{
public :
	ToggleEvent( bool toggle_state,
				 Operation *toggleOn = 0,
				 Operation *toggleOff = 0,
				 Trigger *trig = 0 );

	/// Toggle Operations need the operation to be divided into two distinct ones
	/// This function handles the state management
	/// And calls the overridable toggleOn and toggleOff functions
	virtual bool processTrigger( Trigger *trig );

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