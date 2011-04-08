/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file python_module.cpp
 */
#ifndef VL_PYTHON_MODULE_HPP
#define VL_PYTHON_MODULE_HPP

#include "game_manager.hpp"
#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "player.hpp"

// Necessary for transforming OIS keycodes to python
#include "keycode.hpp"

// Event handling
#include "event_manager.hpp"
#include "actions_transform.hpp"
#include "actions_misc.hpp"
#include "trigger.hpp"

#include "gui/gui.hpp"

// Python global
#include "python.hpp"

/*
struct TriggerWrapper : vl::Trigger, python::wrapper<vl::Trigger>
{
    std::string const &getTypeName( void ) const
    {
		return this->get_override("getTypeName")();
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
*/

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createKeyPressedTrigger_ov, createKeyPressedTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getKeyPressedTrigger_ov, getKeyPressedTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hasKeyPressedTrigger_ov, hasKeyPressedTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createKeyReleasedTrigger_ov, createKeyReleasedTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getKeyReleasedTrigger_ov, getKeyReleasedTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hasKeyReleasedTrigger_ov, hasKeyReleasedTrigger, 1, 2)

BOOST_PYTHON_MODULE(vl)
{
	using namespace vl;

	python::class_<vl::GameManager, boost::noncopyable>("GameManager", python::no_init)
		.add_property("scene", python::make_function( &vl::GameManager::getSceneManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("player", python::make_function( &vl::GameManager::getPlayer, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("event_manager", python::make_function( &vl::GameManager::getEventManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("gui", python::make_function( &vl::GameManager::getGUI, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("stats", python::make_function( &vl::GameManager::getStats, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("logger", python::make_function( &vl::GameManager::getLogger, python::return_value_policy<python::reference_existing_object>() ) )
		.def("createBackgroundSound", &vl::GameManager::createBackgroundSound)
		.def("quit", &vl::GameManager::quit)
	;

	void (sink::*write1)( std::string const & ) = &sink::write;

	python::class_<vl::sink>("sink", python::no_init)
		.def("write", write1 )
	;

	// TODO add setHeadMatrix function to python
	python::class_<vl::Player, boost::noncopyable>("Player", python::no_init)
		.add_property("camera", python::make_function( &Player::getActiveCamera , python::return_value_policy<python::copy_const_reference>() ), &Player::setActiveCamera )
		.add_property("ipd", &Player::getIPD, &Player::setIPD)
	;

	python::class_<vl::Stats, boost::noncopyable>("Stats", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::EntityPtr (SceneManager::*createEntity_ov0)(std::string const &, vl::PREFAB) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov1)(std::string const &, std::string const &) = &SceneManager::createEntity;

	python::class_<vl::SceneManager, boost::noncopyable>("SceneManager", python::no_init)
		// TODO add remove SceneNodes
		.add_property("root", python::make_function( &SceneManager::getRootSceneNode, python::return_value_policy<python::reference_existing_object>() ) )
		.def("createSceneNode", &SceneManager::createSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("hasSceneNode", &SceneManager::hasSceneNode )
		.def("getSceneNode", &SceneManager::getSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("hasEntity", &SceneManager::hasEntity )
		.def("createEntity", createEntity_ov0, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov1, python::return_value_policy<python::reference_existing_object>() )
		.def("getEntity", &SceneManager::getEntity, python::return_value_policy<python::reference_existing_object>() )
		.def("reloadScene", &SceneManager::reloadScene)
		.def("addToSelection", &SceneManager::addToSelection)
		.def("removeFromSelection", &SceneManager::removeFromSelection)
		.add_property("ambient_light", python::make_function( &vl::SceneManager::getAmbientLight, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setAmbientLight )
	;

	python::enum_<vl::PREFAB>("PF")
		.value("NONE", PF_NONE)
		.value("PLANE", PF_PLANE)
		.value("SPHERE", PF_SPHERE)
		.value("CUBE", PF_CUBE)
	;

	python::class_<vl::Entity, boost::noncopyable>("Entity", python::no_init)
		.add_property("name", python::make_function( &vl::Entity::getName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("material_name", python::make_function( &vl::Entity::getMaterialName, python::return_value_policy<python::copy_const_reference>() ), &vl::Entity::setMaterialName )
		.add_property("mesh_name", python::make_function( &vl::Entity::getMeshName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("prefab", &vl::Entity::getPrefab)
	;


	python::class_<vl::SceneNode, boost::noncopyable>("SceneNode", python::no_init)
		.def("addEntity", &vl::SceneNode::addEntity)
		.def("removeEntity", &vl::SceneNode::removeEntity)
		.def("createChildSceneNode", &vl::SceneNode::createChildSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("addChild", &vl::SceneNode::addChild)
		.def("removeChild", &vl::SceneNode::removeChild)
		.add_property("name", python::make_function( &vl::SceneNode::getName, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setName )
		.add_property("position", python::make_function( &vl::SceneNode::getPosition, python::return_internal_reference<>() ), &vl::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &vl::SceneNode::getOrientation, python::return_internal_reference<>() ), &vl::SceneNode::setOrientation )
		.add_property("scale", python::make_function( &vl::SceneNode::getScale, python::return_internal_reference<>() ), &vl::SceneNode::setScale )
		.add_property("visibility", &SceneNode::getVisibility, &vl::SceneNode::setVisibility )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::enum_<KEY_MOD>("KEY_MOD")
		.value("NONE", KEY_MOD_NONE)
		.value("META", KEY_MOD_META)
		.value("CTRL", KEY_MOD_CTRL)
		.value("SHIFT", KEY_MOD_SHIFT)
		.value("SUPER", KEY_MOD_SUPER)
	;

	python::class_<vl::EventManager, boost::noncopyable>("EventManager", python::no_init)
		.def("getTrackerTrigger", &vl::EventManager::getTrackerTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def("hasTrackerTrigger", &vl::EventManager::hasTrackerTrigger )
		.def("getKeyPressedTrigger", &vl::EventManager::getKeyPressedTrigger,
			 getKeyPressedTrigger_ov()[python::return_value_policy<python::reference_existing_object>()] )
		.def("createKeyPressedTrigger", &vl::EventManager::createKeyPressedTrigger,
			 createKeyPressedTrigger_ov()[python::return_value_policy<python::reference_existing_object>()] )
		.def("hasKeyPressedTrigger", &vl::EventManager::hasKeyPressedTrigger, hasKeyPressedTrigger_ov() )
		.def("getKeyReleasedTrigger", &vl::EventManager::getKeyReleasedTrigger,
			 getKeyReleasedTrigger_ov()[python::return_value_policy<python::reference_existing_object>()] )
		.def("createKeyReleasedTrigger", &vl::EventManager::createKeyReleasedTrigger,
			 createKeyReleasedTrigger_ov()[python::return_value_policy<python::reference_existing_object>()] )
		.def("hasKeyReleasedTrigger", &vl::EventManager::hasKeyReleasedTrigger, hasKeyReleasedTrigger_ov() )
		.def("getFrameTrigger", &vl::EventManager::getFrameTrigger, python::return_value_policy<python::reference_existing_object>() )
		.def(python::self_ns::str(python::self_ns::self))
	;

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
		.def("getAction", python::make_function( &BasicActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ) )
		.def("addAction", &BasicActionTrigger::addAction )
		.def("getNActions", &BasicActionTrigger::getNActions )
	;

	python::class_<TransformActionTrigger, boost::noncopyable, python::bases<Trigger> >("TransformActionTrigger", python::no_init )
		.add_property("action", python::make_function( &TransformActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ), &TransformActionTrigger::setAction)
	;

	python::class_<FrameTrigger, boost::noncopyable, python::bases<BasicActionTrigger> >("FrameTrigger", python::no_init )
	;

	python::class_<KeyTrigger, boost::noncopyable, python::bases<BasicActionTrigger> >("KeyTrigger", python::no_init )
		.add_property("key", &KeyTrigger::getKey, &KeyTrigger::setKey )
	;

	python::class_<KeyPressedTrigger, boost::noncopyable, python::bases<KeyTrigger> >("KeyPressedTrigger", python::no_init )
	;

	python::class_<KeyReleasedTrigger, boost::noncopyable, python::bases<KeyTrigger> >("KeyReleasedTrigger", python::no_init )
	;

	// TODO replace with a wrapper
	python::class_<Action, boost::noncopyable>("Action", python::no_init )
		.add_property("type", &Action::getTypeName)
		.def(python::self_ns::str(python::self_ns::self))
	;

	// TODO needs a wrapper
	python::class_<BasicAction, boost::noncopyable, python::bases<Action> >("BasicAction", python::no_init )
	;

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

	python::class_<TimerActionProxy, boost::noncopyable, python::bases<BasicAction> >("TimerActionProxy", python::no_init )
		.add_property("action", python::make_function( &TimerActionProxy::getAction, python::return_value_policy< python::reference_existing_object>() ), &TimerActionProxy::setAction )
		.add_property("time_limit", &TimerActionProxy::getTimeLimit, &TimerActionProxy::setTimeLimit )
		.def("create",&TimerActionProxy::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;



	python::class_<vl::TransformAction, boost::noncopyable, python::bases<Action> >("TransformAction", python::no_init )
	;

	python::class_<vl::SetTransformation, boost::noncopyable, python::bases<vl::TransformAction> >("SetTransformation", python::no_init )
		.add_property("scene_node", python::make_function( &vl::SetTransformation::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &vl::SetTransformation::setSceneNode )
		.def("create",&SetTransformation::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::TrackerTrigger, boost::noncopyable, python::bases<vl::TransformActionTrigger> >("TrackerTrigger", python::no_init )
	;


	python::class_<GameAction, boost::noncopyable, python::bases<BasicAction> >("GameAction", python::no_init )
		.def_readwrite("game", &GameAction::data )
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

	python::class_<AddToSelection, boost::noncopyable, python::bases<SceneManagerAction> >("AddToSelection", python::no_init )
		.add_property("scene_node", python::make_function( &AddToSelection::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &AddToSelection::setSceneNode )		
		.def("create",&AddToSelection::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<RemoveFromSelection, boost::noncopyable, python::bases<SceneManagerAction> >("RemoveFromSelection", python::no_init )
		.add_property("scene_node", python::make_function( &RemoveFromSelection::getSceneNode, python::return_value_policy< python::reference_existing_object>() ), &RemoveFromSelection::setSceneNode )		
		.def("create",&RemoveFromSelection::create, python::return_value_policy<python::reference_existing_object>() )
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
		.add_property("speed", &MoveAction::getSpeed, &MoveAction::setSpeed )
		.add_property("angular_speed", python::make_function( &MoveAction::getAngularSpeed, python::return_internal_reference<>() ), &MoveAction::setAngularSpeed )
		.add_property("local", &MoveAction::getLocal, &MoveAction::setLocal )
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

	/// GUI actions
	python::class_<vl::gui::GUI, boost::noncopyable>("GUI", python::no_init )
		.def("showEditor", &vl::gui::GUI::showEditor )
		.def("hideEditor", &vl::gui::GUI::hideEditor )
		.def("showConsole", &vl::gui::GUI::showConsole )
		.def("hideConsole", &vl::gui::GUI::hideConsole )
		.def("showStats", &vl::gui::GUI::showStats )
		.def("hideStats", &vl::gui::GUI::hideStats )
		.def("showLoadingScreen", &vl::gui::GUI::showLoadingScreen )
		.def("hideLoadingScreen", &vl::gui::GUI::hideLoadingScreen )
	;

	python::class_<vl::gui::GUIActionBase, boost::noncopyable>("GUIActionBase", python::no_init )
		.add_property("gui", python::make_function( &vl::gui::GUIActionBase::getGUI, python::return_value_policy< python::reference_existing_object>() ), &vl::gui::GUIActionBase::setGUI )
	;

	python::class_<vl::gui::HideEditor, boost::noncopyable, python::bases<vl::gui::GUIActionBase, BasicAction> >("HideEditor", python::no_init )
		.def("create", &vl::gui::HideEditor::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::gui::ShowEditor, boost::noncopyable, python::bases<vl::gui::GUIActionBase, BasicAction> >("ShowEditor", python::no_init )
		.def("create",&vl::gui::ShowEditor::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::gui::HideConsole, boost::noncopyable, python::bases<vl::gui::GUIActionBase, BasicAction> >("HideConsole", python::no_init )
		.def("create",&vl::gui::HideConsole::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::gui::ShowConsole, boost::noncopyable, python::bases<vl::gui::GUIActionBase, BasicAction> >("ShowConsole", python::no_init )
		.def("create",&vl::gui::ShowConsole::create, python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
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

#endif	// VL_PYTHON_MODULE_HPP
