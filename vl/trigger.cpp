/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file trigger.cpp
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
 */

#include "trigger.hpp"


/// ----------------- BasicActionTrigger -------------------
vl::BasicActionTrigger::BasicActionTrigger(void)
	: _action(new GroupActionProxy)
{}

void
vl::BasicActionTrigger::update(void)
{
	if(_action)
	{ _action->execute(); }
	_signal();
}

/// ------------- TransformActionTrigger -----------------
vl::TransformActionTrigger::TransformActionTrigger( void )
	: _action(new GroupTransformActionProxy)
{}

void
vl::TransformActionTrigger::update(vl::Transform const &data)
{
	// Copy the data for futher reference
	_value = data;
	_action->execute(_value);
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
	, _action_down(0)
	, _action_up(0)
	, _state(KS_UP)
{}

void 
vl::KeyTrigger::update(vl::KeyTrigger::KEY_STATE state)
{
	if( _state != state )
	{
		if(state == KS_DOWN)
		{
			if(_action_down)
			{ _action_down->execute(); }
			_key_down_signal();
		}
		else if( state == KS_UP)
		{
			if(_action_up)
			{ _action_up->execute(); }
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
