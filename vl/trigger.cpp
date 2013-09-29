/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file trigger.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**
 *	Event Handling Trigger class
 */

#include "trigger.hpp"


/// ----------------- BasicActionTrigger -------------------
vl::BasicActionTrigger::BasicActionTrigger(void)
{}

void
vl::BasicActionTrigger::update(void)
{
	_signal();
}

/// ------------- TransformActionTrigger -----------------
vl::TransformActionTrigger::TransformActionTrigger( void )
{}

void
vl::TransformActionTrigger::update(vl::Transform const &data)
{
	// Copy the data for futher reference
	_value = data;
	_signal(_value);
}

int
vl::TransformActionTrigger::addListener(vl::TransformActionTrigger::Tripped::slot_type const &slot)
{
	_signal.connect(slot);
		
	// Call the slot with old value
	// this calls needlesly the other handlers also
	_signal(_value);

	return 1; 
}


/// KeyTrigger Public
vl::KeyTrigger::KeyTrigger(void)
	: _key( OIS::KC_UNASSIGNED )
	, _modifiers(KEY_MOD_NONE)
	, _state(KS_UP)
{}

void 
vl::KeyTrigger::update(vl::KeyTrigger::KEY_STATE state)
{
	if( _state != state )
	{
		if(state == KS_DOWN)
		{
			_key_down_signal();
		}
		else if( state == KS_UP)
		{
			_key_up_signal();
		}
		_state = state;
	}
}

std::ostream &
vl::KeyTrigger::print(std::ostream& os) const
{
	Trigger::print(os);
	os << " KeyCode = " << vl::getKeyName(_key) << " : released = ";

	return os;
}

///---JoystickTrigger-------

int
vl::JoystickTrigger::addListener(Joystick_signal_t::slot_type const &slot)
{
	_joystick_signal.connect(slot);
	return 1;
};

void
vl::JoystickTrigger::update(vl::JoystickEvent const& evt, vl::JoystickEvent::EventType type, int index)
{ 
	// enable if we can't get this to python
//	std::clog << "update funktio, trigger.cpp" << evt;
	_joystick_signal(evt, type, index);
}



///---Mouse trigger------------------------------------------------------
// @todo: korjaa nää järkeviks, ihan hirveetä boilerplatescheissekoodia:
void
vl::MouseTrigger::update(vl::MouseEvent const &evt, vl::MouseTrigger::MOUSE_STATE ms, vl::MouseEvent::BUTTON b_id)
{
	//@todo: what's the point in mouse state? What if we press multiple mouse buttons down without releasing the first? This code would
	//reject sending signal. We don't want this functionality so remove mouse state asap.
	if(ms != _mstate)
	{
		if(ms == vl::MouseTrigger::MS_PRESSED)
		{
			_button_pressed_signal(evt, b_id);
		}
		else if(ms == vl::MouseTrigger::MS_RELEASED)
		{
			_button_released_signal(evt, b_id);
		}
		else
		{
			//do nothing
		}
		_mstate = ms;
	}
}

//Mouse moved must be updated all the time mouse is really moved so no mouse state is needed:
void
vl::MouseTrigger::update(vl::MouseEvent const &evt, vl::MouseTrigger::MOUSE_STATE ms)
{
	if(ms != _mstate)
	{
		if( ms == vl::MouseTrigger::MS_MOVED )
		{
			_mouse_moved_signal(evt);
		}
	_mstate = ms;
	}
	
}

int
vl::MouseTrigger::addMovedListener(vl::MouseTrigger::Tripped_moved::slot_type const &slot)
{
	_mouse_moved_signal.connect(slot);
	return 1;
}

int
vl::MouseTrigger::addButtonPressedListener(vl::MouseTrigger::Tripped_button::slot_type const &slot)
{
	_button_pressed_signal.connect(slot);
	return 1;
}

int
vl::MouseTrigger::addButtonReleasedListener(vl::MouseTrigger::Tripped_button::slot_type const &slot)
{
	_button_released_signal.connect(slot);
	return 1;
}





/// Time triggers

void
vl::TimeTrigger::update(vl::time const &elapsed_time)
{
	if(!_expired)
	{
		_time += elapsed_time;
		if(_time >= _interval)
		{
			_signal();
			if(!_continuous)
			{ _expired = true; }
			_time = vl::time();
		}
	}
}
