/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */
#include "event_manager.hpp"

#include "base/exceptions.hpp"

vl::EventManager::EventManager( void )
	: _frame_trigger(0)
{}

vl::EventManager::~EventManager( void )
{
	delete _frame_trigger;

	for( std::vector<vl::TriggerFactory *>::iterator iter = _trigger_factories.begin();
		iter != _trigger_factories.end(); ++iter )
	{
		delete *iter;
	}

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

vl::Trigger *
vl::EventManager::createTrigger(const std::string& type)
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );

	std::vector<TriggerFactory *>::iterator iter;
	for( iter = _trigger_factories.begin(); iter != _trigger_factories.end(); ++iter )
	{
		if( (*iter)->getTypeName() == type )
		{ return (*iter)->create(); }
	}

	BOOST_THROW_EXCEPTION( vl::no_factory()
		<< vl::factory_name("Trigger factory") << vl::object_type_name(type) );
}


void
vl::EventManager::addTriggerFactory(vl::TriggerFactory* fact)
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );

	std::vector<TriggerFactory *>::iterator iter;
	for( iter = _trigger_factories.begin(); iter != _trigger_factories.end(); ++iter )
	{
		if( (*iter)->getTypeName() == fact->getTypeName() )
		{
			BOOST_THROW_EXCEPTION( vl::duplicate_factory()
				<< vl::factory_name("Trigger factory")
				<< vl::object_type_name( fact->getTypeName() )
				);
		}
	}

	_trigger_factories.push_back( fact );
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
vl::EventManager::createKeyPressedTrigger(OIS::KeyCode kc)
{
	vl::KeyPressedTrigger *trigger = _findKeyPressedTrigger(kc);
	if( !trigger )
	{
		trigger = new vl::KeyPressedTrigger;
		trigger->setKey(kc);
		_key_pressed_triggers.push_back(trigger);
	}

	return trigger;
}

vl::KeyPressedTrigger *
vl::EventManager::getKeyPressedTrigger(OIS::KeyCode kc)
{
	vl::KeyPressedTrigger *trigger = _findKeyPressedTrigger(kc);
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasKeyPressedTrigger(OIS::KeyCode kc)
{
	return _findKeyPressedTrigger(kc);
}




vl::KeyReleasedTrigger *
vl::EventManager::createKeyReleasedTrigger(OIS::KeyCode kc)
{
	vl::KeyReleasedTrigger *trigger = _findKeyReleasedTrigger(kc);
	if( !trigger )
	{
		trigger = new vl::KeyReleasedTrigger;
		trigger->setKey(kc);
		_key_released_triggers.push_back(trigger);
	}

	return trigger;
}

vl::KeyReleasedTrigger *
vl::EventManager::getKeyReleasedTrigger(OIS::KeyCode kc)
{
	vl::KeyReleasedTrigger *trigger = _findKeyReleasedTrigger(kc);
	if( trigger )
	{ return trigger; }

	BOOST_THROW_EXCEPTION( vl::null_pointer() );
}

bool
vl::EventManager::hasKeyReleasedTrigger(OIS::KeyCode kc)
{
	return _findKeyReleasedTrigger(kc);
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
vl::EventManager::_findKeyPressedTrigger(OIS::KeyCode kc)
{
	for( size_t i = 0; i < _key_pressed_triggers.size(); ++i )
	{
		vl::KeyPressedTrigger *trigger = _key_pressed_triggers.at(i);
		if( trigger->getKey() == kc )
		{ return trigger; }
	}

	return 0;
}


vl::KeyReleasedTrigger *
vl::EventManager::_findKeyReleasedTrigger(OIS::KeyCode kc)
{
	for( size_t i = 0; i < _key_released_triggers.size(); ++i )
	{
		vl::KeyReleasedTrigger *trigger = _key_released_triggers.at(i);
		if( trigger->getKey() == kc )
		{ return trigger; }
	}

	return 0;
}
