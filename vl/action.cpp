/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file action.cpp
 *
 *	Event Handling Action class
 */

#include "action.hpp"

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
vl::GroupActionProxy::remAction( BasicActionPtr action )
{
	std::vector<BasicActionPtr>::iterator iter;
	for( iter = _actions.begin(); iter != _actions.end(); ++iter )
	{
		if( *iter == action )
		{
			_actions.erase(iter); 
			break;
		}
	}
}
