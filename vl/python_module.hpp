/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file python_module.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

#ifndef VL_PYTHON_MODULE_HPP
#define VL_PYTHON_MODULE_HPP

// Game
#include "game_manager.hpp"

// SceneGraph
#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "player.hpp"

// Necessary for transforming OIS keycodes to python
#include "keycode.hpp"

// Event handling
#include "event_manager.hpp"
#include "actions_transform.hpp"
#include "actions_misc.hpp"
#include "trigger.hpp"

// GUI
#include "gui/gui.hpp"
#include "gui/gui_actions.hpp"

// Python global
#include "python.hpp"

// Physics
#include "physics/physics_events.hpp"
#include "physics/physics_world.hpp"
#include "physics/rigid_body.hpp"
#include "physics/shapes.hpp"
#include "physics/constraints.hpp"

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

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createKeyTrigger_ov, createKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getKeyTrigger_ov, getKeyTrigger, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hasKeyTrigger_ov, hasKeyTrigger, 1, 2)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setSpotlightRange_ov, setSpotlightRange, 2, 3)

/// Overloads need to be outside the module definition
/// Physics world member overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createRigidBody_ov, createRigidBody, 4, 5 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( addConstraint_ovs, addConstraint, 1, 2 )

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( setLimit_ovs, setLimit, 2, 5 )

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createMotionState_ov, createMotionState, 0, 2 )

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( hideSceneNodes_ov, hideSceneNodes, 1, 2 )

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createPlane_ovs, createPlane, 3, 6)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createCube_ovs, createCube, 1, 2)

BOOST_PYTHON_MODULE(vl)
{
	using namespace vl;

	python::class_<vl::Transform>("Transform", python::init<python::optional<Ogre::Vector3, Ogre::Quaternion> >() )
		.def(python::init<Ogre::Quaternion>())
		.def("isIdentity", &vl::Transform::isIdentity)
		.def("setIdentity", &vl::Transform::setIdentity)
		.def("isPositionZero", &vl::Transform::isPositionZero)
		.def("setPositionZero", &vl::Transform::setPositionZero)
		.def("isRotationIdentity", &vl::Transform::isRotationIdentity)
		.def("setRotationIdentity", &vl::Transform::setRotationIdentity)
		.def("invert", &vl::Transform::invert)
		.def_readwrite("position", &vl::Transform::position)
		.def_readwrite("quaternion", &vl::Transform::quaternion)
		.def(python::self *= python::self)
		.def(python::self * python::self)
		.def(python::self == python::self)
		.def(python::self != python::self)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::GameManager, boost::noncopyable>("GameManager", python::no_init)
		.add_property("scene", python::make_function( &vl::GameManager::getSceneManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("player", python::make_function( &vl::GameManager::getPlayer, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("event_manager", python::make_function( &vl::GameManager::getEventManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("gui", &vl::GameManager::getGUI)
		.add_property("stats", python::make_function( &vl::GameManager::getStats, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property( "physics_world", &vl::GameManager::getPhysicsWorld)
		.def( "enableAudio", &vl::GameManager::enableAudio )
		.def( "enablePhysics", &vl::GameManager::enablePhysics )
		.add_property("logger", python::make_function( &vl::GameManager::getLogger, python::return_value_policy<python::reference_existing_object>() ) )
		.def("createBackgroundSound", &vl::GameManager::createBackgroundSound)
		.def("toggleBackgroundSound", &vl::GameManager::toggleBackgroundSound )
		.def("quit", &vl::GameManager::quit)
		.add_property("tracker_clients", &vl::GameManager::getTrackerClients)
		.add_property("mesh_manager", &vl::GameManager::getMeshManager)
	;

	void (sink::*write1)( std::string const & ) = &sink::write;

	python::class_<vl::sink>("sink", python::no_init)
		.def("write", write1 )
	;

	// TODO add setHeadMatrix function to python
	python::class_<vl::Player, boost::noncopyable>("Player", python::no_init)
		.add_property("camera", python::make_function( &Player::getActiveCamera , python::return_value_policy<python::copy_const_reference>() ), &Player::setActiveCamera )
		.add_property("ipd", &Player::getIPD, &Player::setIPD)
		.def("takeScreenshot", &vl::Player::takeScreenshot )
	;

	python::class_<vl::Stats, boost::noncopyable>("Stats", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::MeshRefPtr (MeshManager::*loadMesh_ov0)(std::string const &) = &MeshManager::loadMesh;

	python::class_<vl::MeshManager, vl::MeshManagerRefPtr, boost::noncopyable>("MeshManager", python::no_init)
		.def("loadMesh", loadMesh_ov0)
		.def("createPlane", &vl::MeshManager::createPlane, createPlane_ovs())
		.def("createSphere", &vl::MeshManager::createSphere)
		.def("createCube", &vl::MeshManager::createCube, createCube_ovs())
		.def("getMesh", &vl::MeshManager::getMesh)
		.def("hasMesh", &vl::MeshManager::hasMesh)
		.def("cleanup_unused", &vl::MeshManager::cleanup_unused)
	;

	python::class_<vl::FogInfo>("FogInfo", python::init<>())
		.def(python::init<std::string, python::optional<Ogre::ColourValue, Ogre::Real, Ogre::Real, Ogre::Real> >())
		.add_property("mode", &vl::FogInfo::setMode, &vl::FogInfo::getMode)
		.def_readwrite("colour", &vl::FogInfo::colour_diffuse)
		.def_readwrite("density", &vl::FogInfo::exp_density)
		.def_readwrite("linear_start", &vl::FogInfo::linear_start)
		.def_readwrite("linear_end", &vl::FogInfo::linear_end)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::SkyDomeInfo>("SkyDomeInfo", python::init< python::optional<std::string> >())
		.def_readwrite("material_name", &vl::SkyDomeInfo::material_name)
		.def_readwrite("curvature", &vl::SkyDomeInfo::curvature)
		.def_readwrite("tiling", &vl::SkyDomeInfo::tiling)
		.def_readwrite("distance", &vl::SkyDomeInfo::distance)
		.def_readwrite("draw_first", &vl::SkyDomeInfo::draw_first)
		.def_readwrite("orientation", &vl::SkyDomeInfo::orientation)
		.def_readwrite("xsegments", &vl::SkyDomeInfo::xsegments)
		.def_readwrite("ysegments", &vl::SkyDomeInfo::ysegments)
		.def_readwrite("ysegments_keep", &vl::SkyDomeInfo::ysegments_keep)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	void (vl::ShadowInfo::*setShadowTechnique_ov0)(std::string const &) = &vl::ShadowInfo::setShadowTechnique;

	python::class_<vl::ShadowInfo>("ShadowInfo",  python::init< python::optional<std::string, Ogre::ColourValue, std::string> >())
		.def("enable", &vl::ShadowInfo::enable)
		.def("disable", &vl::ShadowInfo::disable)
		.def("isEnabled", &vl::ShadowInfo::isEnabled)
		.add_property("technique", &vl::ShadowInfo::getShadowTechniqueName, setShadowTechnique_ov0)
		.add_property("colour", python::make_function(&vl::ShadowInfo::getColour, python::return_value_policy<python::copy_const_reference>()), &vl::ShadowInfo::setColour)
		.add_property("camera", python::make_function(&vl::ShadowInfo::getCamera, python::return_value_policy<python::copy_const_reference>()), &vl::ShadowInfo::setCamera)
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::EntityPtr (SceneManager::*createEntity_ov0)(std::string const &, vl::PREFAB) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov1)(std::string const &, std::string const &) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov2)(std::string const &, std::string const &, bool) = &SceneManager::createEntity;
	vl::ShadowInfo &(SceneManager::*getShadowInfo_ov0)(void) = &vl::SceneManager::getShadowInfo;
	
	python::class_<vl::SceneManager, boost::noncopyable>("SceneManager", python::no_init)
		// TODO add remove SceneNodes
		.add_property("root", python::make_function( &SceneManager::getRootSceneNode, python::return_value_policy<python::reference_existing_object>() ) )
		.def("createSceneNode", &SceneManager::createSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("hasSceneNode", &SceneManager::hasSceneNode )
		.def("getSceneNode", &SceneManager::getSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("hasEntity", &SceneManager::hasEntity )
		.def("createEntity", createEntity_ov0, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov1, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov2, python::return_value_policy<python::reference_existing_object>() )
		.def("getEntity", &SceneManager::getEntity, python::return_value_policy<python::reference_existing_object>() )
		.def("hasCamera", &SceneManager::hasCamera)
		.def("createCamera", &SceneManager::createCamera, python::return_value_policy<python::reference_existing_object>() )
		.def("getCamera", &SceneManager::getCamera, python::return_value_policy<python::reference_existing_object>() )
		.def("hasLight", &SceneManager::hasLight)
		.def("createLight", &SceneManager::createLight, python::return_value_policy<python::reference_existing_object>() )
		.def("getLight", &SceneManager::getLight, python::return_value_policy<python::reference_existing_object>() )
		
		/// Scene parameters
		/// returns copies of the objects
		.add_property("sky", python::make_function( &vl::SceneManager::getSkyDome, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setSkyDome )
		.add_property("fog", python::make_function( &vl::SceneManager::getFog, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setFog )
		.add_property("ambient_light", python::make_function( &vl::SceneManager::getAmbientLight, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setAmbientLight )
		.add_property("shadows", python::make_function(getShadowInfo_ov0, python::return_internal_reference<>()), &vl::SceneManager::setShadowInfo)

		/// Selection
		.def("addToSelection", &SceneManager::addToSelection)
		.def("removeFromSelection", &SceneManager::removeFromSelection)
		.def("clearSelection", &SceneManager::clearSelection)

		.def("hideSceneNodes", &vl::SceneManager::hideSceneNodes, hideSceneNodes_ov())
		.def("reloadScene", &SceneManager::reloadScene)
		/// @todo add printing
	;

	python::enum_<vl::PREFAB>("PF")
		.value("NONE", PF_NONE)
		.value("PLANE", PF_PLANE)
		.value("SPHERE", PF_SPHERE)
		.value("CUBE", PF_CUBE)
	;

	python::class_<vl::MovableObject, boost::noncopyable>("MovableObject", python::no_init)
		.add_property("name", python::make_function( &vl::MovableObject::getName, python::return_value_policy<python::copy_const_reference>() ) )
		// @todo does not work, pure virtual
		//.add_property("type", python::make_function( &vl::Camera::getTypeName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("parent", python::make_function( &vl::MovableObject::getParent,python::return_value_policy<python::reference_existing_object>() ) )
	;

	python::class_<vl::LightAttenuation>("LightAttenuation", python::init<>() )
		.def(python::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def_readwrite("range", &vl::LightAttenuation::range)
		.def_readwrite("constant", &vl::LightAttenuation::constant)
		.def_readwrite("linear", &vl::LightAttenuation::linear)
		.def_readwrite("quadratic", &vl::LightAttenuation::quadratic)
		.def(python::self_ns::str(python::self_ns::self))
	;

	void (vl::Light::*setType_ov1)(std::string const &type) = &vl::Light::setType;

	python::class_<vl::Light, boost::noncopyable, python::bases<vl::MovableObject> >("Light", python::no_init)
		.add_property("type", &vl::Light::getLightTypeName, setType_ov1 )
		.add_property("diffuse", python::make_function( &vl::Light::getDiffuseColour, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setDiffuseColour )
		.add_property("specular", python::make_function( &vl::Light::getSpecularColour, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setSpecularColour )
		.add_property("direction", python::make_function( &vl::Light::getDirection, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setDirection )
		.add_property("position", python::make_function( &vl::Light::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setPosition )
		.add_property("visible", &vl::Light::getVisible, &vl::Light::setVisible )
		.add_property("cast_shadows", &vl::Light::getCastShadows, &vl::Light::setCastShadows )
		// TODO this should use a struct and be a property
		.def("setSpotRange", &vl::Light::setSpotlightRange, setSpotlightRange_ov())
		.add_property("attenuation", python::make_function( &vl::Light::getAttenuation, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setAttenuation )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Camera, boost::noncopyable, python::bases<vl::MovableObject> >("Camera", python::no_init)
		.add_property("near_clip", &vl::Camera::getNearClipDistance, &vl::Camera::setNearClipDistance )
		.add_property("far_clip", &vl::Camera::getFarClipDistance, &vl::Camera::setFarClipDistance )
		.add_property("position", python::make_function( &vl::Camera::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::Camera::setPosition )
		.add_property("orientation", python::make_function( &vl::Camera::getOrientation, python::return_value_policy<python::copy_const_reference>() ), &vl::Camera::setOrientation )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Entity, boost::noncopyable, python::bases<vl::MovableObject> >("Entity", python::no_init)
		.add_property("material_name", python::make_function( &vl::Entity::getMaterialName, python::return_value_policy<python::copy_const_reference>() ), &vl::Entity::setMaterialName )
		.add_property("cast_shadows", &vl::Entity::getCastShadows, &vl::Entity::setCastShadows )
		.add_property("mesh_name", python::make_function( &vl::Entity::getMeshName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("prefab", &vl::Entity::getPrefab)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::enum_<TransformSpace>("TS")
		.value("LOCAL", TS_LOCAL)
		.value("PARENT", TS_PARENT)
		.value("WORLD", TS_WORLD)
	;

	void (vl::SceneNode::*setTransform_ov0)(vl::Transform const &) = &vl::SceneNode::setTransform;
	void (vl::SceneNode::*setTransform_ov1)(vl::Transform const &, vl::SceneNodePtr) = &vl::SceneNode::setTransform;
	vl::Transform const &(vl::SceneNode::*getTransform_ov0)(void) const = &vl::SceneNode::getTransform;
	vl::Transform (vl::SceneNode::*getTransform_ov1)(vl::SceneNodePtr) const = &vl::SceneNode::getTransform;
	void (vl::SceneNode::*translate_ov0)(Ogre::Vector3 const &) = &vl::SceneNode::translate;
	void (vl::SceneNode::*translate_ov1)(Ogre::Vector3 const &, vl::SceneNodePtr) = &vl::SceneNode::translate;
	void (vl::SceneNode::*translate_ov2)(Ogre::Vector3 const &, TransformSpace) = &vl::SceneNode::translate;
	void (vl::SceneNode::*rotate_ov0)(Ogre::Quaternion const &) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov1)(Ogre::Quaternion const &, vl::SceneNodePtr) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov2)(Ogre::Quaternion const &, TransformSpace) = &vl::SceneNode::rotate;
	SceneNodePtr (vl::SceneNode::*sn_clone_ov0)() const = &vl::SceneNode::clone;
	SceneNodePtr (vl::SceneNode::*sn_clone_ov1)(std::string const &) const = &vl::SceneNode::clone;

	python::class_<vl::SceneNode, boost::noncopyable>("SceneNode", python::no_init)
		.def("attachObject", &vl::SceneNode::attachObject)
		.def("detachObject", &vl::SceneNode::detachObject)
		.def("hasObject", &vl::SceneNode::hasObject)
		.def("createChildSceneNode", &vl::SceneNode::createChildSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("addChild", &vl::SceneNode::addChild)
		.def("removeChild", &vl::SceneNode::removeChild)
		.def("translate", translate_ov0)
		.def("translate", translate_ov1)
		.def("translate", translate_ov2)
		.def("rotate", rotate_ov0)
		.def("rotate", rotate_ov1)
		.def("rotate", rotate_ov2)
		.def("clone", sn_clone_ov0, python::return_value_policy<python::reference_existing_object>())
		.def("clone", sn_clone_ov1, python::return_value_policy<python::reference_existing_object>())
		.add_property("name", python::make_function( &vl::SceneNode::getName, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setName )
		.add_property("transform", python::make_function( getTransform_ov0, python::return_value_policy<python::copy_const_reference>() ), setTransform_ov0)
		.add_property("world_transform", &vl::SceneNode::getWorldTransform, &vl::SceneNode::setWorldTransform)
		.add_property("position", python::make_function( &vl::SceneNode::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setPosition )
		.add_property("orientation", python::make_function( &vl::SceneNode::getOrientation, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setOrientation )
		.add_property("scale", python::make_function( &vl::SceneNode::getScale, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setScale )
		.add_property("visible", &SceneNode::getVisible, &vl::SceneNode::setVisible)
		.add_property("parent", python::make_function(&vl::SceneNode::getParent, python::return_value_policy<python::reference_existing_object>()) )
		.def("hide", &vl::SceneNode::hide)
		.def("isHidden", &vl::SceneNode::isHidden)
		.def("show", &vl::SceneNode::show)
		.def("isShown", &vl::SceneNode::isShown)
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
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	python::class_<vl::Sensor, boost::noncopyable>("Sensor", python::no_init)
		.add_property("trigger", python::make_function(&vl::Sensor::getTrigger, python::return_value_policy<python::reference_existing_object>() ), &vl::Sensor::setTrigger)
		.add_property("transform", python::make_function(&vl::Sensor::getCurrentTransform, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("default_transform", python::make_function(&vl::Sensor::getDefaultTransform, python::return_value_policy<python::copy_const_reference>() ), &vl::Sensor::setDefaultTransform)
		.def(python::self_ns::str(python::self_ns::self))
	;
	
	Sensor & (vl::Tracker::*getSensor_ov0)(size_t) = &vl::Tracker::getSensor;
	
	python::class_<vl::Tracker, TrackerRefPtr, boost::noncopyable>("Tracker", python::no_init)
		.def("setSensor", &vl::Tracker::setSensor)
		.def("addSensor", &vl::Tracker::addSensor)
		.def("getSensor", python::make_function(getSensor_ov0, python::return_value_policy<python::reference_existing_object>()) )
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
	;

	python::class_<TransformActionTrigger, boost::noncopyable, python::bases<Trigger> >("TransformActionTrigger", python::no_init )
		.add_property("action", python::make_function( &TransformActionTrigger::getAction, python::return_value_policy< python::reference_existing_object>() ), &TransformActionTrigger::setAction)
	;
	
	python::class_<vl::TrackerTrigger, python::bases<vl::TransformActionTrigger>, boost::noncopyable>("TrackerTrigger", python::no_init)
		.add_property("name", &vl::TrackerTrigger::getName, &vl::TrackerTrigger::setName)
	;
	
	python::class_<FrameTrigger, boost::noncopyable, python::bases<BasicActionTrigger> >("FrameTrigger", python::no_init )
	;

	python::class_<vl::KeyTrigger, boost::noncopyable, python::bases<Trigger> >("KeyTrigger", python::no_init )
		.add_property("key", &vl::KeyTrigger::getKey, &vl::KeyTrigger::setKey )
		.add_property("modifiers", &vl::KeyTrigger::getModifiers, &vl::KeyTrigger::setModifiers )
		.add_property("action_down", python::make_function( &vl::KeyTrigger::getKeyDownAction, python::return_value_policy< python::reference_existing_object>() ), &vl::KeyTrigger::setKeyDownAction)
		.add_property("action_up", python::make_function( &vl::KeyTrigger::getKeyUpAction, python::return_value_policy< python::reference_existing_object>() ), &vl::KeyTrigger::setKeyUpAction)
	;


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

	/// GUI actions
	python::class_<vl::gui::GUI, vl::gui::GUIRefPtr, boost::noncopyable>("GUI", python::no_init )
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
		.add_property("gui", &vl::gui::GUIActionBase::getGUI, &vl::gui::GUIActionBase::setGUI )
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

	/// Physics
	/// @todo physics should be in differen module

	python::class_<vl::physics::CollisionShape, boost::noncopyable >("CollisionShape", python::no_init )
	;

	python::class_<vl::physics::BoxShape, boost::noncopyable, vl::physics::BoxShapeRefPtr, python::bases<vl::physics::CollisionShape> >("BoxShape", python::no_init )
		.def("create", &vl::physics::BoxShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::SphereShape, boost::noncopyable, vl::physics::SphereShapeRefPtr, python::bases<vl::physics::CollisionShape> >("SphereShape", python::no_init )
		.def("create", &vl::physics::SphereShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::StaticPlaneShape, boost::noncopyable, vl::physics::StaticPlaneShapeRefPtr, python::bases<vl::physics::CollisionShape> >("StaticPlaneShape", python::no_init )
		.def("create", &vl::physics::StaticPlaneShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::StaticTriangleMeshShape, boost::noncopyable, vl::physics::StaticTriangleMeshShapeRefPtr, python::bases<vl::physics::CollisionShape> >("StaticTriangleMeshShape", python::no_init )
		.def("create", &vl::physics::StaticTriangleMeshShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::ConvexHullShape, boost::noncopyable, vl::physics::ConvexHullShapeRefPtr, python::bases<vl::physics::CollisionShape> >("ConvexHullShape", python::no_init )
		.add_property("scale", &vl::physics::ConvexHullShape::getLocalScaling, &vl::physics::ConvexHullShape::setLocalScaling)
		.def("create", &vl::physics::ConvexHullShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::CylinderShape, boost::noncopyable, vl::physics::CylinderShapeRefPtr, python::bases<vl::physics::CollisionShape> >("CylinderShape", python::no_init )
	;

	python::class_<vl::physics::ConeShape, boost::noncopyable, vl::physics::ConeShapeRefPtr, python::bases<vl::physics::CollisionShape> >("ConeShape", python::no_init )
	;

	python::class_<vl::physics::CapsuleShape, boost::noncopyable, vl::physics::CapsuleShapeRefPtr, python::bases<vl::physics::CollisionShape> >("CapsuleShape", python::no_init )
	;

	/// Abstract master class for all constraints
	python::class_<vl::physics::Constraint, vl::physics::ConstraintRefPtr, boost::noncopyable>("Constraint", python::no_init)
	;

	/// 6dof constraint
	python::class_<vl::physics::SixDofConstraint, vl::physics::SixDofConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("SixDofConstraint", python::no_init)
		/// @todo change to properties
		.def("setLinearLowerLimit", &vl::physics::SixDofConstraint::setLinearLowerLimit)
		.def("setLinearUpperLimit", &vl::physics::SixDofConstraint::setLinearUpperLimit)
		.def("setAngularLowerLimit", &vl::physics::SixDofConstraint::setAngularLowerLimit)
		.def("setAngularUpperLimit", &vl::physics::SixDofConstraint::setAngularUpperLimit)
		.add_property("bodyA", &vl::physics::SixDofConstraint::getBodyA)
		.add_property("bodyB", &vl::physics::SixDofConstraint::getBodyB)
		.def("create", &vl::physics::SixDofConstraint::create)
		.staticmethod("create")
	;

	/// slider constraint
	python::class_<vl::physics::SliderConstraint, vl::physics::SliderConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("SliderConstraint", python::no_init)
		// Limits
		.add_property("lower_lin_limit", &vl::physics::SliderConstraint::getLowerLinLimit, &vl::physics::SliderConstraint::setLowerLinLimit)
		.add_property("upper_lin_limit", &vl::physics::SliderConstraint::getUpperLinLimit, &vl::physics::SliderConstraint::setUpperLinLimit)
		.add_property("lower_ang_limit", &vl::physics::SliderConstraint::getLowerAngLimit, &vl::physics::SliderConstraint::setLowerAngLimit)
		.add_property("upper_ang_limit", &vl::physics::SliderConstraint::getUpperAngLimit, &vl::physics::SliderConstraint::setUpperAngLimit)
		// Motor
		.add_property("powered_lin_motor", &vl::physics::SliderConstraint::getPoweredLinMotor, &vl::physics::SliderConstraint::setPoweredLinMotor)
		.add_property("target_lin_motor_velocity", &vl::physics::SliderConstraint::getTargetLinMotorVelocity, &vl::physics::SliderConstraint::setTargetLinMotorVelocity)
		.add_property("max_lin_motor_force", &vl::physics::SliderConstraint::getMaxLinMotorForce, &vl::physics::SliderConstraint::setMaxLinMotorForce)
		.add_property("powered_ang_motor", &vl::physics::SliderConstraint::getPoweredAngMotor, &vl::physics::SliderConstraint::setPoweredAngMotor)
		.add_property("target_ang_motor_velocity", &vl::physics::SliderConstraint::getTargetAngMotorVelocity, &vl::physics::SliderConstraint::setTargetAngMotorVelocity)
		.add_property("max_ang_motor_force", &vl::physics::SliderConstraint::getMaxAngMotorForce, &vl::physics::SliderConstraint::setMaxAngMotorForce)
		.def("create", &vl::physics::SliderConstraint::create)
		.staticmethod("create")
	;

	/// hinge constraint
	python::class_<vl::physics::HingeConstraint, vl::physics::HingeConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("HingeConstraint", python::no_init)
		.def("create", &vl::physics::HingeConstraint::create)
		.staticmethod("create")
	;

	/// rigid body
	vl::physics::MotionState *(vl::physics::RigidBody::*getMotionState_ov1)(void) = &vl::physics::RigidBody::getMotionState;
	void (vl::physics::RigidBody::*applyForce_ov0)(Ogre::Vector3 const &, Ogre::Vector3 const &) = &vl::physics::RigidBody::applyForce;
	void (vl::physics::RigidBody::*applyForce_ov1)(Ogre::Vector3 const &, Ogre::Vector3 const &, bool) = &vl::physics::RigidBody::applyForce;
	void (vl::physics::RigidBody::*applyForce_ov2)(Ogre::Vector3 const &, Ogre::Vector3 const &, vl::physics::RigidBodyRefPtr) = &vl::physics::RigidBody::applyForce;
	void (vl::physics::RigidBody::*applyCentralForce_ov0)(Ogre::Vector3 const &) = &vl::physics::RigidBody::applyCentralForce;
	void (vl::physics::RigidBody::*applyCentralForce_ov1)(Ogre::Vector3 const &, bool) = &vl::physics::RigidBody::applyCentralForce;
	void (vl::physics::RigidBody::*applyCentralForce_ov2)(Ogre::Vector3 const &, vl::physics::RigidBodyRefPtr) = &vl::physics::RigidBody::applyCentralForce;

	python::class_<vl::physics::RigidBody, vl::physics::RigidBodyRefPtr, boost::noncopyable>("RigidBody", python::no_init )
		.def( "applyForce", applyForce_ov0)
		.def( "applyForce", applyForce_ov1)
		.def( "applyForce", applyForce_ov2)
		.def( "applyTorque", &vl::physics::RigidBody::applyTorque )
		.def( "applyTorqueImpulse", &vl::physics::RigidBody::applyTorqueImpulse )
		.def( "applyCentralForce", applyCentralForce_ov0)
		.def( "applyCentralForce", applyCentralForce_ov1)
		.def( "applyCentralForce", applyCentralForce_ov2)
		.def( "applyCentralImpulse", &vl::physics::RigidBody::applyCentralImpulse )
		.def( "setAngularVelocity", &vl::physics::RigidBody::setAngularVelocity )
		.def( "setLinearVelocity", &vl::physics::RigidBody::setLinearVelocity )
		.def( "setDamping", &vl::physics::RigidBody::setDamping )
		.def( "getInvMass", &vl::physics::RigidBody::getInvMass )
		.def( "clearForces", &vl::physics::RigidBody::clearForces )
		.def("setInertia", &vl::physics::RigidBody::setInertia)
		.def("setMassProps", &vl::physics::RigidBody::setMassProps)
		.def("transform_to_local", &vl::physics::RigidBody::transformToLocal)
		.def("translate", &vl::physics::RigidBody::translate)
		.add_property("total_force", &vl::physics::RigidBody::getTotalForce )
		.add_property("total_torque", &vl::physics::RigidBody::getTotalTorque )
		.add_property("center_of_mass_transform", &vl::physics::RigidBody::getCenterOfMassTransform, &vl::physics::RigidBody::setCenterOfMassTransform)
		.add_property("linear_damping", &vl::physics::RigidBody::getLinearDamping, &vl::physics::RigidBody::setLinearDamping)
		.add_property("angular_damping", &vl::physics::RigidBody::getAngularDamping, &vl::physics::RigidBody::setAngularDamping)
		.add_property("linear_velocity", &vl::physics::RigidBody::getLinearVelocity, &vl::physics::RigidBody::setLinearVelocity)
		.add_property("angular_velocity", &vl::physics::RigidBody::getAngularVelocity, &vl::physics::RigidBody::setAngularVelocity)
		.add_property("world_transform", &vl::physics::RigidBody::getWorldTransform)
		.add_property("shape", &vl::physics::RigidBody::getShape)
		.add_property("user_controlled", &vl::physics::RigidBody::isUserControlled, &vl::physics::RigidBody::setUserControlled)
		.add_property("motion_state", python::make_function(getMotionState_ov1, python::return_value_policy<python::reference_existing_object>()), &vl::physics::RigidBody::setMotionState )
		.add_property("name", python::make_function(&vl::physics::RigidBody::getName, python::return_value_policy<python::copy_const_reference>()) )
		.add_property("mass", &vl::physics::RigidBody::getMass, &vl::physics::RigidBody::setMass)
		.def(python::self_ns::str(python::self_ns::self))
	;

	Transform (vl::physics::MotionState::*getWorldTransform_ov0)(void) const = &vl::physics::MotionState::getWorldTransform;

	/// motion state
	python::class_<vl::physics::MotionState, boost::noncopyable>("MotionState", python::no_init)
		.add_property("node", python::make_function( &vl::physics::MotionState::getNode, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::MotionState::setNode )
		.add_property("position", &vl::physics::MotionState::getPosition, &vl::physics::MotionState::setPosition)
		.add_property("orientation", &vl::physics::MotionState::getOrientation, &vl::physics::MotionState::setOrientation)
		.add_property("world_transform", getWorldTransform_ov0)
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// world
	python::class_<vl::physics::World, vl::physics::WorldRefPtr, boost::noncopyable>("PhysicsWorld", python::no_init)
		.def("createRigidBody", &vl::physics::World::createRigidBody, createRigidBody_ov() )
		.def("getRigidBody", &vl::physics::World::getRigidBody)
		.def("hasRigidBody", &vl::physics::World::hasRigidBody)
		.def("removeRigidBody", &vl::physics::World::removeRigidBody)
		.def("createMotionState", &vl::physics::World::createMotionState,
			 createMotionState_ov()[ python::return_value_policy<python::reference_existing_object>() ] )
		.def("addConstraint", &vl::physics::World::addConstraint, addConstraint_ovs() )
		.add_property("gravity", &vl::physics::World::getGravity, &vl::physics::World::setGravity )
		.def(python::self_ns::str(python::self_ns::self))
	;



	/// Physics Actions
	python::class_<vl::physics::SliderMotorAction, boost::noncopyable, python::bases<vl::BasicAction> >("SliderMotorAction", python::no_init )
		.def_readwrite("velocity", &vl::physics::SliderMotorAction::velocity)
		.def_readwrite("constraint", &vl::physics::SliderMotorAction::constraint)
		.def("create",&vl::physics::SliderMotorAction::create,
			 python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::physics::KinematicAction, boost::noncopyable, python::bases<vl::MoveAction> >("KinematicAction", python::no_init )
		.add_property("body", python::make_function( &vl::physics::KinematicAction::getRigidBody, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::KinematicAction::setRigidBody )
		.def("create",&vl::physics::KinematicAction::create,
			 python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::physics::DynamicAction, boost::noncopyable, python::bases<vl::MoveAction> >("DynamicAction", python::no_init )
		.add_property("body", python::make_function( &vl::physics::DynamicAction::getRigidBody, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::DynamicAction::setRigidBody )
		.add_property("force", python::make_function( &vl::physics::DynamicAction::getForce, python::return_value_policy<python::copy_const_reference>() ),
					  &vl::physics::DynamicAction::setForce )
		.add_property("torque", python::make_function( &vl::physics::DynamicAction::getTorque, python::return_value_policy<python::copy_const_reference>() ),
					  &vl::physics::DynamicAction::setTorque )
		.add_property("max_speed", &vl::physics::DynamicAction::getSpeed, &vl::physics::DynamicAction::setSpeed)
		.def("create",&vl::physics::DynamicAction::create,
			 python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::physics::ApplyForce, boost::noncopyable, python::bases<BasicAction> >("ApplyForce", python::no_init )
		.add_property("body", python::make_function( &vl::physics::ApplyForce::getRigidBody, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::ApplyForce::setRigidBody )
		.add_property("force", python::make_function( &vl::physics::ApplyForce::getForce, python::return_value_policy<python::copy_const_reference>() ),
					  &vl::physics::ApplyForce::setForce )
		.add_property("local", &vl::physics::ApplyForce::getLocal, &vl::physics::ApplyForce::setLocal)
		.def("create",&vl::physics::ApplyForce::create,
			 python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::physics::ApplyTorque, boost::noncopyable, python::bases<BasicAction> >("ApplyTorque", python::no_init )
		.add_property("body", python::make_function( &vl::physics::ApplyTorque::getRigidBody, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::ApplyTorque::setRigidBody )
		.add_property("torque", python::make_function( &vl::physics::ApplyTorque::getTorque, python::return_value_policy<python::copy_const_reference>() ),
					  &vl::physics::ApplyTorque::setTorque )
		.def("create",&vl::physics::ApplyTorque::create,
			 python::return_value_policy<python::reference_existing_object>() )
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
