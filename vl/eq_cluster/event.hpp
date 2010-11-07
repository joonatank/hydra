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

	virtual bool isEqual( Trigger const &other ) const = 0;

	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << "Trigger : " << getTypeName() << std::endl;
		return os;
	}
};

inline std::ostream &operator<<( std::ostream &os, eqOgre::Trigger const &t )
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
	
	void setReleased( bool released )
	{ _released = released; }

	bool getReleased( void ) const
	{ return _released;}

	virtual bool isEqual( Trigger const &other ) const;

	virtual std::string const &getTypeName( void ) const;

	static const std::string TYPENAME;

	virtual std::ostream & print( std::ostream & os ) const
	{
		Trigger::print(os);
		os << " KeyCode = " << getKeyName(_key) << " : released = ";
		if( _released == true )
			os << "true";
		else
			os << "false";

		return os;
	}

private :
	OIS::KeyCode _key;
	bool _released;
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




/// Trigger that is triggered when a frame is rendered
class FrameTrigger : public Trigger
{
public :
	virtual bool isEqual( Trigger const &other ) const
	{
		return true;
	}

	virtual std::string const &getTypeName( void ) const;

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



class Operation
{
public :
	virtual void execute( void ) = 0;

	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << "Operation of type = " << getTypeName() << std::endl;
		return os;
	}

};

inline std::ostream &operator<<( std::ostream &os, eqOgre::Operation const &o )
{ return o.print(os); }


/// Trigger Factory class for creating new Triggers
class OperationFactory
{
public :
	virtual Operation *create( void ) = 0;
	virtual std::string const &getTypeName( void ) const = 0;
};

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

	Operation *getOperation( void )
	{ return _operation; }
	
	void setTimeLimit( double time_limit )
	{ _time_limit = time_limit; }

	double getTimeLimit( void ) const
	{ return _time_limit; }

	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const;

protected :
	std::vector<Trigger *>::iterator _findTrigger( Trigger *trig );
	
	std::vector< Trigger *> _triggers;
	Operation *_operation;

	::clock_t _last_time;
	double _time_limit;

};	// class Event

inline std::ostream &operator<<( std::ostream &os, eqOgre::Event const &e )
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
	BasicEvent( Operation *oper = 0, Trigger *trig = 0, double time_limit = 0 )
		: Event( oper, trig, time_limit )
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
	ToggleEvent( bool toggle_state = false,
				 Operation *toggleOn = 0,
				 Operation *toggleOff = 0,
				 Trigger *trig = 0 );

	/// Toggle Operations need the operation to be divided into two distinct ones
	/// This function handles the state management
	/// And calls the overridable toggleOn and toggleOff functions
	virtual bool processTrigger( Trigger *trig );

	virtual std::string const &getTypeName( void ) const;

	// TODO override the Event::print

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

class ToggleEventFactory : public EventFactory
{
public :
	virtual Event *create( void )
	{ return new ToggleEvent; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};



/**	EventHandler
	Has multiple Events
	*/

}	// namespace eqOgre

#endif // EQ_OGRE_EVENT_HPP