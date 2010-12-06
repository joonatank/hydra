/**	Joonatan Kuosa
 *	2010-11
 *
 */
#include "event.hpp"

#include "base/exceptions.hpp"


/// Event Public
vl::Event::Event( void )
	: _action(0)
{}

bool
vl::Event::processTrigger(vl::Trigger* trig)
{
	if( !_action )
	{
		std::cerr << "No operation" << std::endl;
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	std::vector<Trigger *>::iterator iter = _triggers.begin();
	for( ; iter != _triggers.end(); ++iter )
	{
		if( (*iter)->isSimilar(trig) )
		{ break; }
	}

	if( iter != _triggers.end() )
	{
		_action->execute();
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

	return os;
}


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
