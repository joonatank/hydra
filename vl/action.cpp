/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file action.cpp
 *
 *	Event Handling Action class
 */

#include "action.hpp"

/// ------------------------- ToggleActionProxy --------------------------------
void
vl::ToggleActionProxy::execute(void)
{
	// It's not a real problem if we only have one action but the toggle
	// will not work correctly till we have the other one
	if( _state && _action_off )
	{
		_action_off->execute();
		_state = !_state;
	}
	else if( !_state && _action_on )
	{
		_action_on->execute();
		_state = !_state;
	}
}

/// ------------------------- BufferActionProxy --------------------------------
void
vl::BufferActionProxy::execute(void)
{
	// TODO throw
	assert( _buffer.size() > _index );

	_buffer.at(_index)->execute();
	++_index;
	if( _index == _buffer.size() )
	{ _index = 0; }
}

void
vl::BufferActionProxy::remAction( vl::BasicActionPtr action )
{
	std::vector<BasicActionPtr>::iterator iter;
	for( iter = _buffer.begin(); iter != _buffer.end(); ++iter )
	{
		if( *iter == action )
		{
			_buffer.erase(iter);
			if( _index >= _buffer.size() )
			{ _index = 0; }
			break;
		}
	}

	// TODO throw
}

vl::BasicActionPtr
vl::BufferActionProxy::getAction( size_t index )
{
	if( index >= _buffer.size() )
	{ return 0; }

	return _buffer.at(index);
}

/// ----------------------- GroupActionProxy -----------------------------------
void
vl::GroupActionProxy::execute(void)
{
	std::vector<BasicActionPtr>::iterator iter;
	for( iter = _actions.begin(); iter != _actions.end(); ++iter )
	{
		(*iter)->execute();
	}
}

void
vl::GroupActionProxy::addAction(vl::BasicActionPtr action)
{
	/// Silently ignored if trying to add the same action multiple times
	if( !hasAction(action) )
	{ _actions.push_back(action); }
}

void
vl::GroupActionProxy::remAction( BasicActionPtr action )
{
	std::vector<BasicActionPtr>::iterator iter
		= std::find( _actions.begin(), _actions.end(), action );
	if( iter == _actions.end() )
	{
		// TODO throw
	}
	else
	{ _actions.erase(iter); }
}

bool
vl::GroupActionProxy::hasAction(vl::BasicActionPtr action) const
{
	std::vector<BasicActionPtr>::const_iterator iter
		= std::find( _actions.begin(), _actions.end(), action );
	if( iter == _actions.end() )
	{ return false; }
	else
	{ return true; }
}

void
vl::GroupTransformActionProxy::execute(vl::Transform const &t)
{
	std::vector<TransformActionPtr>::iterator iter;
	for( iter = _actions.begin(); iter != _actions.end(); ++iter )
	{ (*iter)->execute(t); }

	// Save for later use
	_value = t;
}

void
vl::GroupTransformActionProxy::addAction(vl::TransformActionPtr action)
{
	/// Silently ignored if trying to add the same action multiple times
	if( !hasAction(action) )
	{
		_actions.push_back(action);
		action->execute(_value);
	}
}

void
vl::GroupTransformActionProxy::remAction(vl::TransformActionPtr action)
{
	std::vector<TransformActionPtr>::iterator iter
		= std::find( _actions.begin(), _actions.end(), action );
	if( iter == _actions.end() )
	{
		// TODO throw
	}
	else
	{ _actions.erase(iter); }
}

bool
vl::GroupTransformActionProxy::hasAction(vl::TransformActionPtr action) const
{
	std::vector<TransformActionPtr>::const_iterator iter
		= std::find( _actions.begin(), _actions.end(), action );
	if( iter == _actions.end() )
	{ return false; }
	else
	{ return true; }
}

/// ---------------------------- TimerAction -----------------------------------
vl::TimerActionProxy::TimerActionProxy(void)
	: _action(0), _clock(10), _time_limit(0)
{}

void
vl::TimerActionProxy::execute(void)
{
	double time = _clock.elapsed();
	if( _action && time > _time_limit )
	{
		_action->execute();
		_clock.reset();
	}
}
