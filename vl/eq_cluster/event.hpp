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

	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const;

protected :
	std::vector<Trigger *>::iterator _findTrigger( Trigger *trig );

	std::vector< Trigger *> _triggers;
	BasicActionPtr _action;

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


}	// namespace vl

#endif // VL_EVENT_HPP
