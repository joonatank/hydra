#ifndef EQ_OGRE_CONFIG_PYTHON_HPP
#define EQ_OGRE_CONFIG_PYTHON_HPP

#include "config.hpp"
#include "scene_node.hpp"
#include "keycode.hpp"
#include "event_manager.hpp"
#include "config_events.hpp"

#include <boost/python.hpp>

struct TriggerWrapper : eqOgre::Trigger, python::wrapper<eqOgre::Trigger>
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


struct EventWrapper : eqOgre::Event, python::wrapper<eqOgre::Event>
{
	bool processTrigger( eqOgre::Trigger *trig )
	{
		return this->get_override("processTrigger")();
	}

	std::string const &getTypeName( void ) const
	{
		return this->get_override("getTypeName")();
	}

};

struct OperationWrapper : eqOgre::Operation, python::wrapper<eqOgre::Operation>
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

inline std::ostream &operator<<( std::ostream &os, OperationWrapper const &o )
{
	o.print(os);
	return os;
}

namespace python = boost::python;

BOOST_PYTHON_MODULE(eqOgre_python)
{
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
		.def("addSceneNode", &Config::addSceneNode)
		.def("removeSceneNode", &Config::removeSceneNode)
	;

	python::class_<EventManager, boost::noncopyable>("EventManager", python::no_init)
		.def("createEvent", &EventManager::createEvent, python::return_value_policy<python::reference_existing_object>() )
		.def("createOperation", &EventManager::createOperation, python::return_value_policy<python::reference_existing_object>() )
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
	

	python::class_<OperationWrapper, boost::noncopyable>("Operation", python::no_init )
		// FIXME pure virtual getTypeName
		.add_property("type", python::make_function( &Operation::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		.def("execute", python::pure_virtual(&Operation::execute) )
		// FIXME this does not work
//		.def(python::str(python::self))
	;


	python::class_<EventWrapper, boost::noncopyable>("Event", python::no_init )
		.add_property("type", python::make_function( &Operation::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		.def("processTrigger", &Event::processTrigger)
		.def("removeTrigger", &Event::removeTrigger)
		.def("addTrigger", &Event::addTrigger)
		.add_property("operation", python::make_function( &Event::getOperation, python::return_value_policy< python::reference_existing_object>() ), &Event::setOperation)
		.add_property("time_limit", &Event::getTimeLimit, &Event::setTimeLimit)
		// FIXME this does not work
//		.def(python::str(python::self))
	;

	python::class_<ToggleEvent, boost::noncopyable>("ToggleEvent", python::no_init )
	;


	/// Config Operations

	python::class_<ConfigOperation, boost::noncopyable, python::bases<Operation> >("ConfigOperation", python::no_init )
		.add_property("config", python::make_function( &QuitOperation::getConfig, python::return_value_policy< python::reference_existing_object>() ), &QuitOperation::setConfig )
	;

	python::class_<QuitOperation, boost::noncopyable, python::bases<ConfigOperation> >("QuitOperation", python::no_init )
	;

	python::class_<ReloadScene, boost::noncopyable, python::bases<ConfigOperation> >("ReloadScene", python::no_init )
	;

	/// EventManager Operations
	python::class_<EventManagerOperation, boost::noncopyable, python::bases<Operation> >("EventManagerOperation", python::no_init )
		.add_property("event_manager", python::make_function( &EventManagerOperation::getManager, python::return_value_policy< python::reference_existing_object>() ), &EventManagerOperation::setManager )
	;
	
	python::class_<AddTransformOperation, boost::noncopyable, python::bases<EventManagerOperation> >("AddTransformOperation", python::no_init )
	;

	python::class_<RemoveTransformOperation, boost::noncopyable, python::bases<EventManagerOperation> >("RemoveTransformOperation", python::no_init )
	;

	/// SceneNode Operations
	python::class_<HideOperation, boost::noncopyable, python::bases<Operation> >("HideOperation", python::no_init )
		.add_property("scene_node", python::make_function( &HideOperation::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &HideOperation::setSceneNode )
	;
	


	python::class_<eqOgre::SceneNode>("SceneNode", python::no_init)
		// TODO the factory method should return ref counted ptr
		.def("create", &eqOgre::SceneNode::create,  python::return_value_policy<python::reference_existing_object>() ).staticmethod("create")
		.add_property("name", python::make_function( &eqOgre::SceneNode::getName, python::return_internal_reference<>() ), &eqOgre::SceneNode::setName )
		.add_property("position", python::make_function( &eqOgre::SceneNode::getPosition, python::return_internal_reference<>() ), &eqOgre::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &eqOgre::SceneNode::getOrientation, python::return_internal_reference<>() ), &eqOgre::SceneNode::setOrientation )
		.add_property("visibility", &SceneNode::getVisibility, &eqOgre::SceneNode::setVisibility )
	;

	python::class_<eqOgre::TransformationEvent, boost::noncopyable>("TransformationEvent", python::no_init )
		.add_property("speed", &eqOgre::TransformationEvent::getSpeed, &eqOgre::TransformationEvent::setSpeed )
		.add_property("name", python::make_function( &eqOgre::TransformationEvent::getAngularSpeed, python::return_internal_reference<>() ), &eqOgre::TransformationEvent::setAngularSpeed )
		.def("setTransXKeys", &eqOgre::TransformationEvent::setTransXtrigger )
		.def("setTransYKeys", &eqOgre::TransformationEvent::setTransYtrigger )
		.def("setTransZKeys", &eqOgre::TransformationEvent::setTransZtrigger )
		.def("setRotXKeys", &eqOgre::TransformationEvent::setRotXtrigger )
		.def("setRotYKeys", &eqOgre::TransformationEvent::setRotYtrigger )
		.def("setRotZKeys", &eqOgre::TransformationEvent::setRotZtrigger )
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