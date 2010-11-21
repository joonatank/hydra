#ifndef EQ_OGRE_CONFIG_PYTHON_HPP
#define EQ_OGRE_CONFIG_PYTHON_HPP

#include "config.hpp"
#include "scene_node.hpp"
#include "keycode.hpp"
#include "event_manager.hpp"
#include "config_events.hpp"

#include <boost/python.hpp>

struct TriggerWrapper : vl::Trigger, python::wrapper<vl::Trigger>
{
    std::string const &getTypeName( void ) const
    {
		return this->get_override("getTypeName")();
    }

    bool isEqual( Trigger const &other )
    {
		return this->get_override("isEqual")();
	}
};


struct EventWrapper : vl::Event, python::wrapper<vl::Event>
{
	bool processTrigger( vl::Trigger *trig )
	{
		return this->get_override("processTrigger")();
	}

	std::string const &getTypeName( void ) const
	{
		return this->get_override("getTypeName")();
	}

};

struct ActionWrapper : vl::Action, python::wrapper<vl::Action>
{
    std::string const &getTypeName( void ) const
    {
		return this->get_override("getTypeName")();
    }

	void execute( void )
	{
		this->get_override("execute")();
	}

};

inline std::ostream &operator<<( std::ostream &os, ActionWrapper const &o )
{
	o.print(os);
	return os;
}

namespace python = boost::python;

BOOST_PYTHON_MODULE(eqOgre_python)
{
	using namespace vl;
	using namespace eqOgre;

	// TODO check for overloads and default arguments, they need some extra work

	python::class_<Ogre::Vector3>("Vector3", python::init<Ogre::Real, Ogre::Real, Ogre::Real>() )
		.def_readwrite("x", &Ogre::Vector3::x)
		.def_readwrite("y", &Ogre::Vector3::y)
		.def_readwrite("z", &Ogre::Vector3::z)
		.def("length", &Ogre::Vector3::length)
		.def("normalise", &Ogre::Vector3::normalise)
		// Operators
		.def(python::self + python::self )
		.def(python::self - python::self )
		.def(python::self * python::self )
		.def(python::self * Ogre::Real() )
		.def(python::self / python::self )
		.def(python::self / Ogre::Real() )
		.def(python::self += python::self )
		.def(python::self += Ogre::Real() )
		.def(python::self -= python::self )
		.def(python::self -= Ogre::Real() )
		.def(python::self *= python::self )
		.def(python::self *= Ogre::Real() )
		.def(python::self /= python::self )
		.def(python::self /= Ogre::Real() )
		// Comparison
		.def(python::self == python::self )
		.def(python::self != python::self )
		;

	python::class_<Ogre::Quaternion>("Quaternion", python::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>() )
		.def_readwrite("x", &Ogre::Quaternion::x)
		.def_readwrite("y", &Ogre::Quaternion::y)
		.def_readwrite("z", &Ogre::Quaternion::z)
		.def_readwrite("w", &Ogre::Quaternion::w)
		.def("Norm", &Ogre::Quaternion::Norm)
		.def("normalise", &Ogre::Quaternion::normalise)
		.def("equals", &Ogre::Quaternion::equals)
		.def("isNaN", &Ogre::Quaternion::isNaN)
		// Operators
		.def(python::self + python::self )
		.def(python::self - python::self )
		.def(python::self * python::self )
		.def(python::self * Ogre::Vector3() )
		.def(python::self * Ogre::Real() )
		.def(python::self == python::self )
		.def(python::self != python::self )
		;

	python::class_<Ogre::Radian>("Radian")
	;

	python::class_<Config, boost::noncopyable>("Config", python::no_init)
		.def("removeSceneNode", &Config::removeSceneNode )
		.def("getSceneNode", &Config::getSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("setActiveCamera", &Config::setActiveCamera)
	;

	python::class_<EventManager, boost::noncopyable>("EventManager", python::no_init)
		.def("createEvent", &EventManager::createEvent, python::return_value_policy<python::reference_existing_object>() )
		.def("createAction", &EventManager::createAction, python::return_value_policy<python::reference_existing_object>() )
		.def("createTrigger", &EventManager::createTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("addEvent", &EventManager::addEvent)
		.def("removeEvent", &EventManager::removeEvent)
		.def("hasEvent", &EventManager::hasEvent)
//		.def(python::str(python::self))
	;

	// TODO try the Abstract classes out by overriding them in python
	// NOTE Abstract wrappers seem to be needed for inheriting from these classes
	// in python
	// Problem : how to expose these and get the hierarchy correct for c++ classes?
	// should we expose both the wrappers and abstract classes
	// should we declate the wrappers as bases for inherited c++ classes here
	// should we just expose the wrapper for python inheritance and use the c++
	// classes and bases otherwise?
	python::class_<TriggerWrapper, boost::noncopyable>("Trigger", python::no_init )
		.def(python::self == python::self )
		// FIXME declaring getTypeName as virtual does not work
		// (might be because it's a property not a function)
		.add_property("type", python::make_function( &KeyTrigger::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		.def("isEqual", python::pure_virtual(&Trigger::isEqual) )
//		.def(python::str(python::self))
	;

	python::class_<KeyTrigger, boost::noncopyable, python::bases<Trigger> >("KeyTrigger", python::no_init )
		.def(python::self == python::self )
		.add_property("key", &KeyTrigger::getKey, &KeyTrigger::setKey )
//		.add_property("released", &KeyTrigger::getReleased, &KeyTrigger::setReleased )
	;

	python::class_<KeyPressedTrigger, boost::noncopyable, python::bases<KeyTrigger> >("KeyPressedTrigger", python::no_init )
	;

	python::class_<KeyReleasedTrigger, boost::noncopyable, python::bases<KeyTrigger> >("KeyReleasedTrigger", python::no_init )
	;


	python::class_<ActionWrapper, boost::noncopyable>("Action", python::no_init )
		.add_property("type", python::make_function( &Action::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		// FIXME this does not work
//		.def(python::str(python::self))
	;

	// TODO needs a wrapper
	python::class_<BasicAction, boost::noncopyable, python::bases<Action> >("BasicAction", python::no_init )
		.def("execute", python::pure_virtual(&BasicAction::execute) )
		.add_property("type", python::make_function( &Action::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		// FIXME this does not work
//		.def(python::str(python::self))
	;

	python::class_<EventWrapper, boost::noncopyable>("Event", python::no_init )
		.add_property("type", python::make_function( &Event::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		.def("processTrigger", &Event::processTrigger)
		.def("removeTrigger", &Event::removeTrigger)
		.def("addTrigger", &Event::addTrigger)
		.add_property("action", python::make_function( &Event::getAction, python::return_value_policy< python::reference_existing_object>() ), &Event::setAction)
		.add_property("time_limit", &Event::getTimeLimit, &Event::setTimeLimit)
		// FIXME this does not work
//		.def(python::str(python::self))
	;

	python::class_<ToggleEvent, boost::noncopyable, python::bases<Event> >("ToggleEvent", python::no_init )
		.add_property("toggle_off_action", python::make_function( &ToggleEvent::getToggleOff, python::return_value_policy< python::reference_existing_object>() ), &ToggleEvent::setToggleOff )
		.add_property("toggle_on_action", python::make_function( &ToggleEvent::getToggleOn, python::return_value_policy< python::reference_existing_object>() ), &ToggleEvent::setToggleOn )
		.add_property("toggle_state", &ToggleEvent::getToggleState, &ToggleEvent::setToggleState )
	;


	/// Config Operations

	python::class_<ConfigOperation, boost::noncopyable, python::bases<BasicAction> >("ConfigOperation", python::no_init )
		.add_property("config", python::make_function( &QuitOperation::getConfig, python::return_value_policy< python::reference_existing_object>() ), &QuitOperation::setConfig )
	;

	python::class_<QuitOperation, boost::noncopyable, python::bases<ConfigOperation> >("QuitOperation", python::no_init )
	;

	python::class_<ReloadScene, boost::noncopyable, python::bases<ConfigOperation> >("ReloadScene", python::no_init )
	;

	python::class_<ToggleMusic, boost::noncopyable, python::bases<ConfigOperation> >("ToggleMusic", python::no_init )
	;

	python::class_<ActivateCamera, boost::noncopyable, python::bases<ConfigOperation> >("ActivateCamera", python::no_init )
		.add_property("camera", python::make_function( &ActivateCamera::getCamera, python::return_value_policy<python::copy_const_reference>() ), &ActivateCamera::setCamera )
	;

	/// EventManager Operations
	python::class_<EventManagerOperation, boost::noncopyable, python::bases<BasicAction> >("EventManagerOperation", python::no_init )
		.add_property("event_manager", python::make_function( &EventManagerOperation::getManager, python::return_value_policy< python::reference_existing_object>() ), &EventManagerOperation::setManager )
	;

	python::class_<AddTransformOperation, boost::noncopyable, python::bases<EventManagerOperation> >("AddTransformOperation", python::no_init )
	;

	python::class_<RemoveTransformOperation, boost::noncopyable, python::bases<EventManagerOperation> >("RemoveTransformOperation", python::no_init )
	;

	/// SceneNode Operations
	// TODO expose the Base class
	python::class_<HideAction, boost::noncopyable, python::bases<BasicAction> >("HideAction", python::no_init )
		.add_property("scene_node", python::make_function( &HideAction::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &HideAction::setSceneNode )
	;

	python::class_<ShowAction, boost::noncopyable, python::bases<BasicAction> >("ShowAction", python::no_init )
		.add_property("scene_node", python::make_function( &ShowAction::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &ShowAction::setSceneNode )
	;


	python::class_<eqOgre::SceneNode>("SceneNode", python::no_init)
		.add_property("name", python::make_function( &eqOgre::SceneNode::getName, python::return_internal_reference<>() ), &eqOgre::SceneNode::setName )
		.add_property("position", python::make_function( &eqOgre::SceneNode::getPosition, python::return_internal_reference<>() ), &eqOgre::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &eqOgre::SceneNode::getOrientation, python::return_internal_reference<>() ), &eqOgre::SceneNode::setOrientation )
		.add_property("visibility", &SceneNode::getVisibility, &eqOgre::SceneNode::setVisibility )
	;

	python::class_<TransformationEvent, boost::noncopyable, python::bases<Event> >("TransformationEvent", python::no_init )
		.add_property("scene_node", python::make_function( &TransformationEvent::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &TransformationEvent::setSceneNode )
		.add_property("speed", &eqOgre::TransformationEvent::getSpeed, &eqOgre::TransformationEvent::setSpeed )
		.add_property("angular_speed", python::make_function( &eqOgre::TransformationEvent::getAngularSpeed, python::return_internal_reference<>() ), &eqOgre::TransformationEvent::setAngularSpeed )
		.def("setTransXtrigger", &eqOgre::TransformationEvent::setTransXtrigger )
		.def("setTransYtrigger", &eqOgre::TransformationEvent::setTransYtrigger )
		.def("setTransZtrigger", &eqOgre::TransformationEvent::setTransZtrigger )
		.def("setRotXtrigger", &eqOgre::TransformationEvent::setRotXtrigger )
		.def("setRotYtrigger", &eqOgre::TransformationEvent::setRotYtrigger )
		.def("setRotZtrigger", &eqOgre::TransformationEvent::setRotZtrigger )
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

#endif
