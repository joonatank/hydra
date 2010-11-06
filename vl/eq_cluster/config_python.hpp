#ifndef EQ_OGRE_CONFIG_PYTHON_HPP
#define EQ_OGRE_CONFIG_PYTHON_HPP

#include "config.hpp"
#include "scene_node.hpp"
#include "keycode.hpp"
#include "event_manager.hpp"
#include "frame_data_events.hpp"

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

namespace python = boost::python;

BOOST_PYTHON_MODULE(eqOgre_python)
{
	using namespace eqOgre;

	// NOTE renaming classes works fine
	// TODO registering objects is not working when using modules
	// or I don't know how to register the modules first
	// init the interpreter and then start calling functions one by one
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
	;

	//	TODO this causes problems about virtual getTypeName method
	python::class_<TriggerWrapper, boost::noncopyable>("Trigger", python::no_init )
		.def(python::self == python::self )
		.def("getTypeName", python::pure_virtual(&Trigger::getTypeName), python::return_value_policy<python::copy_const_reference>() )
		.def("isEqual", python::pure_virtual(&Trigger::isEqual) )
	;

/*	TODO this causes problems about static variables */
	python::class_<OperationWrapper, boost::noncopyable>("Operation", python::no_init )
		.def("getTypeName", python::pure_virtual(&Operation::getTypeName), python::return_value_policy<python::copy_const_reference>() )
		.def("getTypeName", python::pure_virtual(&Operation::execute) )
	;

	python::class_<EventWrapper, boost::noncopyable>("Event", python::no_init )
		.def("processTrigger", &Event::processTrigger)
//		.def_readonly("TYPNAME", &BasicEvent::TYPENAME ).staticmethod()
		.def("removeTrigger", &Event::removeTrigger)
		.def("addTrigger", &Event::addTrigger)
		.def("setOperation", &Event::setOperation)
		.def("setTimeLimit", &Event::setTimeLimit)
		.def("getTimeLimit", &Event::getTimeLimit)
		.def("getTypeName", python::pure_virtual(&Trigger::getTypeName), python::return_value_policy<python::copy_const_reference>() )
	;

	python::class_<AddTransformOperation, boost::noncopyable>("AddTransformOperation", python::no_init )
	;

	python::class_<RemoveTransformOperation, boost::noncopyable>("RemoveTransformOperation", python::no_init )
	;


	python::class_<QuitOperation, boost::noncopyable>("QuitOperation", python::no_init )
		.def("getTypeName", &QuitOperation::getTypeName, python::return_value_policy<python::copy_const_reference>() )
		.def("getTypeName", &QuitOperation::setConfig )
	;


	python::class_<ToggleEvent, boost::noncopyable>("ToggleEvent", python::no_init )
	;

	python::class_<ReloadScene, boost::noncopyable>("ReloadScene", python::no_init )
	;

	python::class_<eqOgre::SceneNode>("SceneNode", python::no_init)
		// TODO the factory method should return ref counted ptr
		.def("create", &eqOgre::SceneNode::create,  python::return_value_policy<python::reference_existing_object>() ).staticmethod("create")
		.add_property("name", python::make_function( &eqOgre::SceneNode::getName, python::return_internal_reference<>() ), &eqOgre::SceneNode::setName )
		.add_property("position", python::make_function( &eqOgre::SceneNode::getPosition, python::return_internal_reference<>() ), &eqOgre::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &eqOgre::SceneNode::getOrientation, python::return_internal_reference<>() ), &eqOgre::SceneNode::setOrientation )
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