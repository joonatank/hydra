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

	/// Key triggers
	/// @brief creates a new Key trigger or returns an already created one
	/// Preferred method for doing event mapping, this one will always return
	/// a working KeyTrigger.
	vl::KeyTrigger *createKeyTrigger(OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE);

	/// @brief get an already created key trigger
	/// @return key trigger
	/// @throw if no such trigger exists
	vl::KeyTrigger *getKeyTrigger(OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE);

	bool hasKeyTrigger(OIS::KeyCode kc, KEY_MOD mod = KEY_MOD_NONE);

	/// Update methods for key triggers should only be called from the Input event receiver
	void keyPressed(OIS::KeyCode kc);
	void keyReleased(OIS::KeyCode kc);

	/// Frame trigger
	vl::FrameTrigger *getFrameTrigger( void );

private :
	vl::TrackerTrigger *_findTrackerTrigger( std::string const &name );

	vl::KeyTrigger *_find_key_trigger(OIS::KeyCode kc, std::bitset<8> mod);

	vl::KeyTrigger *_find_best_match(OIS::KeyCode kc, std::bitset<8> mod);

	bool _keyDown( OIS::KeyCode kc );

	bool _keyUp( OIS::KeyCode kc );

	void _update_key_modifers(std::bitset<8> new_mod);

/// Data
private :
	std::vector<vl::TrackerTrigger *> _tracker_triggers;
	std::vector<vl::KeyTrigger *> _key_triggers;

	vl::FrameTrigger *_frame_trigger;

	std::bitset<8> _key_modifiers;

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

