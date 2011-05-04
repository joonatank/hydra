/**	Joonatan Kuosa
 *	2010-12
 *
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
	_action->execute();
}

/// ------------- TransformActionTrigger -----------------
vl::TransformActionTrigger::TransformActionTrigger( void )
	: _action(0)
{}

void 
vl::TransformActionTrigger::setAction(vl::TransformActionPtr action)
{
	if( _action != action )
	{
		_action = action;

		update(_value);
	}
}

void
vl::TransformActionTrigger::update(const vl::Transform& data)
{
	// Copy the data for futher reference
	_value = data;
	if( _action )
	{
		_action->execute(data);
	}
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
		if( state == KS_DOWN && _action_down )
		{
			_action_down->execute();
		}
		else if( state == KS_UP && _action_up )
		{
			_action_up->execute();
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
