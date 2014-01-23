/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file event_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
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

#include "input/tracker.hpp"

#include "input/joystick_event.hpp"

#include "input/mouse_event.hpp"


namespace vl
{

class HYDRA_API EventManager
{
public :
	/// @brief Constructor
	/// @param res_man ResourceManager used for FileLoading
	EventManager(ResourceManager *res_man);

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
	
	void updateGameJoystick(vl::JoystickEvent const&, int index);
	//void updateJoystickButton(vl::JoystickEvent const &evt, int index);
	//void updateJoystickAxis(vl::JoystickEvent const &evt, int index);
	//void updateJoystickVector(vl::JoystickEvent const &evt, int index);
	//void updateGameJoystick(vl::JoystickEvent const &evt);
	
	/// Frame trigger
	vl::FrameTrigger *getFrameTrigger( void );

	vl::TimeTrigger *createTimeTrigger(void);

	void destroyTimeTrigger(vl::TimeTrigger *trigger);

	PCANRefPtr getPCAN(void);

	vl::ClientsRefPtr getTrackerClients(void)
	{ return _trackers; }

	/// @brief create an vrpn analog object or retrieve an already created
	vrpn_analog_client_ref_ptr createAnalogClient(std::string const &name);

	/// @brief called from GameManager to update input devices
	void mainloop(vl::time const &elapsed_time);

	/// @brief remove all triggers
	void removeTriggers(void);


	/// File Loaders

	/// @brief Load tracking files
	void loadTrackingFiles(std::vector<std::string> const &files);

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
	// @warning: added due mouse support and raycast picker testing:
	std::vector<vl::MouseTrigger *> _mouse_triggers;
	std::vector< vl::JoystickTrigger* > _joystick_triggers;

	vl::FrameTrigger *_frame_trigger;

	std::bitset<8> _key_modifiers;

	std::vector<OIS::KeyCode> _keys_down;

	std::vector<TimeTrigger *> _time_triggers;

	PCANRefPtr _pcan;


	/// Tracking
	vl::ClientsRefPtr _trackers;
	/// name client map
	std::map<std::string, vrpn_analog_client_ref_ptr> _analog_clients;
	
	/// Resources
	ResourceManager *_resource_manager;

};	// class EventManager

inline std::ostream &
operator<<( std::ostream &os, EventManager const &man )
{
	os << "Event Manager " << std::endl;
	return os;
}

}	// namespace vl

#endif	// HYDRA_EVENT_MANAGER_HPP
