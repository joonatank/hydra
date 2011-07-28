/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file trigger.hpp
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
	virtual std::string getTypeName( void ) const = 0;

	virtual std::string getName( void ) const = 0;

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << "Trigger : " << getTypeName() << std::endl;
		return os;
	}

};

inline std::ostream &operator<<( std::ostream &os, vl::Trigger const &t )
{ return t.print( os ); }

// TODO these two should be moved to a template
// They both have the same functions and a container
// the only difference is the update function, which can be implemented in
// a derived class
// Also there is going to be at least one more of these trigger classes
// for floating point updates
class BasicActionTrigger : public vl::Trigger
{
public :
	BasicActionTrigger(void);

	/// @brief Action to execute when updated, a group of multiple actions
	/// Can not be replaced and will always exist
	GroupActionProxyPtr getAction(void)
	{ return _action; }

	/// Callback function
	void update(void);

protected :
	GroupActionProxyPtr _action;

};	// class BasicActionTrigger

class TransformActionTrigger : public vl::Trigger
{
public :
	TransformActionTrigger(void);

	/// @brief Action to execute when updated, a group of multiple actions
	/// Can not be replaced and will always exist
	GroupTransformActionProxyPtr getAction(void)
	{ return _action; }

	/// Callback function
	void update(Transform const &data);

protected :
	GroupTransformActionProxyPtr _action;

	Transform _value;

};	// class TransformActionTrigger

/// Keyboard modifiers
enum KEY_MOD
{
	KEY_MOD_NONE = 0,		// No modifier is on
	KEY_MOD_META = 1 << 1,	// Alt modifier, either alt
	KEY_MOD_CTRL = 1 << 2,	// Control modifier, either control
	KEY_MOD_SHIFT = 1 << 3,	// Shift modifier, either shift
	KEY_MOD_SUPER = 1 << 4,	// Windows key modifier
	// TODO missing menu modifier if one likes it
};

class KeyTrigger : public Trigger
{
public :
	enum KEY_STATE
	{
		KS_DOWN,
		KS_UP,
	};

	KeyTrigger(void);

	// TODO these should not be in the public interface
	// They should be only accessible when created by the EventManager
	void setKey( OIS::KeyCode key )
	{ _key = key; }

	OIS::KeyCode getKey( void ) const
	{ return _key; }

	void setModifiers( KEY_MOD mod )
	{ _modifiers = mod; }

	KEY_MOD getModifiers( void ) const
	{ return _modifiers; }

	void setKeyDownAction(BasicActionPtr action)
	{ _action_down = action; }

	BasicActionPtr getKeyDownAction(void)
	{ return _action_down; }

	void setKeyUpAction(BasicActionPtr action)
	{ _action_up = action; }

	BasicActionPtr getKeyUpAction(void)
	{ return _action_up; }

	KEY_STATE getState(void) const
	{ return _state; }

	/// @brief Callback function
	/// @param state wether key is pressed or released
	/// Does checking if the call does not invoke state change no action is executed
	/// Action down or up can be NULL, that action is not taken even though the state
	/// change happens.
	/// Also Trigger is valid even though both actions are zero.
	void update(KEY_STATE state);

	virtual std::string getTypeName( void ) const
	{ return "KeyTrigger"; }

	virtual std::ostream & print( std::ostream & os ) const;

	virtual std::string getName( void ) const
	{ return vl::getKeyName( _key ); }

private :
	OIS::KeyCode _key;
	KEY_MOD _modifiers;
	
	BasicActionPtr _action_down;
	BasicActionPtr _action_up;
	KEY_STATE _state;
};

/// @TODO this needs to use a complex structure for the trigger parameters
/// similar system that is used with key triggers will not really cut it
/// Because of the amount of parameters that can be used.
class JoystickTrigger : public BasicActionTrigger
{
public :
	virtual std::string getTypeName( void ) const
	{ return "JoystickTrigger"; }

	virtual std::string getName( void ) const
	{ return std::string(); }

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

	virtual std::string getTypeName( void ) const
	{ return "FrameTrigger"; }

	void setDeltaTime( double delta_time )
	{ _delta_time = delta_time; }

	virtual double value( void ) const
	{ return _delta_time; }

	virtual std::string getName( void ) const
	{ return "FrameTrigger"; }

protected :
	double _delta_time;

};

}	// namespace vl

#endif // VL_TRIGGER_HPP
