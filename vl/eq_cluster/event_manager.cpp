
#include "event_manager.hpp"

#include "transform_event.hpp"
#include "config.hpp"

#include "base/exceptions.hpp"

vl::Event *
vl::EventManager::createEvent(const std::string& type)
{
	std::vector<EventFactory *>::iterator iter;
	for( iter = _event_factories.begin(); iter != _event_factories.end(); ++iter )
	{
		if( (*iter)->getTypeName() == type )
		{ return (*iter)->create(); }
	}

	BOOST_THROW_EXCEPTION( vl::no_factory()
		<< vl::factory_name("Event factory") << vl::object_type_name(type) );
}

vl::Trigger *
vl::EventManager::createTrigger(const std::string& type)
{
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
vl::EventManager::addEventFactory(vl::EventFactory* fact)
{
	std::vector<EventFactory *>::iterator iter;
	for( iter = _event_factories.begin(); iter != _event_factories.end(); ++iter )
	{
		if( (*iter)->getTypeName() == fact->getTypeName() )
		{
			BOOST_THROW_EXCEPTION( vl::duplicate_factory()
				<< vl::factory_name("Event factory")
				<< vl::object_type_name( fact->getTypeName() )
				);
		}
	}

	_event_factories.push_back( fact );
}

void
vl::EventManager::addTriggerFactory(vl::TriggerFactory* fact)
{
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

// TODO not implemented
vl::Event *
vl::EventManager::findEvent( const std::string &name )
{ return 0; }


bool
vl::EventManager::addEvent(vl::Event *event)
{
	_events.push_back(event);
	std::cerr << "Event = " << event << " added. " << _events.size()
		<< " size." << std::endl;
	return true;
}

bool
vl::EventManager::removeEvent(vl::Event *event)
{
	std::vector<Event *>::iterator iter;
	for( iter = _events.begin(); iter != _events.end(); ++iter )
	{
		if( *iter == event )
		{
			_events.erase(iter);
			return true;
		}
	}

	return false;
}

bool
vl::EventManager::hasEvent(vl::Event *event)
{
//	std::cerr << "eqOgre::EventManager::hasEvent" << std::endl;
	std::vector<Event *>::iterator iter;
	for( iter = _events.begin(); iter != _events.end(); ++iter )
	{
		if( event == *iter )
		{ return true; }
	}

	return false;
}

bool
vl::EventManager::processEvents( vl::Trigger *trig )
{
	bool retval = false;
//	std::cout << "eqOgre::EventManager::processEvents" << std::endl;
	std::vector<Event *>::iterator iter;
	for( iter = _events.begin(); iter != _events.end(); ++iter )
	{
		retval |= (*iter)->processTrigger(trig);
	}

	return retval;
}

void
vl::EventManager::printEvents(std::ostream& os) const
{
	os << " Events, " << _events.size() << " of them." << std::endl << std::endl;
	std::vector<Event *>::const_iterator iter;
	for( iter = _events.begin(); iter != _events.end(); ++iter )
	{
		os << *(*iter) << std::endl;
	}
}

vl::TrackerTrigger *
vl::EventManager::createTrackerTrigger( std::string const &name )
{
	// TODO this should check that no two triggers are equal
	vl::TrackerTrigger *trigger = new vl::TrackerTrigger;
	trigger->setName( name );
	_tracker_triggers.push_back( trigger );
	return trigger;
}

vl::TrackerTrigger *
vl::EventManager::getTrackerTrigger(const std::string& name)
{
	for( size_t i = 0; i < _tracker_triggers.size(); ++i )
	{
		vl::TrackerTrigger *trigger = _tracker_triggers.at(i);
		if( trigger->getName() == name )
		{ return trigger; }
	}

	return 0;
}

bool
vl::EventManager::hasTrackerTrigger(const std::string& name)
{
	for( size_t i = 0; i < _tracker_triggers.size(); ++i )
	{
		vl::TrackerTrigger *trigger = _tracker_triggers.at(i);
		if( trigger->getName() == name )
		{ return true; }
	}

	return false;
}
