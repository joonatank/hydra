
#include "event.hpp"

#include "base/exceptions.hpp"

/// KeyTrigger Public
eqOgre::KeyTrigger::KeyTrigger( void )
	: _key( OIS::KC_UNASSIGNED )
{}

bool
eqOgre::KeyTrigger::isEqual(const eqOgre::Trigger& other) const
{
	KeyTrigger const &key_other = static_cast<KeyTrigger const &>( other );
	if( key_other._key == _key )
	{ return true; }

	return false;
}

bool
eqOgre::KeyTrigger::isSpecialisation(const eqOgre::Trigger* other) const
{
	KeyTrigger const *a = dynamic_cast<KeyTrigger const *>( other );
	if( a )
	{ return isEqual(*a); }
	else
	{ return false; }
}

std::ostream &
eqOgre::KeyTrigger::print(std::ostream& os) const
{
	Trigger::print(os);
	os << " KeyCode = " << getKeyName(_key) << " : released = ";

	return os;
}


std::string const &
eqOgre::KeyTrigger::getTypeName(void ) const
{ return eqOgre::KeyTriggerFactory::TYPENAME; }

std::string const eqOgre::KeyTriggerFactory::TYPENAME = "KeyTrigger";

bool
eqOgre::KeyPressedTrigger::isSpecialisation(const eqOgre::Trigger* other) const
{
	KeyPressedTrigger const *a = dynamic_cast<KeyPressedTrigger const *>( other );
	if( a )
	{ return isEqual(*a); }
	else
	{ return false; }
}

std::string const &
eqOgre::KeyPressedTrigger::getTypeName(void ) const
{ return eqOgre::KeyPressedTriggerFactory::TYPENAME; }

std::string const eqOgre::KeyPressedTriggerFactory::TYPENAME = "KeyPressedTrigger";

bool
eqOgre::KeyReleasedTrigger::isSpecialisation(const eqOgre::Trigger* other) const
{
	KeyReleasedTrigger const *a = dynamic_cast<KeyReleasedTrigger const *>( other );
	if( a )
	{ return isEqual(*a); }
	else
	{ return false; }
}

std::string const &
eqOgre::KeyReleasedTrigger::getTypeName(void ) const
{ return eqOgre::KeyReleasedTriggerFactory::TYPENAME; }

std::string const eqOgre::KeyReleasedTriggerFactory::TYPENAME = "KeyReleasedTrigger";

std::string const &
eqOgre::FrameTrigger::getTypeName(void ) const
{ return eqOgre::FrameTriggerFactory::TYPENAME; }

std::string const eqOgre::FrameTriggerFactory::TYPENAME = "FrameTrigger";

/// Event Public
// FIXME after creating the program the Event can not be used before waiting for time limit s
// because ::clock is zero when the program is created so we need to wait for
// time_limit seconds before using the event
// NOTE not a really high priority but still annoying
eqOgre::Event::Event( void )
	: _operation(0), _last_time(0), _time_limit(0)
{}

bool
eqOgre::Event::processTrigger(eqOgre::Trigger* trig)
{
	if( !_operation )
	{
		std::cerr << "No operation" << std::endl;
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	clock_t time = ::clock();

	std::vector<Trigger *>::iterator iter = _triggers.begin();
	for( ; iter != _triggers.end(); ++iter )
	{
		if( (*iter)->isSimilar(trig) )
		{ break; }
	}
	
	if( iter != _triggers.end() )
	{
//		std::cerr << "Trigger found : trigger = " << trig
//			<< " : operation = " << _operation << "." << std::endl;

		// We need to wait _time_limit secs before issuing the command again
		if( ( (double)(time - _last_time) )/CLOCKS_PER_SEC > _time_limit )
		{
			_operation->execute();
			_last_time = time;
		}

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
		std::cerr << "Trigger added : trigger = " << trig << std::endl;
		_triggers.push_back( trig );
		return true;
	}

	return false;
}
void
eqOgre::Event::setOperation(eqOgre::Operation* oper)
{ _operation = oper; }

std::ostream &
eqOgre::Event::print(std::ostream& os) const
{
	// TODO add identifier
	os << "Event : type = " << getTypeName() << " " << std::endl;
	os << "Operation = " << *_operation;// << std::endl;
	os << _triggers.size() << " Triggers : they are = " << std::endl;
	for( size_t i = 0; i < _triggers.size(); ++i )
	{
		os << *(_triggers.at(i)) << std::endl;
	}
	os << "Time limit = " << _time_limit << std::endl;

	return os;
}

std::string const &
eqOgre::BasicEvent::getTypeName(void ) const
{ return eqOgre::BasicEventFactory::TYPENAME; }


std::string const eqOgre::BasicEventFactory::TYPENAME = "Event";


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
eqOgre::ToggleEvent::ToggleEvent( void )
	: eqOgre::Event(), _state(false), _toggleOn(0), _toggleOff(0)
{}

bool
eqOgre::ToggleEvent::processTrigger(eqOgre::Trigger* trig)
{
	std::vector<Trigger *>::iterator iter = _triggers.begin();
	for( ; iter != _triggers.end(); ++iter )
	{
		if( (*iter)->isSimilar(trig) )
		{ break; }
	}

	clock_t time = ::clock();
	if( iter != _triggers.end() )
	{
		if( ( (double)(time - _last_time) )/CLOCKS_PER_SEC > _time_limit )
		{
			if( _state )
			{
				toggleOff();
				_state = false;
			}
			else
			{
				toggleOn();
				_state = true;
			}
			_last_time = time;
		}
		return true;
	}
	return false;
}

std::string const &
eqOgre::ToggleEvent::getTypeName(void ) const
{ return eqOgre::ToggleEventFactory::TYPENAME; }


std::string const eqOgre::ToggleEventFactory::TYPENAME = "ToggleEvent";

/// ToggleEvent private
void
eqOgre::ToggleEvent::toggleOn(void )
{
	if( _toggleOn )
		_toggleOn->execute();
}

void
eqOgre::ToggleEvent::toggleOff(void )
{
	if( _toggleOff )
		_toggleOff->execute();
}
