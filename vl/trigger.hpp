/**	Joonatan Kuosa
 *	2010-12
 *
 *	Event Handling Trigger class
 */

#ifndef VL_TRIGGER_HPP
#define VL_TRIGGER_HPP

#include "keycode.hpp"

#include "action.hpp"

namespace vl
{

// TODO add str conversions to python
// TODO add event to print all the events (do this in python)


/// Base class for all Triggers
/// Abstract
class Trigger
{
public :
	virtual std::string const &getTypeName( void ) const = 0;

	virtual std::string getName( void ) const = 0;

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


// TODO these two should be moved to a template
// They both have the same functions and a container
// the only difference is the update function, which can be implemented in
// a derived class
// Also there is going to be at least one more of these trigger classes
// for floating point updates
class BasicActionTrigger : public vl::Trigger
{
public :
	BasicActionTrigger( void ) {}

	/// Action to execute when updated
	void addAction( BasicActionPtr action );

	BasicActionPtr getAction( size_t i );

	size_t getNActions( void ) const
	{ return _actions.size(); }

	/// Callback function
	void update( void );

protected :
	std::vector<BasicActionPtr> _actions;

};	// class BasicActionTrigger


class TransformActionTrigger : public vl::Trigger
{
public :
	TransformActionTrigger( void );

	/// Action to execute when updated
	// TODO there should be a stack of actions not just one of them
	void setAction( TransformActionPtr action );

	TransformActionPtr getAction( void )
	{ return _action; }

	/// Callback function
	void update( Transform const &data );

protected :
	TransformActionPtr _action;

	Transform _value;

};	// class TransformActionTrigger


class KeyTrigger : public BasicActionTrigger
{
public :
	KeyTrigger( void );

	// TODO these should not be in the public interface
	// They should be only accessible when created by the EventManager
	void setKey( OIS::KeyCode key )
	{ _key = key; }

	OIS::KeyCode getKey( void ) const
	{ return _key; }

	virtual std::string const &getTypeName( void ) const;

	virtual std::ostream & print( std::ostream & os ) const;

	virtual std::string getName( void ) const
	{ return vl::getKeyName( _key ); }

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
// TODO should be a float trigger, so that the frame time can be passed
// the actions this executes
class FrameTrigger : public BasicActionTrigger
{
public :
	FrameTrigger( void )
		: _delta_time(0)
	{}

	virtual std::string const &getTypeName( void ) const;

	void setDeltaTime( double delta_time )
	{ _delta_time = delta_time; }

	virtual double value( void ) const
	{ return _delta_time; }

	virtual std::string getName( void ) const
	{ return "FrameTrigger"; }

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

}	// namespace vl

#endif // VL_TRIGGER_HPP