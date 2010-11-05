
#include "event.hpp"

/// KeyTrigger Public
eqOgre::KeyTrigger::KeyTrigger(OIS::KeyCode key, bool released)
	: _key(key), _released(released)
{}

bool
eqOgre::KeyTrigger::isEqual(const eqOgre::Trigger& other)
{
	KeyTrigger const &key_other = static_cast<KeyTrigger const &>( other );
	if( key_other._key == _key && key_other._released == _released )
	{ return true; }

	return false;
}

/// Event Public
eqOgre::Event::Event(eqOgre::Operation* oper, eqOgre::Trigger* trig)
	: _operation(oper)
{
	if( trig )
	{ _triggers.push_back(trig); }
}

bool
eqOgre::Event::processTrigger(eqOgre::Trigger* trig)
{
	if( _findTrigger(trig) != _triggers.end() )
	{
		(*_operation)();
		return true;
	}

	return false;
}

bool
eqOgre::Event::removeTrigger(eqOgre::Trigger* trig)
{
	std::vector<Trigger *>::iterator iter = _findTrigger( trig );
	if( iter != _triggers.end() )
	{
		_triggers.erase(iter);
		return true;
	}

	return false;
}

bool
eqOgre::Event::addTrigger(eqOgre::Trigger* trig)
{
	// Only add Trigger once
	if( _findTrigger(trig) == _triggers.end() )
	{
		_triggers.push_back( trig );
		return true;
	}

	return false;
}
void
eqOgre::Event::setOperation(eqOgre::Operation* oper)
{ _operation = oper; }

/// Event protected
std::vector< eqOgre::Trigger * >::iterator
eqOgre::Event::_findTrigger(eqOgre::Trigger* trig)
{
	std::vector<Trigger *>::iterator iter = _triggers.begin();
	for( ; iter != _triggers.end(); ++iter )
	{
		if( *(*iter) == *trig )
		{ return iter; }
	}

	return _triggers.end();
}

/// ToggleEvent public
eqOgre::ToggleEvent::ToggleEvent(bool toggle_state, eqOgre::Operation* toggleOn, eqOgre::Operation* toggleOff, eqOgre::Trigger* trig)
	: eqOgre::Event( 0, trig ), _toggle(toggle_state), _toggleOn(toggleOn), _toggleOff(toggleOff)
{}

bool
eqOgre::ToggleEvent::processTrigger(eqOgre::Trigger* trig)
{
	if( _findTrigger(trig) != _triggers.end() )
	{
		if( _toggle )
		{
			toggleOff();
			_toggle = false;
		}
		else
		{
			toggleOn();
			_toggle = true;
		}
		return true;
	}
	return false;
}

/// ToggleEvent private
void
eqOgre::ToggleEvent::toggleOn(void )
{
	if( _toggleOn )
		(*_toggleOn)();
}

void
eqOgre::ToggleEvent::toggleOff(void )
{
	if( _toggleOff )
		(*_toggleOff)();
}
