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
	KeyTrigger( OIS::KeyCode key, bool released )
		: _key(key), _released(released)
	{}

	virtual bool isEqual( Trigger const &other )
	{
		KeyTrigger const &key_other = static_cast<KeyTrigger const &>( other );
		if( key_other._key == _key && key_other._released == _released )
		{ return true; }

		return false;
	}

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
	Event( void )
		: _operation(0)
	{}

	virtual ~Event( void ) {}
	// Goes through the triggers in this event and if the trigger passed here
	// is present the appropriate Operation is performed.
	// Returns true if the Trigger triggered something
	// false otherwise.
	bool processTrigger( Trigger *trig )
	{
		if( _findTrigger(trig) != _triggers.end() )
		{
			(*_operation)();
			return true;
		}

		return false;
	}

	bool removeTrigger( Trigger *trig )
	{
		std::vector<Trigger *>::iterator iter = _findTrigger( trig );
		if( iter != _triggers.end() )
		{
			_triggers.erase(iter);
			return true;
		}

		return false;
	}

	bool addTrigger( Trigger *trig )
	{
		// Only add Trigger once
		if( _findTrigger(trig) == _triggers.end() )
		{
			_triggers.push_back( trig );
			return true;
		}

		return false;
	}

	void setOperation( Operation *oper )
	{ _operation = oper; }

private :
	std::vector<Trigger *>::iterator _findTrigger( Trigger *trig )
	{
		std::vector<Trigger *>::iterator iter = _triggers.begin();
		for( ; iter != _triggers.end(); ++iter )
		{
			if( *(*iter) == *trig )
			{ return iter; }
		}
		
		return _triggers.end();
	}
	
	std::vector< Trigger *> _triggers;
	Operation *_operation;

};	// class Event

/**	EventHandler
	Has multiple Events
	*/

}	// namespace eqOgre

#endif // EQ_OGRE_EVENT_HPP