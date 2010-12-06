#ifndef EQ_OGRE_CONFIG_PYTHON_HPP
#define EQ_OGRE_CONFIG_PYTHON_HPP

#include "config.hpp"
#include "scene_node.hpp"
#include "keycode.hpp"
#include "event_manager.hpp"
#include "config_events.hpp"
#include "game_manager.hpp"

#include <boost/python.hpp>
#include <player.hpp>

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

struct ActionWrapper : vl::Action, python::wrapper<vl::Action>
{
    std::string getTypeName( void ) const
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

BOOST_PYTHON_MODULE(eqOgre)
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
		.def(-python::self )
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

	python::class_<vl::GameManager, boost::noncopyable>("GameManager", python::no_init)
		.add_property("scene", python::make_function( &vl::GameManager::getSceneManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("player", python::make_function( &vl::GameManager::getPlayer, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("event_manager", python::make_function( &vl::GameManager::getEventManager, python::return_value_policy<python::reference_existing_object>() ) )
		//.def("getSceneManager", &vl::GameManager::getSceneManager, python::return_value_policy<python::reference_existing_object>() )
//		.def("setActiveCamera", &vl::GameManager::setActiveCamera )
	;

	// TODO add setHeadMatrix function to python
	python::class_<vl::Player, boost::noncopyable>("Player", python::no_init)
		.add_property("camera", python::make_function( &Player::getActiveCamera , python::return_internal_reference<>() ), &Player::setActiveCamera )
	;


	// Overloads for the getSceneNode
	// TODO const versions didn't go straight with the same return value policy
	SceneNodePtr (SceneManager::*getsn1)( std::string const & ) = &SceneManager::getSceneNode;
	const SceneNodePtr (SceneManager::*getsn2)( std::string const & ) const = &SceneManager::getSceneNode;
	SceneNodePtr (SceneManager::*getsn3)( size_t ) = &SceneManager::getSceneNode;
	const SceneNodePtr (SceneManager::*getsn4)( size_t ) const = &SceneManager::getSceneNode;

	python::class_<eqOgre::SceneManager, boost::noncopyable>("SceneManager", python::no_init)
		// TODO add remove and add SceneNodes
//		.def("removeSceneNode", &SceneManager::removeSceneNode )
		.def("hasSceneNode", &SceneManager::hasSceneNode )
		.def("getSceneNode", getsn1, python::return_value_policy<python::reference_existing_object>() )
		.def("getSceneNode", getsn3, python::return_value_policy<python::reference_existing_object>() )
		.def("reloadScene", &SceneManager::reloadScene)
	;


	python::class_<eqOgre::SceneNode>("SceneNode", python::no_init)
		.add_property("name", python::make_function( &eqOgre::SceneNode::getName, python::return_internal_reference<>() ), &eqOgre::SceneNode::setName )
		.add_property("position", python::make_function( &eqOgre::SceneNode::getPosition, python::return_internal_reference<>() ), &eqOgre::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &eqOgre::SceneNode::getOrientation, python::return_internal_reference<>() ), &eqOgre::SceneNode::setOrientation )
		.add_property("visibility", &SceneNode::getVisibility, &eqOgre::SceneNode::setVisibility )
	;

	python::class_<vl::EventManager, boost::noncopyable>("EventManager", python::no_init)
		.def("createEvent", &EventManager::createEvent, python::return_value_policy<python::reference_existing_object>() )
// 		.def("createAction", &EventManager::createAction, python::return_value_policy<python::reference_existing_object>() )
		.def("createTrigger", &EventManager::createTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("addEvent", &EventManager::addEvent)
		.def("removeEvent", &EventManager::removeEvent)
		.def("hasEvent", &EventManager::hasEvent)
		.def("getTrackerTrigger", &vl::EventManager::getTrackerTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("hasTrackerTrigger", &vl::EventManager::hasTrackerTrigger )
		.def("getKeyPressedTrigger", &vl::EventManager::getKeyPressedTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("createKeyPressedTrigger", &vl::EventManager::createKeyPressedTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("hasKeyPressedTrigger", &vl::EventManager::hasKeyPressedTrigger )
		.def("getKeyReleasedTrigger", &vl::EventManager::getKeyReleasedTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("createKeyReleasedTrigger", &vl::EventManager::createKeyReleasedTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("hasKeyReleasedTrigger", &vl::EventManager::hasKeyReleasedTrigger )
		.def("getFrameTrigger", &vl::EventManager::getFrameTrigger, python::return_value_policy<python::reference_existing_object>() )
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
		.add_property("type", python::make_function( &Trigger::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		.def("isEqual", python::pure_virtual(&Trigger::isEqual) )
//		.def(python::str(python::self))
	;

	python::class_<BasicActionTrigger, boost::noncopyable, python::bases<Trigger> >("BasicActionTrigger", python::no_init )
		.add_property("action", python::make_function( &BasicActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ), &BasicActionTrigger::setAction )
	;

	python::class_<TransformActionTrigger, boost::noncopyable, python::bases<Trigger> >("TransformActionTrigger", python::no_init )
		.add_property("action", python::make_function( &TransformActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ), &TransformActionTrigger::setAction)
	;

	python::class_<FrameTrigger, boost::noncopyable, python::bases<BasicActionTrigger> >("FrameTrigger", python::no_init )
	;

	python::class_<KeyTrigger, boost::noncopyable, python::bases<BasicActionTrigger> >("KeyTrigger", python::no_init )
// 		.def(python::self == python::self )
		.add_property("key", &KeyTrigger::getKey, &KeyTrigger::setKey )
// 		.add_property("action", python::make_function( &KeyTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ), &KeyTrigger::setAction)
	;

	python::class_<KeyPressedTrigger, boost::noncopyable, python::bases<KeyTrigger> >("KeyPressedTrigger", python::no_init )
	;

	python::class_<KeyReleasedTrigger, boost::noncopyable, python::bases<KeyTrigger> >("KeyReleasedTrigger", python::no_init )
	;


	python::class_<ActionWrapper, boost::noncopyable>("Action", python::no_init )
// 		.add_property("type", python::make_function( &Action::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		.add_property("type", &Action::getTypeName )
// 		FIXME this does not work
// 		.def(python::str(python::self))
	;

	// TODO needs a wrapper
	python::class_<BasicAction, boost::noncopyable, python::bases<Action> >("BasicAction", python::no_init )
// 		.def("execute", python::pure_virtual(&BasicAction::execute) )

// 		.add_property("type", python::make_function( &Action::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		// FIXME this does not work
//		.def(python::str(python::self))
	;

	python::class_<ToggleActionProxy, boost::noncopyable, python::bases<BasicAction> >("ToggleActionProxy", python::no_init )
		.add_property("action_on", python::make_function( &ToggleActionProxy::getActionOn, python::return_value_policy< python::reference_existing_object>() ), &ToggleActionProxy::setActionOn)
		.add_property("action_off", python::make_function( &ToggleActionProxy::getActionOff, python::return_value_policy< python::reference_existing_object>() ), &ToggleActionProxy::setActionOff)
		.def("create",&ToggleActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;


	python::class_<TimerActionProxy, boost::noncopyable, python::bases<BasicAction> >("TimerActionProxy", python::no_init )
		.add_property("action", python::make_function( &TimerActionProxy::getAction, python::return_value_policy< python::reference_existing_object>() ), &TimerActionProxy::setAction )
		.add_property("time_limit", &TimerActionProxy::getTimeLimit, &TimerActionProxy::setTimeLimit )
		.def("create",&TimerActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;



	python::class_<vl::TransformAction, boost::noncopyable, python::bases<Action> >("TransformAction", python::no_init )
	;

	python::class_<eqOgre::SetTransformation, boost::noncopyable, python::bases<vl::TransformAction> >("SetTransformation", python::no_init )
		.add_property("scene_node", python::make_function( &eqOgre::SetTransformation::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &eqOgre::SetTransformation::setSceneNode )
		.def("create",&SetTransformation::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
//		.def("execute", python::pure_virtual(&BasicAction::execute) )
//		.add_property("type", python::make_function( &Action::getTypeName, python::return_value_policy<python::copy_const_reference>()  )  )
		// FIXME this does not work
//		.def(python::str(python::self))
	;

	python::class_<vl::TrackerTrigger, boost::noncopyable, python::bases<vl::TransformActionTrigger> >("TrackerTrigger", python::no_init )
// 		.add_property("action", python::make_function( &TrackerTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ), &TrackerTrigger::setAction)
	;


	// TODO can be removed when the TransformationEvent has been removed
	python::class_<Event, boost::noncopyable>("Event", python::no_init )
	;

	python::class_<GameAction, boost::noncopyable, python::bases<BasicAction> >("GameAction", python::no_init )
		.def_readwrite("game", &GameAction::data )
// 		.add_property("game", python::make_function( &GameAction::getGame, python::return_value_policy< python::reference_existing_object>() ), &GameAction::setGame )
	;

	python::class_<QuitAction, boost::noncopyable, python::bases<GameAction> >("QuitAction", python::no_init )
		.def("create",&QuitAction::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<ToggleMusic, boost::noncopyable, python::bases<GameAction> >("ToggleMusic", python::no_init )
		.def("create",&ToggleMusic::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;


	python::class_<SceneManagerAction, boost::noncopyable, python::bases<BasicAction> >("SceneAction", python::no_init )
		.def_readwrite("scene", &SceneManagerAction::data )
	;

	python::class_<ReloadScene, boost::noncopyable, python::bases<SceneManagerAction> >("ReloadScene", python::no_init )
		.def("create",&ReloadScene::create, python::return_value_policy<python::reference_existing_object>() )
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

	python::class_<MoveAction, boost::noncopyable, python::bases<BasicAction> >("MoveAction", python::no_init )
		.add_property("scene_node", python::make_function( &MoveAction::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &MoveAction::setSceneNode)
		.add_property("speed", &MoveAction::getSpeed, &MoveAction::setSpeed )
		.add_property("angular_speed", python::make_function( &MoveAction::getAngularSpeed, python::return_internal_reference<>() ), &MoveAction::setAngularSpeed )
		.def("create",&MoveAction::create, python::return_value_policy<python::reference_existing_object>() )
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
