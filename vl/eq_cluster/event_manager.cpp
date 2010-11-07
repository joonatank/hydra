
#include "event_manager.hpp"

#include "transform_event.hpp"
#include "config.hpp"

#include "base/exceptions.hpp"

eqOgre::Event *
eqOgre::EventManager::createEvent(const std::string& type)
{
	if( type == "Event"  )
	{
		return new BasicEvent();
	}
	else if( type == "ToggleEvent" )
	{
		return new ToggleEvent();
	}
	else if( type == "TransformationEvent" )
	{
		return new TransformationEvent();
	}

	BOOST_THROW_EXCEPTION( vl::no_factory()
		<< vl::factory_name("Event factory")
		<< vl::object_type_name(type)
		);
}

// TODO these should not be hard coded
eqOgre::Operation *
eqOgre::EventManager::createOperation(const std::string& type)
{
	if( type == AddTransformOperation::TYPENAME )
	{
		return new AddTransformOperation();
	}
	else if( type == RemoveTransformOperation::TYPENAME )
	{
		return new RemoveTransformOperation();
	}
	else if( type == QuitOperation::TYPENAME )
	{
		return new QuitOperation();
	}
	else if( type == ReloadScene::TYPENAME )
	{
		return new ReloadScene();
	}

	BOOST_THROW_EXCEPTION( vl::no_factory()
		<< vl::factory_name("Operation factory")
		<< vl::object_type_name(type)
		);
}

eqOgre::Trigger *
eqOgre::EventManager::createTrigger(const std::string& type)
{
	if( type == "KeyTrigger" )
	{
		return new KeyTrigger();
	}
	else if( type == "FrameTrigger" )
	{
		return new FrameTrigger();
	}

	BOOST_THROW_EXCEPTION( vl::no_factory()
		<< vl::factory_name("Trigger factory")
		<< vl::object_type_name(type)
		);
}

eqOgre::KeyTrigger *eqOgre::EventManager::createKeyTrigger(void )
{
	return new KeyTrigger();
}

bool
eqOgre::EventManager::addEvent(eqOgre::Event *event)
{
	_events.push_back(event);
	std::cerr << "Event = " << event << " added. " << _events.size()
		<< " size." << std::endl;
	return true;
}

bool
eqOgre::EventManager::removeEvent(eqOgre::Event *event)
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
eqOgre::EventManager::hasEvent(eqOgre::Event *event)
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

void
eqOgre::EventManager::processEvents( Trigger *trig )
{
//	std::cout << "eqOgre::EventManager::processEvents" << std::endl;
	std::vector<Event *>::iterator iter;
	for( iter = _events.begin(); iter != _events.end(); ++iter )
	{
		(*iter)->processTrigger(trig);
	}
	
}

void eqOgre::EventManager::printEvents(std::ostream& os) const
{
	os << " Events, " << _events.size() << " of them." << std::endl << std::endl;
	std::vector<Event *>::const_iterator iter;
	for( iter = _events.begin(); iter != _events.end(); ++iter )
	{
		os << *(*iter) << std::endl;
	}
}
