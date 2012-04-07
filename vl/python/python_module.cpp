/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file python/python_module.cpp
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

// Callback helpers
#include <toast/python/callback.hpp>
// For namespace renaming
#include "python_context_impl.hpp"

// Game
#include "game_manager.hpp"

// SceneGraph
#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "movable_text.hpp"
#include "ray_object.hpp"

#include "material.hpp"
#include "material_manager.hpp"

#include "player.hpp"

// For some reason necessary
#include "event_manager.hpp"

// GUI
#include "gui/gui.hpp"
#include "gui/gui_window.hpp"
#include "gui/console.hpp"

// Animation framework
#include "animation/constraints.hpp"
#include "animation/kinematic_body.hpp"
#include "animation/kinematic_world.hpp"

#include "recording.hpp"

/// Necessary for exporting math
#include "math/transform.hpp"
#include "math/types.hpp"

// Necessary for hide/show system console
#include "base/system_util.hpp"

#include "game_object.hpp"

// Necessary for exposing vectors
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

/// Overloads need to be outside the module definition

/// SceneGraph overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setSpotlightRange_ov, setSpotlightRange, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(hideSceneNodes_ov, hideSceneNodes, 1, 3)

// MeshManager overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createPlane_ovs, createPlane, 1, 6)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createCube_ovs, createCube, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createSphere_ovs, createSphere, 1, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createCylinder_ovs, createCylinder, 1, 5)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(createCapsule_ovs, createCapsule, 1, 5)


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setDirection_ovs, setDirection, 1, 4)

BOOST_PYTHON_FUNCTION_OVERLOADS(lookAt_ovs, lookAt, 3, 5)

using namespace vl;


void export_math(void)
{
	python::class_<vl::Transform>("Transform", python::init<python::optional<Ogre::Vector3, Ogre::Quaternion> >() )
		.def(python::init<Ogre::Quaternion, python::optional<Ogre::Vector3> >())
		.def(python::init<vl::Transform>())
		.def("isIdentity", &vl::Transform::isIdentity)
		.def("setIdentity", &vl::Transform::setIdentity)
		.def("isPositionZero", &vl::Transform::isPositionZero)
		.def("setPositionZero", &vl::Transform::setPositionZero)
		.def("isRotationIdentity", &vl::Transform::isRotationIdentity)
		.def("setRotationIdentity", &vl::Transform::setRotationIdentity)
		.def("invert", &vl::Transform::invert)
		.def("inverted", &vl::Transform::inverted)
		/// @todo these make links so if we store the value in one of these
		/// it will be a reference so changing it will change
		/// the transformation and vice versa
		.def_readwrite("position", &vl::Transform::position)
		.def_readwrite("quaternion", &vl::Transform::quaternion)
		
		.def(-python::self)
		.def(python::self *= python::self)
		.def(python::self *= Ogre::Vector3())
		.def(python::self *= Ogre::Matrix4())
		
		.def(python::self * python::self)
		.def(python::self * Ogre::Quaternion())
		.def(python::self * Ogre::Vector3())
		.def(python::self * Ogre::Matrix4())

		.def(python::self == python::self)
		.def(python::self != python::self)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::def("lookAt", vl::lookAt, lookAt_ovs());
}

void export_animation(void)
{
	/// Abstract classes can not expose any methods, they are not available for derived classes...
	python::class_<vl::ObjectInterface, boost::noncopyable>("ObjectInterface", python::no_init)
	;


	void (vl::KinematicBody::*oi_translate_ov0)(Ogre::Vector3 const &) = &vl::KinematicBody::translate;
	void (vl::KinematicBody::*oi_translate_ov1)(Ogre::Real, Ogre::Real, Ogre::Real) = &vl::ObjectInterface::translate;
	void (vl::KinematicBody::*oi_rotate_ov0)(Ogre::Quaternion const &) = &vl::KinematicBody::rotate;

	KinematicBodyRefPtr (vl::KinematicBody::*kb_clone_ov0)() const = &vl::KinematicBody::clone;
	KinematicBodyRefPtr (vl::KinematicBody::*kb_clone_ov1)(std::string const &) const = &vl::KinematicBody::clone;

	python::class_<vl::KinematicBody, vl::KinematicBodyRefPtr, boost::noncopyable, python::bases<vl::ObjectInterface> >("KinematicBody", python::no_init)
		.add_property("scene_node", python::make_function(&vl::KinematicBody::getSceneNode, python::return_value_policy<python::reference_existing_object>()))
		.add_property("state", python::make_function(&vl::KinematicBody::getMotionState, python::return_value_policy<python::reference_existing_object>()))
		.add_property("name", python::make_function(&vl::KinematicBody::getName, python::return_value_policy<python::copy_const_reference>()))
		.add_property("world_transformation", &vl::KinematicBody::getWorldTransform, &vl::KinematicBody::setWorldTransform)
		.add_property("visible", &vl::KinematicBody::isVisible, &vl::KinematicBody::setVisibility)
		.add_property("position", python::make_function( &vl::KinematicBody::getPosition, python::return_value_policy<python::copy_const_reference>() ), 
				&vl::KinematicBody::setPosition)
		.add_property("orientation", python::make_function( &vl::KinematicBody::getOrientation, python::return_value_policy<python::copy_const_reference>() ), 
				&vl::KinematicBody::setOrientation)
		.def("transform", &vl::KinematicBody::transform)
		.def("translate", oi_translate_ov0)
		.def("translate", oi_translate_ov1)
		.def("rotate", oi_rotate_ov0)
		.def("hide", &vl::ObjectInterface::hide)
		.def("show", &vl::ObjectInterface::show)
		.def("clone", kb_clone_ov0)
		.def("clone", kb_clone_ov1)
		/// Optimisation parameters
		.add_property("disable", &vl::KinematicBody::isDisableUpdate, &vl::KinematicBody::setDisableUpdate)
		.add_property("use_dirties", &vl::KinematicBody::isUseDirties, &vl::KinematicBody::setUseDirties)
		.add_property("assume_in_world", &vl::KinematicBody::isAssumeInWorld, &vl::KinematicBody::setAssumeInWorld)
		.add_property("collisions_enabled", &vl::KinematicBody::isCollisionsEnabled, &vl::KinematicBody::enableCollisions)

		.def("addListener", toast::python::signal_connect<void (vl::Transform const &)>(&vl::KinematicBody::addListener))
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// Shared pointer needs Proxies to be turned off
	python::class_<std::vector<boost::shared_ptr<KinematicBody> > >("KinematicBodyList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<KinematicBody> >, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;


	/// Shared pointer needs Proxies to be turned off
	python::class_<std::vector<boost::shared_ptr<Constraint> > >("ConstraintList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<Constraint> >, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::KinematicBodyRefPtr (vl::KinematicWorld::*getKinematicBody_ov0)(std::string const &) const = &vl::KinematicWorld::getKinematicBody;
	vl::KinematicBodyRefPtr (vl::KinematicWorld::*getKinematicBody_ov1)(vl::SceneNodePtr) const = &vl::KinematicWorld::getKinematicBody;

	vl::ConstraintRefPtr (vl::KinematicWorld::*createConstraint_ov0)(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, vl::Transform const &trans) = &vl::KinematicWorld::createConstraint;
	vl::ConstraintRefPtr (vl::KinematicWorld::*createConstraint_ov1)(std::string const &type, 
		KinematicBodyRefPtr body0, KinematicBodyRefPtr body1, vl::Transform const &, vl::Transform const &) = &vl::KinematicWorld::createConstraint;

	/// @todo add list getters
	// bodies not yet working as we would like, probably something to do with ref ptrs
	python::class_<vl::KinematicWorld, vl::KinematicWorldRefPtr, boost::noncopyable>("KinematicWorld", python::no_init)
		.def("get_kinematic_body", getKinematicBody_ov0)
		.def("get_kinematic_body", getKinematicBody_ov1)
		.def("create_kinematic_body", &vl::KinematicWorld::createKinematicBody)
		.def("remove_kinematic_body", &vl::KinematicWorld::removeKinematicBody)
		.def("create_constraint", createConstraint_ov0)
		.def("create_constraint", createConstraint_ov1)
		.def("remove_constraint", &vl::KinematicWorld::removeConstraint)
		.def("get_constraint", &vl::KinematicWorld::getConstraint)
		.add_property("constraints", python::make_function(&vl::KinematicWorld::getConstraints, python::return_value_policy<python::copy_const_reference>()))
		.add_property("bodies", python::make_function(&vl::KinematicWorld::getBodies, python::return_value_policy<python::copy_const_reference>()))
		.add_property("collision_detection_enabled", &vl::KinematicWorld::isCollisionDetectionEnabled, &vl::KinematicWorld::enableCollisionDetection)
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// Abstract master class for all non-physics constraints
	python::class_<vl::Constraint, vl::ConstraintRefPtr, boost::noncopyable>("Constraint", python::no_init)
		.add_property("body_a", &vl::Constraint::getBodyA)
		.add_property("body_b", &vl::Constraint::getBodyB)
		.add_property("actuator", &vl::Constraint::isActuator, &vl::Constraint::setActuator)
		.add_property("name", python::make_function(&vl::Constraint::getName, python::return_value_policy<python::copy_const_reference>()), &vl::Constraint::setName)
		.def("set_velocity", &vl::Constraint::setVelocity)
		.def("add_velocity", &vl::Constraint::addVelocity)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::FixedConstraint, vl::FixedConstraintRefPtr, boost::noncopyable, python::bases<vl::Constraint> >("FixedConstraint", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::SliderConstraint, vl::SliderConstraintRefPtr, boost::noncopyable, python::bases<vl::Constraint> >("SliderConstraint", python::no_init)
		.add_property("lower_limit", &vl::SliderConstraint::getLowerLimit, &vl::SliderConstraint::setLowerLimit)
		.add_property("upper_limit", &vl::SliderConstraint::getUpperLimit, &vl::SliderConstraint::setUpperLimit)
		.add_property("speed", &vl::SliderConstraint::getActuatorSpeed, &vl::SliderConstraint::setActuatorSpeed)
		.add_property("target", &vl::SliderConstraint::getActuatorTarget, &vl::SliderConstraint::setActuatorTarget)
		.add_property("position", &vl::SliderConstraint::getPosition)
		.add_property("axis", python::make_function(&vl::SliderConstraint::getAxis, python::return_value_policy<python::copy_const_reference>()), &vl::SliderConstraint::setAxis)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::HingeConstraint, vl::HingeConstraintRefPtr, boost::noncopyable, python::bases<vl::Constraint> >("HingeConstraint", python::no_init)
		.add_property("lower_limit", python::make_function(&vl::HingeConstraint::getLowerLimit, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setLowerLimit)
		.add_property("upper_limit", python::make_function(&vl::HingeConstraint::getUpperLimit, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setUpperLimit)
		.add_property("speed", python::make_function(&vl::HingeConstraint::getActuatorSpeed, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setActuatorSpeed)
		.add_property("target", python::make_function(&vl::HingeConstraint::getActuatorTarget, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setActuatorTarget)
		.add_property("angle", python::make_function(&vl::HingeConstraint::getHingeAngle))
		.add_property("axis", python::make_function(&vl::HingeConstraint::getAxis, python::return_value_policy<python::copy_const_reference>()), &vl::HingeConstraint::setAxis)
		.def(python::self_ns::str(python::self_ns::self))
	;
}

// @todo move export scene graph to separate file
void export_scene_graph(void)
{
	vl::MeshRefPtr (MeshManager::*loadMesh_ov0)(std::string const &) = &MeshManager::loadMesh;
	
	python::class_<vl::MeshManager, vl::MeshManagerRefPtr, boost::noncopyable>("MeshManager", python::no_init)
		.def("loadMesh", loadMesh_ov0)
		.def("createPlane", &vl::MeshManager::createPlane, createPlane_ovs())
		.def("createSphere", &vl::MeshManager::createSphere, createSphere_ovs())
		.def("createCube", &vl::MeshManager::createCube, createCube_ovs())
		.def("createCylinder", &vl::MeshManager::createCylinder, createCylinder_ovs())
		.def("createCapsule", &vl::MeshManager::createCapsule, createCapsule_ovs())
		.def("getMesh", &vl::MeshManager::getMesh)
		.def("hasMesh", &vl::MeshManager::hasMesh)
		.def("cleanup_unused", &vl::MeshManager::cleanup_unused)
	;

	
	python::class_<vl::Material, vl::MaterialRefPtr, boost::noncopyable>("Material", python::no_init)
		.add_property("shader", python::make_function(&vl::Material::getShader, 
			python::return_value_policy<python::copy_const_reference>() ), &vl::Material::setShader)
		.add_property("texture", python::make_function(&vl::Material::getTexture, 
			python::return_value_policy<python::copy_const_reference>() ), &vl::Material::setTexture)
		.add_property("diffuse", python::make_function(&vl::Material::getDiffuse, 
			python::return_value_policy<python::copy_const_reference>() ), &vl::Material::setDiffuse)
		.add_property("specular", python::make_function(&vl::Material::getSpecular, 
			python::return_value_policy<python::copy_const_reference>() ), &vl::Material::setSpecular)
		.add_property("emissive", python::make_function(&vl::Material::getEmissive, 
			python::return_value_policy<python::copy_const_reference>() ), &vl::Material::setEmissive)
		.add_property("ambient", python::make_function(&vl::Material::getAmbient, 
			python::return_value_policy<python::copy_const_reference>() ), &vl::Material::setAmbient)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<std::vector<MaterialRefPtr> >("MaterialList")
		.def(python::vector_indexing_suite<std::vector<MaterialRefPtr>, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::MaterialManager, vl::MaterialManagerRefPtr, boost::noncopyable>("MaterialManager", python::no_init)
		.def("create_material", &vl::MaterialManager::createMaterial)
		.def("get_material", &vl::MaterialManager::getMaterial)
		.def("has_material", &vl::MaterialManager::hasMaterial)
		.add_property("materials", python::make_function(&vl::MaterialManager::getMaterialList, python::return_value_policy<python::copy_const_reference>()) )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::FogInfo>("FogInfo", python::init<>())
		.def(python::init<std::string, python::optional<Ogre::ColourValue, Ogre::Real, Ogre::Real, Ogre::Real> >())
		.def(python::init<vl::FogInfo>())
		.add_property("mode", &vl::FogInfo::setMode, &vl::FogInfo::getMode)
		.def_readwrite("colour", &vl::FogInfo::colour_diffuse)
		.def_readwrite("density", &vl::FogInfo::exp_density)
		.def_readwrite("linear_start", &vl::FogInfo::linear_start)
		.def_readwrite("linear_end", &vl::FogInfo::linear_end)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::SkyDomeInfo>("SkyDomeInfo", python::init< python::optional<std::string> >())
		.def(python::init<vl::SkyDomeInfo>())
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

	python::class_<vl::SkyInfo>( "SkyInfo", python::init< python::optional<std::string> >() )
		.add_property("preset", python::make_function(&vl::SkyInfo::getPreset, python::return_value_policy<python::copy_const_reference>()), &vl::SkyInfo::setPreset)
	;

	python::class_<vl::ShadowInfo>( "ShadowInfo", python::init<std::string>() )
		.def(python::init<vl::ShadowInfo>())
		.def("enable", &vl::ShadowInfo::enable)
		.def("disable", &vl::ShadowInfo::disable)
		.add_property("enabled", &vl::ShadowInfo::isEnabled, &vl::ShadowInfo::setEnabled)
		.add_property("camera", python::make_function(&vl::ShadowInfo::getCamera, python::return_value_policy<python::copy_const_reference>()), &vl::ShadowInfo::setCamera)
		.add_property("shelf_shadow", &vl::ShadowInfo::isShelfShadowEnabled, &vl::ShadowInfo::setShelfShadowEnabled)
		.add_property("dir_light_texture_offset", &vl::ShadowInfo::getDirLightTextureOffset, &vl::ShadowInfo::setDirLightTextureOffset)
		.add_property("max_distance", &vl::ShadowInfo::getMaxDistance, &vl::ShadowInfo::setMaxDistance)
		.add_property("caster_material", python::make_function(&vl::ShadowInfo::getShadowCasterMaterial, python::return_value_policy<python::copy_const_reference>()), &vl::ShadowInfo::setShadowCasterMaterial)
		.add_property("texture_size", &vl::ShadowInfo::getTextureSize, &vl::ShadowInfo::setTextureSize)
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::EntityPtr (SceneManager::*createEntity_ov0)(std::string const &, vl::PREFAB) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov1)(std::string const &, std::string const &) = &SceneManager::createEntity;
	vl::EntityPtr (SceneManager::*createEntity_ov2)(std::string const &, std::string const &, bool) = &SceneManager::createEntity;
	vl::ShadowInfo &(SceneManager::*getShadowInfo_ov0)(void) = &vl::SceneManager::getShadowInfo;

	python::class_<std::vector<SceneNode *> >("SceneNodeList")
		.def(python::vector_indexing_suite<std::vector<SceneNode *> >())
	;

	python::class_<std::vector<Camera *> >("CameraList")
		.def(python::vector_indexing_suite<std::vector<Camera *> >())
	;

	python::class_<std::vector<MovableObject *> >("ObjectList")
		.def(python::vector_indexing_suite<std::vector<MovableObject *> >())
	;

	python::class_<vl::SceneManager, boost::noncopyable>("SceneManager", python::no_init)
		// TODO add remove SceneNodes
		.add_property("root", python::make_function(&vl::SceneManager::getRootSceneNode, python::return_value_policy<python::reference_existing_object>()))
		// Copy const reference here causes the containers to be out-of-date 
		// if they are stored in the python side. Returning internal references
		// might work but we need to be sure they can not be modified from python.
		.add_property("selection", python::make_function(&vl::SceneManager::getSelection, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("active_object", python::make_function(&vl::SceneManager::getActiveObject, python::return_value_policy<python::reference_existing_object>()), &vl::SceneManager::setActiveObject)
		.add_property("scene_nodes", python::make_function(&vl::SceneManager::getSceneNodeList, python::return_value_policy<python::copy_const_reference>()))
		.add_property("objects", python::make_function(&vl::SceneManager::getMovableObjectList, python::return_value_policy<python::copy_const_reference>()))
		.add_property("cameras", &vl::SceneManager::getCameraList )
		.def("createSceneNode", &vl::SceneManager::createSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("hasSceneNode", &SceneManager::hasSceneNode )
		.def("getSceneNode", &SceneManager::getSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov0, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov1, python::return_value_policy<python::reference_existing_object>() )
		.def("createEntity", createEntity_ov2, python::return_value_policy<python::reference_existing_object>() )
		.def("getEntity", &SceneManager::getEntity, python::return_value_policy<python::reference_existing_object>() )
		.def("hasEntity", &SceneManager::hasEntity)
		.def("createCamera", &SceneManager::createCamera, python::return_value_policy<python::reference_existing_object>() )
		.def("getCamera", &SceneManager::getCamera, python::return_value_policy<python::reference_existing_object>() )
		.def("hasCamera", &SceneManager::hasCamera)
		.def("createLight", &SceneManager::createLight, python::return_value_policy<python::reference_existing_object>() )
		.def("getLight", &SceneManager::getLight, python::return_value_policy<python::reference_existing_object>() )
		.def("hasLight", &SceneManager::hasLight)
		.def("createMovableText", &SceneManager::createMovableText, python::return_value_policy<python::reference_existing_object>() )
		.def("getMovableText", &SceneManager::getMovableText, python::return_value_policy<python::reference_existing_object>() )
		.def("hasMovableText", &SceneManager::hasMovableText)
		.def("createRayObject", &SceneManager::createRayObject, python::return_value_policy<python::reference_existing_object>() )
		.def("getRayObject", &SceneManager::getRayObject, python::return_value_policy<python::reference_existing_object>() )
		.def("hasRayObject", &SceneManager::hasRayObject)
		.def("show_debug_displays", &SceneManager::showDebugDisplays)
		.def("show_bounding_boxes", &SceneManager::showBoundingBoxes)
		.def("show_axes", &SceneManager::showAxes)

		/// Scene parameters
		/// returns copies of the objects
		.add_property("sky_dome", python::make_function( &vl::SceneManager::getSkyDome, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setSkyDome )
		.add_property("fog", python::make_function( &vl::SceneManager::getFog, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setFog )
		.add_property("ambient_light", python::make_function( &vl::SceneManager::getAmbientLight, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneManager::setAmbientLight )
		.add_property("shadows", python::make_function(getShadowInfo_ov0, python::return_internal_reference<>()), &vl::SceneManager::setShadowInfo)
		.add_property("root", python::make_function(&vl::SceneManager::getRootSceneNode, python::return_value_policy<python::reference_existing_object>()))
		.add_property("sky", python::make_function(&vl::SceneManager::getSkyInfo, python::return_internal_reference<>()), &vl::SceneManager::setSkyInfo)

		/// Selection
		.def("addToSelection", &SceneManager::addToSelection)
		.def("removeFromSelection", &SceneManager::removeFromSelection)
		.def("clearSelection", &SceneManager::clearSelection)

		.def("hideSceneNodes", &vl::SceneManager::hideSceneNodes, hideSceneNodes_ov())
		.def("mapCollisionBarriers", &vl::SceneManager::mapCollisionBarriers)
		.def("reloadScene", &SceneManager::reloadScene)
		.def(python::self_ns::str(python::self_ns::self))
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
		.add_property("visible", &vl::MovableObject::getVisible, &vl::MovableObject::setVisible)
		.add_property("position", python::make_function( &vl::MovableObject::getPosition, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableObject::setPosition )
		.add_property("orientation", python::make_function( &vl::MovableObject::getOrientation, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableObject::setOrientation )
		.add_property("world_position", python::make_function( &vl::MovableObject::getWorldPosition))
		.add_property("world_orientation", python::make_function(&vl::MovableObject::getWorldOrientation))
		.def("hide", &vl::MovableObject::hide)
		.def("show", &vl::MovableObject::show)
	;

	python::class_<vl::LightAttenuation>("LightAttenuation", python::init<>() )
		.def(python::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(python::init<vl::LightAttenuation>())
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
		.add_property("cast_shadows", &vl::Light::getCastShadows, &vl::Light::setCastShadows)
		// TODO this should use a struct and be a property
		.def("setSpotRange", &vl::Light::setSpotlightRange, setSpotlightRange_ov())
		.add_property("attenuation", python::make_function( &vl::Light::getAttenuation, python::return_value_policy<python::copy_const_reference>() ), &vl::Light::setAttenuation )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Camera, boost::noncopyable, python::bases<vl::MovableObject> >("Camera", python::no_init)
		.add_property("near_clip", &vl::Camera::getNearClipDistance, &vl::Camera::setNearClipDistance )
		.add_property("far_clip", &vl::Camera::getFarClipDistance, &vl::Camera::setFarClipDistance )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Entity, boost::noncopyable, python::bases<vl::MovableObject> >("Entity", python::no_init)
		.add_property("material_name", python::make_function( &vl::Entity::getMaterialName, python::return_value_policy<python::copy_const_reference>() ), &vl::Entity::setMaterialName )
		.add_property("cast_shadows", &vl::Entity::getCastShadows, &vl::Entity::setCastShadows )
		.add_property("mesh_name", python::make_function( &vl::Entity::getMeshName, python::return_value_policy<python::copy_const_reference>() ) )
		.add_property("mesh", &vl::Entity::getMesh)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::MovableText, boost::noncopyable, python::bases<vl::MovableObject> >("MovableText", python::no_init)
		.add_property("font_name", python::make_function( &vl::MovableText::getFontName, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setFontName )
		.add_property("caption", python::make_function( &vl::MovableText::getCaption, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setCaption )
		.add_property("colour", python::make_function( &vl::MovableText::getColour, python::return_value_policy<python::copy_const_reference>() ), &vl::MovableText::setColour)
		.add_property("char_height", &vl::MovableText::getCharacterHeight, &vl::MovableText::setCharacterHeight)
		.add_property("space_width", &vl::MovableText::getSpaceWidth, &vl::MovableText::setSpaceWidth)
		.add_property("show_on_top", &vl::MovableText::getShowOnTop, &vl::MovableText::showOnTop)
		.def("set_text_alignment", &vl::MovableText::setTextAlignment)
	;
	
	python::class_<vl::RayObject, boost::noncopyable, python::bases<vl::MovableObject> >("RayObject", python::no_init)
		.add_property("direction", python::make_function( &vl::RayObject::getDirection, python::return_value_policy<python::copy_const_reference>() ), &vl::RayObject::setDirection)
		.add_property("material", python::make_function( &vl::RayObject::getMaterial, python::return_value_policy<python::copy_const_reference>() ), &vl::RayObject::setMaterial)
		.add_property("length", &vl::RayObject::getLength, &vl::RayObject::setLength)
		.add_property("sphere_radius", &vl::RayObject::getSphereRadius, &vl::RayObject::setSphereRadius)
		.add_property("collision_detection", &vl::RayObject::getCollisionDetection, &vl::RayObject::setCollisionDetection)
		.add_property("draw_collision_sphere", &vl::RayObject::getDrawCollisionSphere, &vl::RayObject::setDrawCollisionSphere)
		.add_property("draw_ray", &vl::RayObject::getDrawRay, &vl::RayObject::setDrawRay)
		.add_property("show_recording", &vl::RayObject::getShowRecordedRays, &vl::RayObject::showRecordedRays)
		.add_property("recording", &vl::RayObject::getRecording, &vl::RayObject::setRecording)
		.def("update", &vl::RayObject::update)
	;

	python::enum_<TransformSpace>("TS")
		.value("LOCAL", TS_LOCAL)
		.value("PARENT", TS_PARENT)
		.value("WORLD", TS_WORLD)
	;

	void (vl::SceneNode::*transform_ov0)(vl::Transform const &) = &vl::SceneNode::transform;
	void (vl::SceneNode::*transform_ov1)(Ogre::Matrix4 const &) = &vl::SceneNode::transform;

	void (vl::SceneNode::*setTransform_ov0)(vl::Transform const &) = &vl::SceneNode::setTransform;
	void (vl::SceneNode::*setTransform_ov1)(vl::Transform const &, vl::SceneNodePtr) = &vl::SceneNode::setTransform;
	void (vl::SceneNode::*setTransform_ov2)(Ogre::Matrix4 const &) = &vl::SceneNode::setTransform;

	vl::Transform const &(vl::SceneNode::*getTransform_ov0)(void) const = &vl::SceneNode::getTransform;
	vl::Transform (vl::SceneNode::*getTransform_ov1)(vl::SceneNodePtr) const = &vl::SceneNode::getTransform;
	void (vl::SceneNode::*translate_ov1)(Ogre::Vector3 const &, vl::SceneNodePtr) = &vl::SceneNode::translate;
	void (vl::SceneNode::*translate_ov2)(Ogre::Vector3 const &, TransformSpace) = &vl::SceneNode::translate;
	void (vl::SceneNode::*rotate_ov2)(Ogre::Quaternion const &, TransformSpace) = &vl::SceneNode::rotate;
//	void (vl::SceneNode::*rotate_ov1)(Ogre::Quaternion const &, vl::SceneNodePtr) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov3)(Ogre::Vector3 const &, Ogre::Degree const &) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov4)(Ogre::Vector3 const &, Ogre::Radian const &) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov5)(Ogre::Degree const &, Ogre::Vector3 const &) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*rotate_ov6)(Ogre::Radian const &, Ogre::Vector3 const &) = &vl::SceneNode::rotate;
	void (vl::SceneNode::*scale_ov0)(Ogre::Vector3 const &) = &vl::SceneNode::scale;
	void (vl::SceneNode::*scale_ov1)(Ogre::Real) = &vl::SceneNode::scale;

	SceneNodePtr (vl::SceneNode::*sn_clone_ov0)() const = &vl::SceneNode::clone;
	SceneNodePtr (vl::SceneNode::*sn_clone_ov1)(std::string const &) const = &vl::SceneNode::clone;
	
	void (vl::SceneNode::*setVisible_ov0)(bool) = &vl::SceneNode::setVisibility;
	void (vl::SceneNode::*setVisible_ov1)(bool, bool) = &vl::SceneNode::setVisibility;

	void (vl::SceneNode::*oi_translate_ov0)(Ogre::Vector3 const &) = &vl::SceneNode::translate;
	void (vl::SceneNode::*oi_translate_ov1)(Ogre::Real, Ogre::Real, Ogre::Real) = &vl::ObjectInterface::translate;
	void (vl::SceneNode::*oi_rotate_ov0)(Ogre::Quaternion const &) = &vl::SceneNode::rotate;

	// naming convention 
	// transform is the operation of transforming
	// transformation is the noun descriping that transform operation
	python::class_<vl::SceneNode, boost::noncopyable, python::bases<vl::ObjectInterface> >("SceneNode", python::no_init)
		.add_property("name", python::make_function(&vl::SceneNode::getName, python::return_value_policy<python::copy_const_reference>()))
		.add_property("world_transformation", &vl::SceneNode::getWorldTransform, &vl::SceneNode::setWorldTransform)
		.add_property("visible", &vl::SceneNode::isVisible, setVisible_ov0)
		.add_property("position", python::make_function( &vl::SceneNode::getPosition, python::return_value_policy<python::copy_const_reference>() ), 
				&vl::SceneNode::setPosition)
		.add_property("orientation", python::make_function( &vl::SceneNode::getOrientation, python::return_value_policy<python::copy_const_reference>() ), 
				&vl::SceneNode::setOrientation)
		.add_property("inherit_scale", &SceneNode::getInheritScale, &vl::SceneNode::setInheritScale)
		.add_property("show_bounding_box", &SceneNode::getShowBoundingBox, &vl::SceneNode::setShowBoundingBox)
		.add_property("parent", python::make_function(&vl::SceneNode::getParent, python::return_value_policy<python::reference_existing_object>()) )
		.add_property("direction", &vl::SceneNode::getDirection)
		.add_property("childs", python::make_function(&vl::SceneNode::getChilds, python::return_value_policy<python::copy_const_reference>()))
		.add_property("objects", python::make_function(&vl::SceneNode::getObjects, python::return_value_policy<python::copy_const_reference>()))
		.add_property("show_debug_display", &SceneNode::isShowDebugDisplay, &vl::SceneNode::setShowDebugDisplay)
		.add_property("show_axes", &SceneNode::isShowAxes, &vl::SceneNode::setShowAxes)

		.def("attachObject", &vl::SceneNode::attachObject)
		.def("detachObject", &vl::SceneNode::detachObject)
		.def("hasObject", &vl::SceneNode::hasObject)
		.def("createChildSceneNode", &vl::SceneNode::createChildSceneNode, python::return_value_policy<python::reference_existing_object>() )
		.def("addChild", &vl::SceneNode::addChild)
		.def("removeChild", &vl::SceneNode::removeChild)
		.def("removeAllChildren", &vl::SceneNode::removeAllChildren)
		.def("transform", transform_ov1)
		.def("transform", transform_ov0)
		.def("translate", translate_ov1)
		.def("translate", translate_ov2)
		.def("rotate", rotate_ov2)
		.def("rotate", rotate_ov3)
		.def("rotate", rotate_ov4)
		.def("rotate", rotate_ov5)
		.def("rotate", rotate_ov6)
		.def("rotate_around", &vl::SceneNode::rotateAround)
		.def("scale", scale_ov0)
		.def("scale", scale_ov1)
		.def("translate", oi_translate_ov0)
		.def("translate", oi_translate_ov1)
		.def("rotate", oi_rotate_ov0)
		.def("hide", &vl::SceneNode::hide)
		.def("show", &vl::SceneNode::show)
		.def("clone", sn_clone_ov0, python::return_value_policy<python::reference_existing_object>())
		.def("clone", sn_clone_ov1, python::return_value_policy<python::reference_existing_object>())
		.def("set_transformation", setTransform_ov0)
		.def("set_transformation", setTransform_ov1)
		.def("set_transformation", setTransform_ov2)
		.def("set_world_transformation", &vl::SceneNode::setWorldTransform)
		.add_property("transformation", python::make_function( getTransform_ov0, python::return_value_policy<python::copy_const_reference>() ), setTransform_ov0)
		.add_property("scaling", python::make_function( &vl::SceneNode::getScale, python::return_value_policy<python::copy_const_reference>() ), &vl::SceneNode::setScale )
		.def("set_visible", setVisible_ov0)
		.def("set_visible", setVisible_ov1)
		.def("set_direction", &vl::SceneNode::setDirection, setDirection_ovs())
		.def("look_at", &vl::SceneNode::lookAt)
		.def("isHidden", &vl::SceneNode::isHidden)
		.def("isShown", &vl::SceneNode::isShown)
		.def("addListener", toast::python::signal_connect<void (vl::Transform const &)>(&vl::SceneNode::addListener))
		.def(python::self_ns::str(python::self_ns::self))
		/// @todo add repr with a debug string presentation
		/// This is used for debuging purposes (which we use __str__ or operator<< at the moment)
		/// then we can clear the default print values to something sensible
		/// that can be presented to the user.
		//.def("__repr__", &vl::SceneNode::getDebugString)
	;

}

void export_game(void)
{
	/// System utils
	python::def("show_system_console", vl::show_system_console);
	python::def("hide_system_console", vl::hide_system_console);

	python::class_<vl::time>("time", python::init<uint32_t, uint32_t>())
 		.def(python::init<int>())
 		.def(python::init<double>())
		.def(python::init<>())
		.def(python::self -= python::self)
		.def(python::self - python::self)
		.def(python::self += python::self)
		.def(python::self + python::self)
		.def(python::self == python::self)
		.def(python::self != python::self)
		.def(python::self >= python::self)
		.def(python::self <= python::self)
		.def(python::self > python::self)
		.def(python::self < python::self)
		.def(python::self / double())
		.def(python::self /= double())
		.def(python::self / size_t())
		.def(python::self /= size_t())
		.def(python::self_ns::str(python::self_ns::self))
		.def(python::self_ns::float_(python::self_ns::self))
 	;

	void (vl::GameManager::*loadScene_ov0)(std::string const &)= &vl::GameManager::loadScene;

	python::class_<vl::chrono>("chrono", python::init<>())
		.def(python::init<vl::time const &>())
		.def("elapsed", &vl::chrono::elapsed)
		.def("reset", &vl::chrono::reset)
	;

	python::class_<vl::Report<vl::time>, boost::noncopyable>("Report", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;

	void (vl::GameObject::*setCollisionModel_ov0)(physics::CollisionShapeRefPtr shape) = &vl::GameObject::setCollisionModel;

	python::class_<vl::GameObject, vl::GameObjectRefPtr, boost::noncopyable, python::bases<vl::ObjectInterface> >("GameObject", python::no_init)
		.add_property("graphics_node", python::make_function( &vl::GameObject::getGraphicsNode, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("kinematic_node", &vl::GameObject::getKinematicsNode)
		.add_property("dynamics_node", &vl::GameObject::getPhysicsNode)
		.add_property("collision_detection", &vl::GameObject::isCollisionDetectionEnabled, &vl::GameObject::enableCollisionDetection)
		.add_property("visible", &vl::GameObject::isVisible, &vl::GameObject::setVisibility)
		.add_property("name", python::make_function(&vl::GameObject::getName, python::return_value_policy<python::copy_const_reference>()) )
		.add_property("transformation", python::make_function(&vl::GameObject::getTransform, python::return_value_policy<python::copy_const_reference>()), &vl::GameObject::setTransform)
		.add_property("world_transformation", &vl::GameObject::getWorldTransform, &vl::GameObject::setWorldTransform)
		.add_property("position", python::make_function(&vl::GameObject::getPosition, python::return_value_policy<python::copy_const_reference>()), &vl::GameObject::setPosition)
		.add_property("orientation", python::make_function(&vl::GameObject::getOrientation, python::return_value_policy<python::copy_const_reference>()), &vl::GameObject::setOrientation)
		.add_property("collision_model", &vl::GameObject::getCollisionModel, setCollisionModel_ov0)
		.add_property("kinematic", &vl::GameObject::isKinematic, &vl::GameObject::setKinematic)		
		.def("transform", &vl::GameObject::transform)
		.def("translate", &vl::GameObject::translate)
		.def("rotate", &vl::GameObject::rotate)
		.def("set_world_transformation", &vl::GameObject::setWorldTransform)
		// @todo create rigid body should be changed, but needs redesign in the GameObject class itself
		.def("create_rigid_body", &vl::GameObject::createRigidBody)
		.def("addListener", toast::python::signal_connect<void (vl::Transform const &)>(&vl::GameObject::addListener))
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// Shared pointer needs Proxies to be turned off
	python::class_<std::vector<boost::shared_ptr<GameObject> > >("GameObjectList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<GameObject> >, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::GameManager, boost::noncopyable>("GameManager", python::no_init)
		.add_property("scene", python::make_function( &vl::GameManager::getSceneManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("player", python::make_function( &vl::GameManager::getPlayer, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("event_manager", python::make_function( &vl::GameManager::getEventManager, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("gui", &vl::GameManager::getGUI)
		.add_property("rendering_report", python::make_function( &vl::GameManager::getRenderingReport, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("init_report", python::make_function( &vl::GameManager::getInitReport, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property( "physics_world", &vl::GameManager::getPhysicsWorld)
		.def( "enablePhysics", &vl::GameManager::enablePhysics )
		.add_property("logger", python::make_function( &vl::GameManager::getLogger, python::return_value_policy<python::reference_existing_object>() ) )
		.add_property("kinematic_world", &vl::GameManager::getKinematicWorld)
		// GameObject
		.add_property("objects", python::make_function( &vl::GameManager::getGameObjectList, python::return_value_policy<python::reference_existing_object>() ) )
		.def("get_object", &vl::GameManager::getGameObject)
		.def("has_object", &vl::GameManager::hasGameObject)
		.def("create_object", &vl::GameManager::createGameObject)

		// State management
		.def("quit", &vl::GameManager::quit)
		.def("pause", &vl::GameManager::pause)
		.def("play", &vl::GameManager::play)
		.def("stop", &vl::GameManager::stop)
		.def("restart", &vl::GameManager::restart)
		.add_property("paused", &vl::GameManager::isPaused)
		.add_property("playing", &vl::GameManager::isPlaying)
		.add_property("quited", &vl::GameManager::isQuited)
		.add_property("auto_start", &vl::GameManager::auto_start, &vl::GameManager::set_auto_start)
		/// @todo add state inqueries
		/// @todo add step function, needs to rename the current one in GameManager which is internal
		/// called from Config...
		/// python step function should step the simulation with a delta time (or a default one like 1/60 s)
		.add_property("tracker_clients", &vl::GameManager::getTrackerClients)
		.add_property("mesh_manager", &vl::GameManager::getMeshManager)
		.add_property("material_manager", &vl::GameManager::getMaterialManager)
		.def("loadRecording", &vl::GameManager::loadRecording)
		.def("load_scene", loadScene_ov0)
		.def("save_scene", &vl::GameManager::saveScene)
		.def("create_analog_client", &vl::GameManager::createAnalogClient)
	;

	void (sink::*write1)( std::string const & ) = &sink::write;

	python::class_<vl::sink>("sink", python::no_init)
		.def("write", write1 )
	;

	python::class_<vl::Player, boost::noncopyable>("Player", python::no_init)
		.add_property("camera", python::make_function( &Player::getActiveCamera , python::return_value_policy<python::copy_const_reference>() ), &Player::setActiveCamera)
		.add_property("camera_node", python::make_function(&Player::getCameraNode, python::return_value_policy<python::reference_existing_object>()))
		.add_property("head_transformation", python::make_function(&Player::getHeadTransform, python::return_value_policy<python::copy_const_reference>()), &Player::setHeadTransform)
		.add_property("cyclop_transformation", python::make_function(&Player::getCyclopTransform, python::return_value_policy<python::copy_const_reference>()), &Player::setCyclopTransform)
		.add_property("head_frustum_x", &vl::Player::isHeadFrustumX, &vl::Player::enableHeadFrustumX)
		.add_property("head_frustum_y", &vl::Player::isHeadFrustumY, &vl::Player::enableHeadFrustumY)
		.add_property("head_frustum_z", &vl::Player::isHeadFrustumZ, &vl::Player::enableHeadFrustumZ)
		.add_property("asymmetric_stereo_frustum", &vl::Player::isAsymmetricStereoFrustum, &vl::Player::enableAsymmetricStereoFrustum)
		.add_property("ipd", &vl::Player::getIPD, &vl::Player::setIPD)
		.def("takeScreenshot", &vl::Player::takeScreenshot)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::Recording, vl::RecordingRefPtr, boost::noncopyable>("Recording", python::no_init)
		.def(python::self_ns::str(python::self_ns::self))
	;
}

void export_gui(void)
{
	/// GUI
	/// @todo these are useless till the Window exist on both master and rendering thread
	/// so it needs to be distributed...
	python::class_<vl::gui::Window, vl::gui::WindowRefPtr, boost::noncopyable>("GUIWindow", python::no_init)
		.add_property("visible", &vl::gui::Window::isVisible, &vl::gui::Window::setVisible)
		.def("toggle_visible", &vl::gui::Window::toggleVisible)
		.def("hide", &vl::gui::Window::hide)
		.def("show", &vl::gui::Window::show)
	;

	python::class_<vl::gui::ConsoleWindow, vl::gui::ConsoleWindowRefPtr, boost::noncopyable, python::bases<vl::gui::Window> >("GUIConsoleWindow", python::no_init)
	;

	python::class_<vl::gui::GUI, vl::gui::GUIRefPtr, boost::noncopyable>("GUI", python::no_init )
		.add_property("console", &vl::gui::GUI::getConsole)
	;
}
