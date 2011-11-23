/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file python/python_events.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Interface
#include "python_module.hpp"

// Event handling
#include "event_manager.hpp"
#include "actions_transform.hpp"
#include "actions_misc.hpp"
#include "trigger.hpp"

// Necessary for SceneNode actions
#include "scene_node.hpp"
// Necessary for constraint actions
#include "animation/constraints.hpp"
// Necessary for joystick handler
#include "animation/constraints_handlers.hpp"

// Necessary because referenced by the Actions
#include "scene_manager.hpp"
#include "game_manager.hpp"
#include "player.hpp"

// Necessary for transforming OIS keycodes to python
#include "keycode.hpp"

// Input
#include "input/joystick_event.hpp"
#include "input/serial_joystick.hpp"
#include "input/joystick.hpp"


#include "vrpn_analog_client.hpp"

/// Event system overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createKeyTrigger_ov, createKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getKeyTrigger_ov, getKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hasKeyTrigger_ov, hasKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getJoystick_ov, getJoystick, 0, 2)

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
		.def("getJoystick", &vl::EventManager::getJoystick, getJoystick_ov())
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
		.add_property("transformation", python::make_function(&vl::Tracker::getTransformation, python::return_value_policy<python::copy_const_reference>()), &vl::Tracker::setTransformation )
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

	
	bool (vl::JoystickEvent::*isButtonDown_ov0)(size_t) const = &vl::JoystickEvent::isButtonDown;

	python::class_<vl::JoystickEvent>("JoystickEvent", python::init<>())
		.def_readonly("axis_x", &vl::JoystickEvent::axis_x)
		.def_readonly("axis_y", &vl::JoystickEvent::axis_y)
		.def("is_button_down", isButtonDown_ov0)
	;

	/// Input
	
	/// Handlers
	python::class_<vl::JoystickHandler, vl::JoystickHandlerRefPtr, boost::noncopyable>("JoystickHandler", python::no_init)
	;

	void (vl::ConstraintJoystickHandler::*set_axis_constraint_ov0)(int, ConstraintRefPtr)
		= &vl::ConstraintJoystickHandler::set_axis_constraint;
	void (vl::ConstraintJoystickHandler::*set_axis_constraint_ov1)(int, int, ConstraintRefPtr)
		= &vl::ConstraintJoystickHandler::set_axis_constraint;

	python::class_<vl::ConstraintJoystickHandler, vl::ConstraintJoystickHandlerRefPtr, boost::noncopyable, python::bases<vl::JoystickHandler> >("ConstraintJoystickHandler", python::no_init)
		.def("set_axis_constraint", set_axis_constraint_ov0)
		.def("set_axis_constraint", set_axis_constraint_ov1)
		.add_property("velocity_multiplier", &vl::ConstraintJoystickHandler::get_velocity_multiplier, &vl::ConstraintJoystickHandler::set_velocity_multiplier)
		.def("create", &vl::ConstraintJoystickHandler::create)
		.staticmethod("create")
	;

	python::class_<vl::InputDevice, vl::InputDeviceRefPtr, boost::noncopyable>("InputDevice", python::no_init)
	;

	python::class_<vl::Joystick, vl::JoystickRefPtr, boost::noncopyable,  python::bases<vl::InputDevice> >("Joystick", python::no_init)
		.def("addListener", toast::python::signal_connect<void (JoystickEvent const &)>(&vl::Joystick::addListener))
		.def("add_handler", &vl::Joystick::add_handler)
		.def("remove_handler", &vl::Joystick::remove_handler)
		.add_property("zero_size", &vl::Joystick::get_zero_size, &vl::Joystick::set_zero_size)
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
		.add_property("action", python::make_function( &vl::BasicActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ) )
		// @todo fix this to use toast for callbacks
		.def("addListener", toast::python::signal_connect<void (void)>(&vl::BasicActionTrigger::addListener))
	;

	python::class_<TransformActionTrigger, boost::noncopyable, python::bases<Trigger> >("TransformActionTrigger", python::no_init )
		.add_property("action", python::make_function( &vl::TransformActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ))
		// @todo fix this to use toast for callbacks
		.def("addListener", toast::python::signal_connect<void (vl::Transform const &)>(&vl::TransformActionTrigger::addListener))
	;
	
	python::class_<vl::TrackerTrigger, python::bases<vl::TransformActionTrigger>, boost::noncopyable>("TrackerTrigger", python::no_init)
		.add_property("name", &vl::TrackerTrigger::getName, &vl::TrackerTrigger::setName)
	;
	
	python::class_<FrameTrigger, boost::noncopyable, python::bases<Trigger> >("FrameTrigger", python::no_init )
		.add_property("action", python::make_function(&vl::FrameTrigger::getAction, python::return_value_policy< python::reference_existing_object>()))
		.def("addListener", toast::python::signal_connect<void (vl::time const &)>(&vl::FrameTrigger::addListener))
	;

	python::class_<vl::KeyTrigger, boost::noncopyable, python::bases<Trigger> >("KeyTrigger", python::no_init )
		.add_property("key", &vl::KeyTrigger::getKey, &vl::KeyTrigger::setKey )
		.add_property("modifiers", &vl::KeyTrigger::getModifiers, &vl::KeyTrigger::setModifiers )
		.add_property("action_down", python::make_function( &vl::KeyTrigger::getKeyDownAction, python::return_value_policy< python::reference_existing_object>() ), &vl::KeyTrigger::setKeyDownAction)
		.add_property("action_up", python::make_function( &vl::KeyTrigger::getKeyUpAction, python::return_value_policy< python::reference_existing_object>() ), &vl::KeyTrigger::setKeyUpAction)
		.def("addKeyUpListener", toast::python::signal_connect<void (void)>(&vl::KeyTrigger::addKeyUpListener))
		.def("addKeyDownListener", toast::python::signal_connect<void (void)>(&vl::KeyTrigger::addKeyDownListener))
		.def("addListener", toast::python::signal_connect<void (void)>(&vl::KeyTrigger::addListener))
	;
}

void export_action_proxies(void)
{
	// @todo add start state changing
	python::class_<ToggleActionProxy, boost::noncopyable, python::bases<BasicAction> >("ToggleActionProxy", python::no_init )
		.add_property("action_on", python::make_function( &ToggleActionProxy::getActionOn, python::return_value_policy< python::reference_existing_object>() ), &ToggleActionProxy::setActionOn)
		.add_property("action_off", python::make_function( &ToggleActionProxy::getActionOff, python::return_value_policy< python::reference_existing_object>() ), &ToggleActionProxy::setActionOff)
		.def("create",&ToggleActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<BufferActionProxy, boost::noncopyable, python::bases<BasicAction> >("BufferActionProxy", python::no_init )
		.def("add_action", &BufferActionProxy::addAction)
		.def("rem_action", &BufferActionProxy::remAction)
		.def("get_action", python::make_function( &BufferActionProxy::getAction, python::return_value_policy< python::reference_existing_object>() ) )
		.def("create", &BufferActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<GroupActionProxy, boost::noncopyable, python::bases<BasicAction> >("GroupActionProxy", python::no_init )
		.def("add_action", &GroupActionProxy::addAction)
		.def("rem_action", &GroupActionProxy::remAction)
		.def("create", &GroupActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::GroupTransformActionProxy, boost::noncopyable, python::bases<vl::TransformAction> >("GroupTransformActionProxy", python::no_init )
		.def("add_action", &vl::GroupTransformActionProxy::addAction)
		.def("rem_action", &vl::GroupTransformActionProxy::remAction)
		.def("create", &GroupActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<TimerActionProxy, boost::noncopyable, python::bases<BasicAction> >("TimerActionProxy", python::no_init )
		.add_property("action", python::make_function( &TimerActionProxy::getAction, python::return_value_policy< python::reference_existing_object>() ), &TimerActionProxy::setAction )
		.add_property("time_limit", &TimerActionProxy::getTimeLimit, &TimerActionProxy::setTimeLimit )
		.def("create",&TimerActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	/// TransformationAction stuff
	python::class_<FloatAction, boost::noncopyable, python::bases<Action> >("FloatAction", python::no_init )
	;

	python::class_<VectorAction, boost::noncopyable, python::bases<Action> >("VectorAction", python::no_init )
	;

	python::class_<FloatActionMap, boost::noncopyable, python::bases<BasicAction> >("FloatActionMap", python::no_init )
		.def_readwrite("value", &FloatActionMap::value )
		.add_property("action", python::make_function( &FloatActionMap::getAction, python::return_value_policy< python::reference_existing_object>() ), &FloatActionMap::setAction)
		.def("create",&FloatActionMap::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<VectorActionMap, boost::noncopyable, python::bases<FloatAction> >("VectorActionMap", python::no_init )
		.def_readwrite("axis", &VectorActionMap::axis )
		.add_property("action", python::make_function( &VectorActionMap::getAction, python::return_value_policy< python::reference_existing_object>() ), &VectorActionMap::setAction)
		.def("create",&VectorActionMap::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;
}

void export_actions(void)
{
	python::class_<vl::SetTransformation, boost::noncopyable, python::bases<vl::TransformAction> >("SetTransformation", python::no_init )
		.add_property("scene_node", python::make_function( &vl::SetTransformation::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &vl::SetTransformation::setSceneNode )
		.def("create",&SetTransformation::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::HeadTrackerAction, python::bases<vl::TransformAction>, boost::noncopyable>("HeadTrackerAction", python::no_init)
		.add_property("player", python::make_function(&vl::HeadTrackerAction::getPlayer, python::return_value_policy<python::reference_existing_object>()), &vl::HeadTrackerAction::setPlayer)
		.def("create", &vl::HeadTrackerAction::create, python::return_value_policy<python::reference_existing_object>())
		.staticmethod("create")
	;

	/// Game Actions
	python::class_<GameAction, boost::noncopyable, python::bases<BasicAction> >("GameAction", python::no_init )
		.def_readwrite("game", &GameAction::data )
	;

	python::class_<ScriptAction, boost::noncopyable, python::bases<GameAction> >("ScriptAction", python::no_init )
		.def_readwrite("script", &vl::ScriptAction::script )
		.def("create",&ScriptAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<QuitAction, boost::noncopyable, python::bases<GameAction> >("QuitAction", python::no_init )
		.def("create", &QuitAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<ToggleMusic, boost::noncopyable, python::bases<GameAction> >("ToggleMusic", python::no_init )
		.def("create",&ToggleMusic::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;


	/// SceneManager Actions
	python::class_<SceneManagerAction, boost::noncopyable, python::bases<BasicAction> >("SceneAction", python::no_init )
		.def_readwrite("scene", &SceneManagerAction::data )
	;

	python::class_<ReloadScene, boost::noncopyable, python::bases<SceneManagerAction> >("ReloadScene", python::no_init )
		.def("create",&ReloadScene::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<AddToSelection, boost::noncopyable, python::bases<SceneManagerAction> >("AddToSelection", python::no_init )
		.add_property("scene_node", python::make_function( &AddToSelection::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &AddToSelection::setSceneNode )
		.def("create",&AddToSelection::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<RemoveFromSelection, boost::noncopyable, python::bases<SceneManagerAction> >("RemoveFromSelection", python::no_init )
		.add_property("scene_node", python::make_function( &RemoveFromSelection::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &RemoveFromSelection::setSceneNode )
		.def("create", &RemoveFromSelection::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<PlayerAction, boost::noncopyable, python::bases<BasicAction> >("PlayerAction", python::no_init )
		.def_readwrite("player", &PlayerAction::data )
	;

	python::class_<ActivateCamera, boost::noncopyable, python::bases<PlayerAction> >("ActivateCamera", python::no_init )
		.add_property("camera", python::make_function( &ActivateCamera::getCamera, python::return_value_policy<python::copy_const_reference>() ), &ActivateCamera::setCamera )
		.def("create",&ActivateCamera::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<ScreenshotAction, boost::noncopyable, python::bases<PlayerAction> >("ScreenshotAction", python::no_init )
		.def("create",&ScreenshotAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	/// SceneNode Actions
	// TODO expose the Base class
	python::class_<HideAction, boost::noncopyable, python::bases<BasicAction> >("HideAction", python::no_init )
		.add_property("scene_node", python::make_function( &HideAction::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &HideAction::setSceneNode )
		.def("create",&HideAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<ShowAction, boost::noncopyable, python::bases<BasicAction> >("ShowAction", python::no_init )
		.add_property("scene_node", python::make_function( &ShowAction::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &ShowAction::setSceneNode )
		.def("create",&ShowAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<TransformationAction, boost::noncopyable, python::bases<BasicAction> >("TransformationAction", python::no_init )
	;

	python::class_<MoveAction, boost::noncopyable, python::bases<TransformationAction> >("MoveAction", python::no_init )
		.add_property("speed", &MoveAction::getSpeed, &MoveAction::setSpeed)
		.add_property("angular_speed", python::make_function(&MoveAction::getAngularSpeed, python::return_value_policy<python::copy_const_reference>()), &MoveAction::setAngularSpeed )
		.add_property("local", &MoveAction::getLocal, &MoveAction::setLocal)
		.add_property("reference", python::make_function(&MoveAction::getReference, python::return_value_policy< python::reference_existing_object>()), &MoveAction::setReference)
		.def("isLocal", &MoveAction::isLocal)
		.def("isGlobal", &MoveAction::isGlobal)
	;

	python::class_<MoveNodeAction, boost::noncopyable, python::bases<MoveAction> >("MoveNodeAction", python::no_init )
		.add_property("scene_node", python::make_function( &MoveNodeAction::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &MoveNodeAction::setSceneNode)
		.def("create",&MoveNodeAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<MoveSelectionAction, boost::noncopyable, python::bases<MoveAction> >("MoveSelectionAction", python::no_init )
		.add_property("scene", python::make_function( &MoveSelectionAction::getSceneManager, python::return_value_policy< python::reference_existing_object>() ), &MoveSelectionAction::setSceneManager)
		.def("create",&MoveSelectionAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<MoveActionProxy, boost::noncopyable, python::bases<VectorAction> >("MoveActionProxy", python::no_init )
		.add_property("action", python::make_function( &MoveActionProxy::getAction, python::return_value_policy< python::reference_existing_object>() ), &MoveActionProxy::setAction )
		.def("enableRotation", &MoveActionProxy::enableRotation )
		.def("disableRotation", &MoveActionProxy::disableRotation )
		.def("enableTranslation", &MoveActionProxy::enableTranslation )
		.def("disableTranslation", &MoveActionProxy::disableTranslation )
		.def("create",&MoveActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	/// Constraint actions
	python::class_<vl::SliderActuatorAction, boost::noncopyable, python::bases<vl::BasicAction> >("SliderActuatorAction", python::no_init)
		.def_readwrite("constraint", &vl::SliderActuatorAction::constraint)
		.def_readwrite("target", &vl::SliderActuatorAction::target)
		.def("create",&vl::SliderActuatorAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::HingeActuatorAction, boost::noncopyable, python::bases<vl::BasicAction> >("HingeActuatorAction", python::no_init)
		.def_readwrite("constraint", &vl::HingeActuatorAction::constraint)
		.def_readwrite("target", &vl::HingeActuatorAction::target)
		.def("create",&vl::HingeActuatorAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;
}

}

void export_events(void)
{
	// TODO replace with a wrapper
	python::class_<Action, boost::noncopyable>("Action", python::no_init )
		.add_property("type", &Action::getTypeName)
		.def(python::self_ns::str(python::self_ns::self))
	;

	// TODO needs a wrapper
	python::class_<BasicAction, boost::noncopyable, python::bases<Action> >("BasicAction", python::no_init )
	;

	python::class_<vl::TransformAction, boost::noncopyable, python::bases<Action> >("TransformAction", python::no_init )
	;

	export_managers();
	export_triggers();
	export_action_proxies();
	export_actions();
}
