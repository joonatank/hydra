/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file python/python_events.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

// Interface
#include "python_module.hpp"

// Callback helpers
#include <toast/python/callback.hpp>

// For namespace renaming
#include "python_context_impl.hpp"

// Event handling
#include "event_manager.hpp"
#include "trigger.hpp"

// Necessary for transforming OIS keycodes to python
#include "input/keycode.hpp"

// Input
#include "input/pcan.hpp"
//#include "input/mouse_event.hpp"

#include "input/vrpn_analog_client.hpp"

// Necessary for exposing vectors
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


/// Event system overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createKeyTrigger_ov, createKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getKeyTrigger_ov, getKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hasKeyTrigger_ov, hasKeyTrigger, 1, 2)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(set_axis_constraint_ovs, set_axis_constraint, 2, 4)


using namespace vl;

namespace
{

void export_managers(void)
{
	python::enum_<KEY_MOD>("KEY_MOD")
		.value("NONE", KEY_MOD_NONE)
		.value("META", KEY_MOD_META)
		.value("CTRL", KEY_MOD_CTRL)
		.value("SHIFT", KEY_MOD_SHIFT)
		.value("SUPER", KEY_MOD_SUPER)
	;

	python::class_<vl::EventManager, boost::noncopyable>("EventManager", python::no_init)
		.def("getTrackerTrigger", &vl::EventManager::getTrackerTrigger, 
			python::return_value_policy<python::reference_existing_object>() )
		.def("hasTrackerTrigger", &vl::EventManager::hasTrackerTrigger )
		.def("createTrackerTrigger", &vl::EventManager::createTrackerTrigger, 
			python::return_value_policy<python::reference_existing_object>() )
		.def("getKeyTrigger", &vl::EventManager::getKeyTrigger,
			 getKeyTrigger_ov()[python::return_value_policy<python::reference_existing_object>()] )
		.def("createKeyTrigger", &vl::EventManager::createKeyTrigger,
			 createKeyTrigger_ov()[python::return_value_policy<python::reference_existing_object>()] )
		.def("hasKeyTrigger", &vl::EventManager::hasKeyTrigger, hasKeyTrigger_ov() )
		.def("getFrameTrigger", &vl::EventManager::getFrameTrigger, 
			python::return_value_policy<python::reference_existing_object>() )
		.add_property("frame_trigger", python::make_function(&vl::EventManager::getFrameTrigger, 
			python::return_value_policy<python::reference_existing_object>()) )
		.def("get_pcan", &vl::EventManager::getPCAN)
		.def("createTimeTrigger", &vl::EventManager::createTimeTrigger,
			python::return_value_policy<python::reference_existing_object>())
		.def("createMouseTrigger", &vl::EventManager::createMouseTrigger,
			python::return_value_policy<python::reference_existing_object>())
		.def("createJoystickTrigger", &vl::EventManager::createJoystickTrigger,
			python::return_value_policy<python::reference_existing_object>())
		.add_property("tracker_clients", &vl::EventManager::getTrackerClients)
		.def("create_analog_client", &vl::EventManager::createAnalogClient)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	python::class_<vl::TrackerSensor, boost::noncopyable>("TrackerSensor", python::no_init)
		.add_property("trigger", python::make_function(&vl::TrackerSensor::getTrigger, python::return_value_policy<python::reference_existing_object>() ), &vl::TrackerSensor::setTrigger)
		.add_property("transform", python::make_function(&vl::TrackerSensor::getCurrentTransform, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("default_transform", python::make_function(&vl::TrackerSensor::getDefaultTransform, python::return_value_policy<python::copy_const_reference>() ), &vl::TrackerSensor::setDefaultTransform)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	/// @todo replace with a list
	TrackerSensor & (vl::Tracker::*trackerGetSensor_ov0)(size_t) = &vl::Tracker::getSensor;
	
	python::class_<vl::Tracker, TrackerRefPtr, boost::noncopyable>("Tracker", python::no_init)
		.def("setSensor", &vl::Tracker::setSensor)
		.def("addSensor", &vl::Tracker::addSensor)
		.def("getSensor", python::make_function(trackerGetSensor_ov0, python::return_value_policy<python::reference_existing_object>()) )
		.add_property("n_sensors", &vl::Tracker::getNSensors, &vl::Tracker::setNSensors)
		.add_property("name", python::make_function(&vl::Tracker::getName, python::return_value_policy<python::copy_const_reference>()) )
		.def(python::self_ns::str(python::self_ns::self))
		.def("create", &vl::Tracker::create)
		.staticmethod("create")
	;

	python::class_<vl::Clients, ClientsRefPtr, boost::noncopyable>("Clients", python::no_init)
		.def("addTracker", &vl::Clients::addTracker)
		.def("getTracker", &vl::Clients::getTrackerPtr)
		.add_property("n_trackers", &vl::Clients::getNTrackers)
		.add_property("event_manager", python::make_function(&vl::Clients::getEventManager, python::return_value_policy<python::reference_existing_object>()))
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::analog_sensor, vl::analog_sensor_ref_ptr, boost::noncopyable>("analog_sensor", python::no_init)
		.def("addListener", toast::python::signal_connect<void (vl::scalar)>(&vl::analog_sensor::addListener))
		.def(python::self_ns::str(python::self_ns::self))
	;

	
	python::class_<vl::vrpn_analog_client, vrpn_analog_client_ref_ptr, boost::noncopyable>("vrpn_analog_client", python::no_init)
		/// @todo replace with a list
		.add_property("n_sensors", &vl::vrpn_analog_client::getNSensors, &vl::vrpn_analog_client::setNSensors)
		.def("get_sensor", &vl::vrpn_analog_client::getSensor)
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// Input

	python::class_<std::vector<int8_t> >("bytevec")
		.def(python::vector_indexing_suite< std::vector<int8_t> >())
	;

	// Can't expose these with toast for some reason
	python::class_<std::vector<vl::scalar> >("floatvec")
		.def(python::vector_indexing_suite< std::vector<vl::scalar> >())
	;

	python::class_<vl::CANMsg>("CanMsg", python::init<>())
		.def_readonly("id", &vl::CANMsg::id)
		.def_readonly("length", &vl::CANMsg::length)
		.def_readonly("data", &vl::CANMsg::data)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	python::enum_<vl::JoystickEvent::EventType>("JOYSTICK_EVENT_TYPE")
		.value("BUTTON_PRESSED", vl::JoystickEvent::BUTTON_PRESSED)
		.value("BUTTON_RELEASED", vl::JoystickEvent::BUTTON_RELEASED)
		.value("AXIS", vl::JoystickEvent::AXIS)
		.value("VECTOR", vl::JoystickEvent::VECTOR)
		.value("POV", vl::JoystickEvent::POV)
		.value("SLIDER", vl::JoystickEvent::SLIDER)
	;

	python::class_<vl::JoystickEvent>("JoystickEvent", python::init<>())
		.def_readonly("type", &vl::JoystickEvent::type)
		.def_readonly("info", &vl::JoystickEvent::info)
		.def_readonly("state", &vl::JoystickEvent::state)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	python::class_<vl::JoystickInfo>("JoystickInfo", python::init<>())
		//.def_readonly("dev_id", &vl::JoystickInfo::dev_id)
		//.def_readonly("input_manager_id", &vl::JoystickInfo::input_manager_id)
		//.def_readonly("vendor", &vl::JoystickInfo::vendor_id)
		//.def("vendor_id", &vl::JoystickInfo::getVendorID, python::return_value_policy<python::return_by_value>())
		//.def("owner_id", &vl::JoystickInfo::getOwnerID, python::return_value_policy<python::return_by_value>())
		//.def("device_id", &vl::JoystickInfo::getDevID, python::return_value_policy<python::return_by_value>())
		.add_property("device_id", &vl::JoystickInfo::dev_id)
		.add_property("owner_id", &vl::JoystickInfo::input_manager_id)
		.add_property("vendor_id", &vl::JoystickInfo::vendor_id)
		;

	
	python::class_<vl::JoystickState>("JoystickState", python::init<>())
		// can't expose buttons without exposing std::bitset
		//.def_readonly("buttons", &vl::JoystickState::buttons)
		.def_readonly("axes", &vl::JoystickState::axes)
		.def_readonly("vectors", &vl::JoystickState::vectors)
		.def("is_button_down", &vl::JoystickState::isButtonDown)
		.add_property("no_buttons_down", &vl::JoystickState::isNoButtonsDown)
		.add_property("any_button_down", &vl::JoystickState::isAnyButtonDown)
		.add_property("all_buttons_down", &vl::JoystickState::isAllButtonsDown)

		.def(python::self_ns::str(python::self_ns::self))
	;
	
	python::class_<vl::MouseEvent>("MouseEvent", python::init<>())
		.def_readonly("axis_x", &vl::MouseEvent::X)
		.def_readonly("axis_y", &vl::MouseEvent::Y)
		.def_readonly("axis_z", &vl::MouseEvent::Z)
		.def_readonly("buttons", &vl::MouseEvent::buttons)
		//.def_readonly("head_position", &vl::MouseEvent::head_position)
		//.def_readonly("head_orientation", &vl::MouseEvent::head_orientation)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::MouseEvent::Axis>("Axis", python::init<>())
		.add_property("abs", &vl::MouseEvent::Axis::abs)
		.add_property("rel", &vl::MouseEvent::Axis::rel)
		.def("clear", &vl::MouseEvent::Axis::clear)
	;

	python::enum_<vl::MouseEvent::BUTTON>("MOUSEBUTTON_ID")
		.value("MB_L", vl::MouseEvent::MB_L)
		.value("MB_R", vl::MouseEvent::MB_R)
		.value("MB_M", vl::MouseEvent::MB_M)
		.value("MB_3", vl::MouseEvent::MB_3)
		.value("MB_4", vl::MouseEvent::MB_4)
		.value("MB_5", vl::MouseEvent::MB_5)
		.value("MB_6", vl::MouseEvent::MB_6)
		.value("MB_7", vl::MouseEvent::MB_7)
	;

	python::class_<vl::InputDevice, vl::InputDeviceRefPtr, boost::noncopyable>("InputDevice", python::no_init)
	;

	python::class_<vl::PCAN, vl::PCANRefPtr, boost::noncopyable,  python::bases<vl::InputDevice> >("PCAN", python::no_init)
		.def("add_listener", toast::python::signal_connect<void (CANMsg const &)>(&vl::PCAN::addListener))
	;

	python::def( "getKeyName", getKeyName );

	python::def( "getPythonKeyName", getPythonKeyName );

	python::enum_<OIS::KeyCode> python_keycode = python::enum_<OIS::KeyCode>("KC");

	int i = 0;
	while( i < OIS::KC_MEDIASELECT )
	{
		OIS::KeyCode code = (OIS::KeyCode)(i);
		std::string keyName = getKeyName( code );
		if( !keyName.empty() )
		{
			python_keycode.value( keyName.c_str(), code );
		}

		++i;
	}
}

void export_triggers(void)
{
	// TODO try the Abstract classes out by overriding them in python
	// NOTE Abstract wrappers seem to be needed for inheriting from these classes
	// in python
	// Problem : how to expose these and get the hierarchy correct for c++ classes?
	// should we expose both the wrappers and abstract classes
	// should we declate the wrappers as bases for inherited c++ classes here
	// should we just expose the wrapper for python inheritance and use the c++
	// classes and bases otherwise?
	python::class_<Trigger, boost::noncopyable>("Trigger", python::no_init )
		// FIXME declaring getTypeName as virtual does not work
		// (might be because it's a property not a function)
		.add_property("type", &Trigger::getTypeName )
		.def("getName", &Trigger::getName )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<BasicActionTrigger, boost::noncopyable, python::bases<Trigger> >("BasicActionTrigger", python::no_init )
		// @todo fix this to use toast for callbacks
		.def("addListener", toast::python::signal_connect<void (void)>(&vl::BasicActionTrigger::addListener))
	;

	python::class_<TransformActionTrigger, boost::noncopyable, python::bases<Trigger> >("TransformActionTrigger", python::no_init )
		// @todo fix this to use toast for callbacks
		.def("addListener", toast::python::signal_connect<void (vl::Transform const &)>(&vl::TransformActionTrigger::addListener))
	;
	
	python::class_<vl::TrackerTrigger, python::bases<vl::TransformActionTrigger>, boost::noncopyable>("TrackerTrigger", python::no_init)
		.add_property("name", &vl::TrackerTrigger::getName, &vl::TrackerTrigger::setName)
	;
	
	python::class_<FrameTrigger, boost::noncopyable, python::bases<Trigger> >("FrameTrigger", python::no_init )
		.def("addListener", toast::python::signal_connect<void (vl::time const &)>(&vl::FrameTrigger::addListener))
	;

	python::class_<TimeTrigger, boost::noncopyable, python::bases<Trigger> >("TimeTrigger", python::no_init )
		.def("addListener", toast::python::signal_connect<void (void)>(&vl::TimeTrigger::addListener))
		.def("reset", &vl::TimeTrigger::reset)
		.add_property("expired", &vl::TimeTrigger::isExpired)
		.add_property("interval", python::make_function(&vl::TimeTrigger::getInterval, python::return_value_policy<python::copy_const_reference>()), &vl::TimeTrigger::setInterval)
		.add_property("continuous", &vl::TimeTrigger::isContinous, &vl::TimeTrigger::setContinous)
	;

	python::class_<vl::KeyTrigger, boost::noncopyable, python::bases<Trigger> >("KeyTrigger", python::no_init )
		.add_property("key", &vl::KeyTrigger::getKey, &vl::KeyTrigger::setKey )
		.add_property("modifiers", &vl::KeyTrigger::getModifiers, &vl::KeyTrigger::setModifiers )
		.def("addKeyUpListener", toast::python::signal_connect<void (void)>(&vl::KeyTrigger::addKeyUpListener))
		.def("addKeyDownListener", toast::python::signal_connect<void (void)>(&vl::KeyTrigger::addKeyDownListener))
		.def("addListener", toast::python::signal_connect<void (void)>(&vl::KeyTrigger::addListener))
	;

	
	python::class_<vl::MouseTrigger, boost::noncopyable, python::bases<Trigger> >("MouseTrigger", python::no_init )
		.def("addMovedListener", toast::python::signal_connect<void(vl::MouseEvent const &)>(&vl::MouseTrigger::addMovedListener))
		.def("addButtonDownListener", toast::python::signal_connect<void(vl::MouseEvent const &, vl::MouseEvent::BUTTON)>(&vl::MouseTrigger::addButtonPressedListener))
		.def("addButtonUpListener", toast::python::signal_connect<void(vl::MouseEvent const &, vl::MouseEvent::BUTTON)>(&vl::MouseTrigger::addButtonReleasedListener))
	;

	python::class_<vl::JoystickTrigger, boost::noncopyable, python::bases<Trigger> >("JoystickTrigger", python::no_init )
		.def("addListener", toast::python::signal_connect<void(vl::JoystickEvent const &, int)>(&vl::JoystickTrigger::addListener))
		.def("addButtonPressedListener", toast::python::signal_connect<void(vl::JoystickEvent const &, int)>(&vl::JoystickTrigger::addButtonPressedListener))
		.def("addButtonReleasedListener", toast::python::signal_connect<void(vl::JoystickEvent const &, int)>(&vl::JoystickTrigger::addButtonReleasedListener))
		.def("addAxisListener", toast::python::signal_connect<void(vl::JoystickEvent const &, int)>(&vl::JoystickTrigger::addAxisListener))
		.def("addVectorListener", toast::python::signal_connect<void(vl::JoystickEvent const &, int)>(&vl::JoystickTrigger::addVectorListener))


	;	
		



}


}

void export_events(void)
{
	export_managers();
	export_triggers();
}
