/**	Joonatan Kuosa
 *	2010-12
 *
 *	Event Handling Trigger class
 */

#include "trigger.hpp"


/// ----------------- BasicActionTrigger -------------------
void
vl::BasicActionTrigger::addAction(vl::BasicActionPtr action)
{
	// TODO should check that no two actions are the same
	if( action )
	{
		_actions.push_back( action );
	}
}

vl::BasicActionPtr 
vl::BasicActionTrigger::getAction( size_t i )
{ return _actions.at(i); }

void vl::BasicActionTrigger::update( void )
{
	// FIXME this crashes
	for( size_t i = 0; i < _actions.size(); ++i )
	{
		_actions.at(i)->execute();
	}
}

/// ------------- TransformActionTrigger -----------------
vl::TransformActionTrigger::TransformActionTrigger( void )
	: _action(0)
{}

void vl::TransformActionTrigger::setAction(vl::TransformActionPtr action)
{
	if( _action != action )
	{
		_action = action;

		update(_value);
	}
}

void
vl::TransformActionTrigger::update(const vl::Transform& data)
{
	// Copy the data for futher reference
	_value = data;
	if( _action )
	{
		_action->execute(data);
	}
}




/// KeyTrigger Public
vl::KeyTrigger::KeyTrigger( void )
	: _key( OIS::KC_UNASSIGNED )
{}

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

std::string const &
vl::KeyPressedTrigger::getTypeName(void ) const
{ return vl::KeyPressedTriggerFactory::TYPENAME; }

std::string const vl::KeyPressedTriggerFactory::TYPENAME = "KeyPressedTrigger";

std::string const &
vl::KeyReleasedTrigger::getTypeName(void ) const
{ return vl::KeyReleasedTriggerFactory::TYPENAME; }

std::string const vl::KeyReleasedTriggerFactory::TYPENAME = "KeyReleasedTrigger";

std::string const &
vl::FrameTrigger::getTypeName(void ) const
{ return vl::FrameTriggerFactory::TYPENAME; }

std::string const vl::FrameTriggerFactory::TYPENAME = "FrameTrigger";
