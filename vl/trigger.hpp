/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file trigger.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**	
 *	Event Handling Trigger class
 *	
 *	2011-07 Updated to use boost::signals
 */

#ifndef HYDRA_TRIGGER_HPP
#define HYDRA_TRIGGER_HPP

#include <boost/signal.hpp>

#include "keycode.hpp"

#include "math/transform.hpp"

#include "base/time.hpp"

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
	typedef boost::signal<void ()> Tripped;
public :
	BasicActionTrigger(void);

	/// Callback function
	void update(void);

	int addListener(Tripped::slot_type const &slot)
	{ _signal.connect(slot); return 1; }

protected :
	Tripped _signal;

};	// class BasicActionTrigger

class TransformActionTrigger : public vl::Trigger
{
	typedef boost::signal<void (Transform const &)> Tripped;

public :
	TransformActionTrigger(void);

	/// Callback function
	void update(Transform const &data);

	int addListener(Tripped::slot_type const &slot);

protected :
	Tripped _signal;

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
	typedef boost::signal<void ()> Tripped;

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

	KEY_STATE getState(void) const
	{ return _state; }

	/// @brief Callback function
	/// @param state wether key is pressed or released
	/// Does checking if the call does not invoke state change no action is executed
	/// Action down or up can be NULL, that action is not taken even though the state
	/// change happens.
	/// Also Trigger is valid even though both actions are zero.
	void update(KEY_STATE state);

	int addKeyDownListener(Tripped::slot_type const &slot)
	{ _key_down_signal.connect(slot); return 1; }

	int addKeyUpListener(Tripped::slot_type const &slot)
	{ _key_up_signal.connect(slot); return 1; }

	// Syntax sugar, common name for all listeners
	// provides just a common key pressed/released functionality when user does not care which is tripped
	int addListener(Tripped::slot_type const &slot)
	{ _key_down_signal.connect(slot); return 1; }

	virtual std::string getTypeName( void ) const
	{ return "KeyTrigger"; }

	virtual std::ostream & print( std::ostream & os ) const;

	virtual std::string getName( void ) const
	{ return vl::getKeyName( _key ); }

private :
	Tripped _key_down_signal;
	Tripped _key_up_signal;

	OIS::KeyCode _key;
	KEY_MOD _modifiers;

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
class FrameTrigger : public Trigger
{
	typedef boost::signal<void (vl::time const &)> Tripped;
public :
	FrameTrigger( void )
	{}

	virtual std::string getTypeName( void ) const
	{ return "FrameTrigger"; }

//	void setDeltaTime( double delta_time )
//	{ _delta_time = delta_time; }

	virtual double value( void ) const
	{ return _delta_time; }

	virtual std::string getName( void ) const
	{ return "FrameTrigger"; }

	/// Callback function
	void update(vl::time const &elapsed_time)
	{
		_delta_time = elapsed_time;
		_signal(elapsed_time);
	}

	int addListener(Tripped::slot_type const &slot)
	{ _signal.connect(slot); return 1; }

private :
	Tripped _signal;

	vl::time _delta_time;

};

class TimeTrigger : public Trigger
{
	typedef boost::signal<void (void)> Tripped;
public :
	/// Initialises parameters so that the trigger is invalid till user
	/// changes the parameters
	TimeTrigger(void)
		: _continuous(true)
		, _expired(true)
	{}

	~TimeTrigger(void) {}

	virtual std::string getTypeName(void) const
	{ return "TimeTrigger"; }

	virtual std::string getName(void) const
	{ return "unnamed"; }

	int addListener(Tripped::slot_type const &slot)
	{ _signal.connect(slot); return 1; }

	bool isExpired(void) const
	{ return _expired; }

	void reset(void)
	{
		_expired = false;
		_time = vl::time();
	}

	vl::time const &getInterval(void) const
	{ return _interval; }

	void setInterval(vl::time const &t)
	{
		_initialise();
		_interval = t;
	}

	bool isContinous(void) const
	{ return _continuous; }

	void setContinous(bool cont)
	{
		_initialise();
		_continuous = cont;
	}

	/// @brief progress function
	/// @param elapsed_time time since last call to this function
	void update(vl::time const &elapsed_time);

private :
	void _initialise(void)
	{
		if((_expired && _continuous) || (_expired && _interval == vl::time()))
		{
			_expired = false;
		}
	}

	Tripped _signal;

	bool _continuous;

	vl::time _interval;

	vl::time _time;

	bool _expired;

};

}	// namespace vl

#endif // HYDRA_TRIGGER_HPP
