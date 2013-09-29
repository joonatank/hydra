/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file event_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_EVENT_MANAGER_HPP
#define HYDRA_EVENT_MANAGER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include <vector>

#include "trigger.hpp"

#include "tracker.hpp"

#include "input/serial_joystick_event.hpp"

#include "input/joystick_event.hpp"

#include "input/mouse_event.hpp"


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

class HYDRA_API EventManager
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

	/// mouse related:
	// What's this? 
	vl::MouseTrigger *createMouseTrigger(void);
	void destroyMouseTrigger(vl::MouseTrigger *trigger);

	vl::JoystickTrigger *createJoystickTrigger(void);
	void destroyJoystickTrigger(vl::JoystickTrigger *trigger);
	
	void mouseMoved(vl::MouseEvent const &evt);
	void mousePressed(vl::MouseEvent const &evt, vl::MouseEvent::BUTTON b_id);
	void mouseReleased(vl::MouseEvent const &evt, vl::MouseEvent::BUTTON b_id);
	
	void updateGameJoystick(vl::JoystickEvent const&, vl::JoystickEvent::EventType type, int index);
	//void updateJoystickButton(vl::JoystickEvent const &evt, int index);
	//void updateJoystickAxis(vl::JoystickEvent const &evt, int index);
	//void updateJoystickVector(vl::JoystickEvent const &evt, int index);
	//void updateGameJoystick(vl::JoystickEvent const &evt);
	
	/// Frame trigger
	vl::FrameTrigger *getFrameTrigger( void );

	vl::TimeTrigger *createTimeTrigger(void);

	void destroyTimeTrigger(vl::TimeTrigger *trigger);

	/// @todo add params to get specifc joystick and also allow for fallbacks
	/// @param name only valid values are COM ports for now
	/// @return valid pointer if such joystick exists, NULL pointer if not
	// only com ports are supported by name
	// the full name is comX:N
	// where X is the serial port number (in Windows)
	// and N is the joystick number in that serial port
	// comX is a short hand for comX:0
	JoystickRefPtr getJoystick(std::string const &name = "default");

	void update_joystick(vl::SerialJoystickEvent const &evt);

	/// @brief called from GameManager to update input devices
	void mainloop(vl::time const &elapsed_time);

	/// @brief remove all triggers
	void removeAll(void);

private :
	vl::TrackerTrigger *_findTrackerTrigger( std::string const &name );

	vl::KeyTrigger *_find_key_trigger(OIS::KeyCode kc, std::bitset<8> mod);

	vl::KeyTrigger *_find_best_match(OIS::KeyCode kc, std::bitset<8> mod);

	bool _keyDown( OIS::KeyCode kc );

	bool _keyUp( OIS::KeyCode kc );

	void _update_key_modifers(std::bitset<8> new_mod);

	JoystickRefPtr _getSerialJoystick(std::string const &name);

	JoystickRefPtr _getGameJoystick(std::string const &name);

/// Data
private :
	std::vector<vl::TrackerTrigger *> _tracker_triggers;
	std::vector<vl::KeyTrigger *> _key_triggers;
	// @warning: added due mouse support and raycast picker testing:
	std::vector<vl::MouseTrigger *> _mouse_triggers;
	std::vector< vl::JoystickTrigger* > _joystick_triggers;

	vl::FrameTrigger *_frame_trigger;

	std::bitset<8> _key_modifiers;

	std::vector<OIS::KeyCode> _keys_down;

	std::map<std::string, JoystickRefPtr> _joysticks;

	std::map<std::string, SerialJoystickRefPtr> _serial_joysticks;

	std::vector<TimeTrigger *> _time_triggers;

	// stored also separately to avoid searching and casting
	JoystickRefPtr _game_joystick;

	
};	// class EventManager

inline std::ostream &
operator<<( std::ostream &os, EventManager const &man )
{
	os << "Event Manager " << std::endl;
	return os;
}

}	// namespace vl

#endif	// HYDRA_EVENT_MANAGER_HPP
