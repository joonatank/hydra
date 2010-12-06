
#include "trigger.hpp"

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
