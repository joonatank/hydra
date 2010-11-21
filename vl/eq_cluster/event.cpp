/**	Joonatan Kuosa
 *	2010-11
 *
 */
#include "event.hpp"

#include "base/exceptions.hpp"

/// KeyTrigger Public
vl::KeyTrigger::KeyTrigger( void )
	: _key( OIS::KC_UNASSIGNED )
{}

bool
vl::KeyTrigger::isEqual(const vl::Trigger& other) const
{
	KeyTrigger const &key_other = static_cast<KeyTrigger const &>( other );
	if( key_other._key == _key )
	{ return true; }

	return false;
}

bool
vl::KeyTrigger::isSpecialisation(const vl::Trigger* other) const
{
	KeyTrigger const *a = dynamic_cast<KeyTrigger const *>( other );
	if( a )
	{ return isEqual(*a); }
	else
	{ return false; }
}

std::ostream &
vl::KeyTrigger::print(std::ostream& os) const
{
	Trigger::print(os);
	os << " KeyCode = " << vl::getKeyName(_key) << " : released = ";

	return os;
}


std::string const &
vl::KeyTrigger::getTypeName(void ) const
{ return vl::KeyTriggerFactory::TYPENAME; }

std::string const vl::KeyTriggerFactory::TYPENAME = "KeyTrigger";

bool
vl::KeyPressedTrigger::isSpecialisation(const vl::Trigger* other) const
{
	KeyPressedTrigger const *a = dynamic_cast<KeyPressedTrigger const *>( other );
	if( a )
	{ return isEqual(*a); }
	else
	{ return false; }
}

std::string const &
vl::KeyPressedTrigger::getTypeName(void ) const
{ return vl::KeyPressedTriggerFactory::TYPENAME; }

std::string const vl::KeyPressedTriggerFactory::TYPENAME = "KeyPressedTrigger";

bool
vl::KeyReleasedTrigger::isSpecialisation(const vl::Trigger* other) const
{
	KeyReleasedTrigger const *a = dynamic_cast<KeyReleasedTrigger const *>( other );
	if( a )
	{ return isEqual(*a); }
	else
	{ return false; }
}

std::string const &
vl::KeyReleasedTrigger::getTypeName(void ) const
{ return vl::KeyReleasedTriggerFactory::TYPENAME; }

std::string const vl::KeyReleasedTriggerFactory::TYPENAME = "KeyReleasedTrigger";

std::string const &
vl::FrameTrigger::getTypeName(void ) const
{ return vl::FrameTriggerFactory::TYPENAME; }

std::string const vl::FrameTriggerFactory::TYPENAME = "FrameTrigger";

/// Event Public
// FIXME after creating the program the Event can not be used before waiting for time limit s
// because ::clock is zero when the program is created so we need to wait for
// time_limit seconds before using the event
// NOTE not a really high priority but still annoying
vl::Event::Event( void )
	: _action(0), _last_time(0), _time_limit(0)
{}

bool
vl::Event::processTrigger(vl::Trigger* trig)
{
	if( !_action )
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
			_action->execute();
			_last_time = time;
		}

		return true;
	}

	return false;
}

bool
vl::Event::removeTrigger(vl::Trigger* trig)
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
vl::Event::addTrigger(vl::Trigger* trig)
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
vl::Event::setAction(vl::BasicActionPtr action)
{ _action = action; }

std::ostream &
vl::Event::print(std::ostream& os) const
{
	// TODO add identifier
	os << "Event : type = " << getTypeName() << " " << std::endl;
	os << "Operation = " << *_action;
	os << _triggers.size() << " Triggers : they are = " << std::endl;
	for( size_t i = 0; i < _triggers.size(); ++i )
	{
		os << *(_triggers.at(i)) << std::endl;
	}
	os << "Time limit = " << _time_limit << std::endl;

	return os;
}

std::string const &
vl::BasicEvent::getTypeName(void ) const
{ return vl::BasicEventFactory::TYPENAME; }


std::string const vl::BasicEventFactory::TYPENAME = "Event";


/// Event protected
std::vector< vl::Trigger * >::iterator
vl::Event::_findTrigger(vl::Trigger* trig)
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
vl::ToggleEvent::ToggleEvent( void )
	: vl::Event(), _state(false), _toggleOn(0), _toggleOff(0)
{}

bool
vl::ToggleEvent::processTrigger(vl::Trigger* trig)
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
vl::ToggleEvent::getTypeName(void ) const
{ return vl::ToggleEventFactory::TYPENAME; }


std::string const vl::ToggleEventFactory::TYPENAME = "ToggleEvent";

/// ToggleEvent private
void
vl::ToggleEvent::toggleOn(void )
{
	if( _toggleOn )
		_toggleOn->execute();
}

void
vl::ToggleEvent::toggleOff(void )
{
	if( _toggleOff )
		_toggleOff->execute();
}
