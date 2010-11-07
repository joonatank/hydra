
#include "event.hpp"

#include "base/exceptions.hpp"

/// KeyTrigger Public
eqOgre::KeyTrigger::KeyTrigger( void )
	: _key( OIS::KC_UNASSIGNED ), _released(false)
{}

bool
eqOgre::KeyTrigger::isEqual(const eqOgre::Trigger& other) const
{
	KeyTrigger const &key_other = static_cast<KeyTrigger const &>( other );
	if( key_other._key == _key && key_other._released == _released )
	{ return true; }

	return false;
}

std::string const eqOgre::KeyTrigger::TYPENAME = "KeyTrigger";

std::string const eqOgre::FrameTrigger::TYPENAME = "FrameTrigger";

/// Event Public
eqOgre::Event::Event( eqOgre::Operation* oper,
					  eqOgre::Trigger* trig,
					  double time_limit )
	: _operation(oper), _last_time( ::clock() ), _time_limit(time_limit)
{
	std::cerr << "Event created" << std::endl;

	if( trig )
	{ _triggers.push_back(trig); }
}

bool
eqOgre::Event::processTrigger(eqOgre::Trigger* trig)
{
	if( !_operation )
	{
		std::cerr << "No operation" << std::endl;
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	clock_t time = ::clock();

	if( _findTrigger(trig) != _triggers.end() )
	{
		std::cerr << "Trigger found : trigger = " << trig
			<< " : operation = " << _operation << "." << std::endl;

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


std::string const eqOgre::BasicEvent::TYPENAME = "BasicEvent";


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

std::string const eqOgre::ToggleEvent::TYPENAME = "ToggleEvent";

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
