/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */
#include "event_manager.hpp"

#include "base/exceptions.hpp"

vl::EventManager::EventManager( void )
	: _frame_trigger(0)
	, _key_modifiers(KEY_MOD_NONE)
{}

vl::EventManager::~EventManager( void )
{
	delete _frame_trigger;

	for( std::vector<vl::TrackerTrigger *>::iterator iter = _tracker_triggers.begin();
		iter != _tracker_triggers.end(); ++iter )
	{
		delete *iter;
	}

	for( std::vector<vl::KeyPressedTrigger *>::iterator iter = _key_pressed_triggers.begin();
		iter != _key_pressed_triggers.end(); ++iter )
	{
		delete *iter;
	}

	for( std::vector<vl::KeyReleasedTrigger *>::iterator iter = _key_released_triggers.begin();
		iter != _key_released_triggers.end(); ++iter )
	{
		delete *iter;
	}
}


vl::TrackerTrigger *
vl::EventManager::createTrackerTrigger( std::string const &name )
{
	vl::TrackerTrigger *trigger = _findTrackerTrigger( name );
	if( !trigger )
	{
		trigger = new vl::TrackerTrigger;
		trigger->setName( name );
		_tracker_triggers.push_back( trigger );
	}

	return trigger;
}

vl::TrackerTrigger *
vl::EventManager::getTrackerTrigger(const std::string& name)
{
	vl::TrackerTrigger *trigger = _findTrackerTrigger( name );
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasTrackerTrigger(const std::string& name)
{
	if( _findTrackerTrigger( name ) )
	{ return true; }

	return false;
}

vl::KeyPressedTrigger *
vl::EventManager::createKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	vl::KeyPressedTrigger *trigger = _findKeyPressedTrigger(kc, mod);

	if( !trigger )
	{
		trigger = new vl::KeyPressedTrigger;
		trigger->setKey(kc);
		trigger->setModifiers(mod);
		_key_pressed_triggers.push_back(trigger);
	}

	return trigger;
}

vl::KeyPressedTrigger *
vl::EventManager::getKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	vl::KeyPressedTrigger *trigger = _findKeyPressedTrigger(kc,mod);
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	return _findKeyPressedTrigger(kc,mod);
}

void 
vl::EventManager::updateKeyPressedTrigger( OIS::KeyCode kc )
{
	if( getModifier(kc) != KEY_MOD_NONE )
	{
		// Add modifiers
		KEY_MOD new_key_modifiers = (KEY_MOD)(_key_modifiers | getModifier(kc));

		for( size_t i = 0; i < _keys_down.size(); ++i )
		{
			// Only release a trigger if we have one with a better match
			if( hasKeyReleasedTrigger( _keys_down.at(i), _key_modifiers ) )
			{
				getKeyReleasedTrigger( _keys_down.at(i), _key_modifiers )->update();
				if( hasKeyPressedTrigger( _keys_down.at(i), new_key_modifiers ) )
				{
					getKeyPressedTrigger( _keys_down.at(i), new_key_modifiers )->update();
				}
			}
		}

		_key_modifiers = new_key_modifiers;
	}
	else
	{ _keyDown(kc); }

	// Modifiers can also be used as event triggers.

	// Check if the there is a trigger for this event
	if( hasKeyPressedTrigger( kc, _key_modifiers ) )
	{
		getKeyPressedTrigger( kc, _key_modifiers )->update();
	}
}



vl::KeyReleasedTrigger *
vl::EventManager::createKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	vl::KeyReleasedTrigger *trigger = _findKeyReleasedTrigger(kc,mod);

	if( !trigger )
	{
		trigger = new vl::KeyReleasedTrigger;
		trigger->setKey(kc);
		trigger->setModifiers(mod);
		_key_released_triggers.push_back(trigger);
	}

	return trigger;
}

vl::KeyReleasedTrigger *
vl::EventManager::getKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	vl::KeyReleasedTrigger *trigger = _findKeyReleasedTrigger(kc, mod);
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	return _findKeyReleasedTrigger(kc, mod);
}

void 
vl::EventManager::updateKeyReleasedTrigger( OIS::KeyCode kc )
{
	// TODO if modifier is removed we should release the event with modifiers
	KEY_MOD modifier = getModifier(kc);
	if( modifier != KEY_MOD_NONE )
	{
		// Remove modifiers
		// Negate the modifiers, this will produce only once if none of them is released
		// and zero to the one that has been released. So and will produce all the once
		// not released.
		KEY_MOD new_key_modifiers = (KEY_MOD)(_key_modifiers & (~getModifier(kc)));

		// TODO this should release all the events with this modifier
		// needs a list of trigger events with this particular modifier
		// so that they can be released
		// FIXME it is not possible with our current event system.
		// We have no knowledge of the event states that particular keys or what
		// not are. So it can not be done.
		for( size_t i = 0; i < _keys_down.size(); ++i )
		{
			// Only release a trigger if we have one with better match
			if( hasKeyReleasedTrigger( _keys_down.at(i), _key_modifiers ) )
			{
				getKeyReleasedTrigger( _keys_down.at(i), _key_modifiers )->update();
				
				if( hasKeyPressedTrigger( _keys_down.at(i), new_key_modifiers ) )
				{
					getKeyPressedTrigger(_keys_down.at(i), new_key_modifiers)->update();
				}
			}
		}

		_key_modifiers = new_key_modifiers;
	}
	else
	{ _keyUp(kc); }

	// Modifiers can also be used as event triggers.

	// Check if the there is a trigger for this event
	// TODO we need to check all the different modifiers also, not just
	// for the exact match
	if( hasKeyReleasedTrigger( kc, _key_modifiers ) )
	{
		getKeyReleasedTrigger( kc, _key_modifiers )->update();
	}
}

vl::FrameTrigger *
vl::EventManager::getFrameTrigger( void )
{
	if( !_frame_trigger )
	{ _frame_trigger = new vl::FrameTrigger; }

	return _frame_trigger;
}


/// ----------- Protected -------------
vl::TrackerTrigger *
vl::EventManager::_findTrackerTrigger(const std::string& name)
{
	for( size_t i = 0; i < _tracker_triggers.size(); ++i )
	{
		vl::TrackerTrigger *trigger = _tracker_triggers.at(i);
		if( trigger->getName() == name )
		{ return trigger; }
	}

	return 0;
}

vl::KeyPressedTrigger *
vl::EventManager::_findKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	for( size_t i = 0; i < _key_pressed_triggers.size(); ++i )
	{
		vl::KeyPressedTrigger *trigger = _key_pressed_triggers.at(i);
		// exact match
		if( trigger->getKey() == kc && trigger->getModifiers() == mod )
		{ return trigger; }
	}

	return 0;
}


vl::KeyReleasedTrigger *
vl::EventManager::_findKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod )
{
	for( size_t i = 0; i < _key_released_triggers.size(); ++i )
	{
		vl::KeyReleasedTrigger *trigger = _key_released_triggers.at(i);
		// exact match
		if( trigger->getKey() == kc && trigger->getModifiers() == mod )
		{ return trigger; }
	}

	return 0;
}


void
vl::EventManager::_keyDown( OIS::KeyCode kc )
{
	std::vector<OIS::KeyCode>::iterator iter = std::find( _keys_down.begin(), _keys_down.end(), kc );
	if( iter == _keys_down.end() );
	{ _keys_down.push_back(kc); }
}

void 
vl::EventManager::_keyUp( OIS::KeyCode kc )
{
	std::vector<OIS::KeyCode>::iterator iter = std::find( _keys_down.begin(), _keys_down.end(), kc );
	if( iter != _keys_down.end() );
	{ _keys_down.erase(iter); }
}
