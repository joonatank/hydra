/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 *	EventManager used to create and retrieve Triggers
 */
#ifndef VL_EVENT_MANAGER_HPP
#define VL_EVENT_MANAGER_HPP

#include <vector>

#include "trigger.hpp"

#include "tracker.hpp"

namespace vl
{

inline vl::KEY_MOD getModifier( OIS::KeyCode kc )
{
	// Shift
	if( kc == OIS::KC_LSHIFT || kc == OIS::KC_RSHIFT )
	{ return vl::KEY_MOD_SHIFT; }
	// Alt
	else if( kc == OIS::KC_LMENU || kc == OIS::KC_RMENU )
	{ return vl::KEY_MOD_META; }
	// Control
	else if( kc == OIS::KC_RCONTROL || kc == OIS::KC_LCONTROL )
	{ return vl::KEY_MOD_CTRL; }
	// Windows
	else if( kc == OIS::KC_LWIN || kc == OIS::KC_RWIN )
	{ return vl::KEY_MOD_SUPER;	}

	return vl::KEY_MOD_NONE;
}

class EventManager
{
public :
	EventManager( void );

	~EventManager( void );

	// FIXME for now the trigger factory interface is broken
	// use the hard coded functions for creating different types of triggers.
	Trigger *createTrigger( std::string const &type );

	/// Parameter : Pointer to the factory which user wants to add
	/// 	The factory is expected to be alive till the factory is removed
	/// If a factory with that object typeName is not found the factory
	/// is added to the list of factories.
	/// If a factory with the same object typeName is found this will throw

	// FIXME for now the trigger factory interface is broken
	// use the hard coded functions for creating different types of triggers.
	void addTriggerFactory( TriggerFactory *fact );

	/// Tracker Triggers
	/// Create a tracker trigger, this will never fail
	/// If a tracker trigger with the same name exists it will return that one
	/// If not then a new one is created
	/// Returns : pointer to created tracker trigger
	vl::TrackerTrigger *createTrackerTrigger( std::string const &name );

	/// Get a tracker trigger by name
	///
	/// Returns : valid pointer to a tracker trigger
	///
	/// Throws on errors
	/// Errors : no such tracker trigger exists
	vl::TrackerTrigger *getTrackerTrigger( std::string const &name );

	/// If a tracker trigger by the name exists
	///
	/// Returns true if there is one, false otherwise
	bool hasTrackerTrigger( std::string const &name );

	vl::KeyPressedTrigger *createKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE );

	vl::KeyPressedTrigger *getKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE );

	bool hasKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE );

	void updateKeyPressedTrigger( OIS::KeyCode kc );

	vl::KeyReleasedTrigger *createKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE );

	vl::KeyReleasedTrigger *getKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE );

	void updateKeyReleasedTrigger( OIS::KeyCode kc );

	bool hasKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE );

	vl::FrameTrigger *getFrameTrigger( void );

private :
	vl::TrackerTrigger *_findTrackerTrigger( std::string const &name );

	vl::KeyPressedTrigger *_findKeyPressedTrigger( OIS::KeyCode kc, KEY_MOD mod );

	vl::KeyReleasedTrigger *_findKeyReleasedTrigger( OIS::KeyCode kc, KEY_MOD mod );

	void _keyDown( OIS::KeyCode kc );

	void _keyUp( OIS::KeyCode kc );

/// Data
private :
	std::vector<TriggerFactory *> _trigger_factories;
	std::vector<vl::TrackerTrigger *> _tracker_triggers;
	std::vector<vl::KeyPressedTrigger *> _key_pressed_triggers;
	std::vector<vl::KeyReleasedTrigger *> _key_released_triggers;

	vl::FrameTrigger *_frame_trigger;
	
	KEY_MOD _key_modifiers;

	std::vector<OIS::KeyCode> _keys_down;

};	// class EventManager

inline std::ostream &
operator<<( std::ostream &os, EventManager const &man )
{
	os << "Event Manager " << std::endl;
	return os;
}

}	// namespace vl

#endif	// VL_EVENT_MANAGER_HPP

