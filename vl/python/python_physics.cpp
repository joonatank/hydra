/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file python/python_physics.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/// Interface
#include "python_module.hpp"

#include "typedefs.hpp"

// For namespace renaming
#include "python_context_impl.hpp"

// Physics
#include "physics/physics_world.hpp"
#include "physics/rigid_body.hpp"
#include "physics/shapes.hpp"
#include "physics/physics_constraints.hpp"
#include "physics/tube.hpp"
#include "physics/motion_state.hpp"

// Necessary for exposing vectors
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

/// Physics world member overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createDynamicRigidBody_ov, createDynamicRigidBody, 4, 5 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( addConstraint_ovs, addConstraint, 1, 2 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( setLimit_ovs, setLimit, 2, 5 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createMotionState_ov, createMotionState, 0, 2 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createTube_ov, createTube, 3, 5 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( addFixingPoint_ovs, addFixingPoint, 1, 2 )

// @todo remove the namespace usage
using namespace vl;

namespace
{

void export_physics_objects(void)
{
	/// @todo add size
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
		.def("create", &vl::physics::ConvexHullShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::ConcaveHullShape, boost::noncopyable, vl::physics::ConcaveHullShapeRefPtr, python::bases<vl::physics::CollisionShape> >("ConcaveHullShape", python::no_init )
		.def("create", &vl::physics::ConcaveHullShape::create)
		.staticmethod("create")
	;

	python::class_<vl::physics::CompoundShape, boost::noncopyable, vl::physics::CompoundShapeRefPtr, python::bases<vl::physics::CollisionShape> >("CompoundShape", python::no_init )
		.def("create", &vl::physics::CompoundShape::create)
		.staticmethod("create")
		.def("add_child_shape", &vl::physics::CompoundShape::addChildShape)
		.def("remove_child_shape_by_idx", &vl::physics::CompoundShape::removeChildShapeByIndex)
		
		
	;

	vl::physics::CylinderShapeRefPtr (*cyl_create_0)(vl::scalar, vl::scalar) = &vl::physics::CylinderShape::create;
	vl::physics::CylinderShapeRefPtr (*cyl_create_1)(Ogre::Vector3 const &) = &vl::physics::CylinderShape::create;

	python::class_<vl::physics::CylinderShape, boost::noncopyable, vl::physics::CylinderShapeRefPtr, python::bases<vl::physics::CollisionShape> >("CylinderShape", python::no_init )
		.def("create", cyl_create_0)
		.def("create", cyl_create_1)
		.staticmethod("create")
	;

	python::class_<vl::physics::CapsuleShape, boost::noncopyable, vl::physics::CapsuleShapeRefPtr, python::bases<vl::physics::CollisionShape> >("CapsuleShape", python::no_init )
		.def("create", &vl::physics::CapsuleShape::create)
		.staticmethod("create")
	;

	/// Abstract master class for all physics constraints
	python::class_<vl::physics::Constraint, vl::physics::ConstraintRefPtr, boost::noncopyable>("Constraint", python::no_init)
	;
	
	python::class_<vl::physics::Motor3Dof, boost::noncopyable>("Motor", python::no_init)
		//Constraint lowerlimit:
		.add_property("lower_limit",&vl::physics::Motor3Dof::getLowerLimit,&vl::physics::Motor3Dof::setLowerLimit)
		.add_property("upper_limit",&vl::physics::Motor3Dof::getUpperLimit,&vl::physics::Motor3Dof::setUpperLimit)
		.add_property("limit_softness",&vl::physics::Motor3Dof::getLimitSoftness,&vl::physics::Motor3Dof::setLimitSoftness)	
		.add_property("damping",&vl::physics::Motor3Dof::getDamping,&vl::physics::Motor3Dof::setDamping)
		.add_property("restitution",&vl::physics::Motor3Dof::getRestitution,&vl::physics::Motor3Dof::setRestitution)	
		.add_property("normal_CFM",&vl::physics::Motor3Dof::getNormalCFM,&vl::physics::Motor3Dof::setNormalCFM)	
		.add_property("stop_ERP",&vl::physics::Motor3Dof::getStopERP,&vl::physics::Motor3Dof::setStopERP)	
		.add_property("stop_CFM",&vl::physics::Motor3Dof::getStopCFM,&vl::physics::Motor3Dof::setStopCFM)
		.add_property("target_velocity",&vl::physics::Motor3Dof::getTargetVelocity,&vl::physics::Motor3Dof::setTargetVelocity)
		.add_property("max_motor_force",&vl::physics::Motor3Dof::getMaxMotorForce,&vl::physics::Motor3Dof::setMaxMotorForce)
		.add_property("max_limit_torque",&vl::physics::Motor3Dof::getMaxLimitTorque,&vl::physics::Motor3Dof::setMaxLimitTorque)
		.add_property("lock_hack",&vl::physics::Motor3Dof::isLockingEnabled, &vl::physics::Motor3Dof::enableLocking)

		.def("enable_motor", &vl::physics::Motor3Dof::enableMotor)
		.def("disable_motor", &vl::physics::Motor3Dof::disableMotor)
		.def("enable_motors", &vl::physics::Motor3Dof::enableAllMotors)
		.def("disable_motors", &vl::physics::Motor3Dof::disableAllMotors)
	;


	/// 6dof constraint
	python::class_<vl::physics::SixDofConstraint, vl::physics::SixDofConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("PSixDofConstraint", python::no_init)
		/// @todo change to properties
		.def("setLinearLowerLimit", &vl::physics::SixDofConstraint::setLinearLowerLimit)
		.def("setLinearUpperLimit", &vl::physics::SixDofConstraint::setLinearUpperLimit)
		.def("setAngularLowerLimit", &vl::physics::SixDofConstraint::setAngularLowerLimit)
		.def("setAngularUpperLimit", &vl::physics::SixDofConstraint::setAngularUpperLimit)
		.def("setDamping", &vl::physics::SixDofConstraint::setDamping)
		.def("setNormalCFM", &vl::physics::SixDofConstraint::setNormalCFM)
		.def("setStiffness", &vl::physics::SixDofConstraint::setStiffness)
		.def("setStopCFM", &vl::physics::SixDofConstraint::setStopCFM)
		.def("setStopERP", &vl::physics::SixDofConstraint::setStopERP)
		.add_property("angular_upper_limit", &vl::physics::SixDofConstraint::getAngularUpperLimit, &vl::physics::SixDofConstraint::setAngularUpperLimit)
		.add_property("angular_lower_limit", &vl::physics::SixDofConstraint::getAngularLowerLimit, &vl::physics::SixDofConstraint::setAngularLowerLimit)
		.add_property("linear_upper_limit", &vl::physics::SixDofConstraint::getLinearUpperLimit, &vl::physics::SixDofConstraint::setLinearUpperLimit)
		.add_property("linear_lower_limit", &vl::physics::SixDofConstraint::getLinearLowerLimit, &vl::physics::SixDofConstraint::setLinearLowerLimit)
		.add_property("angle", &vl::physics::SixDofConstraint::getCurrentAngle)
		.add_property("position", &vl::physics::SixDofConstraint::getCurrentPosition)
		.add_property("bodyA", &vl::physics::SixDofConstraint::getBodyA)
		.add_property("bodyB", &vl::physics::SixDofConstraint::getBodyB)
		.add_property("rotation_motor", python::make_function(&vl::physics::SixDofConstraint::getRotationalMotor, python::return_value_policy<python::reference_existing_object>()))
		.add_property("translation_motor", python::make_function(&vl::physics::SixDofConstraint::getTranslationalMotor, python::return_value_policy<python::reference_existing_object>())) 
		
		.def(python::self_ns::str(python::self_ns::self))
		.def("create", &vl::physics::SixDofConstraint::createDynamic)
		
		.staticmethod("create")
	;

	/// slider constraint
	python::class_<vl::physics::SliderConstraint, vl::physics::SliderConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("PSliderConstraint", python::no_init)
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
		.def(python::self_ns::str(python::self_ns::self))
		.def("create", &vl::physics::SliderConstraint::createDynamic)
		.staticmethod("create")
	;

	/// hinge constraint
	python::class_<vl::physics::HingeConstraint, vl::physics::HingeConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("PHingeConstraint", python::no_init)
		// Limits
		//.add_property("lower_lin_limit", &vl::physics::HingeConstraint::getLowerLinLimit, &vl::physics::HingeConstraint::setLowerLinLimit)
		//.add_property("upper_lin_limit", &vl::physics::HingeConstraint::getUpperLinLimit, &vl::physics::HingeConstraint::setUpperLinLimit)
		//.add_property("lower_ang_limit", &vl::physics::HingeConstraint::getLowerAngLimit, &vl::physics::HingeConstraint::setLowerAngLimit)
		//.add_property("upper_ang_limit", &vl::physics::HingeConstraint::getUpperAngLimit, &vl::physics::HingeConstraint::setUpperAngLimit)
		.def("set_limit", &vl::physics::HingeConstraint::setLimit, setLimit_ovs())
		// Motor
		.def("enable_motor", &vl::physics::HingeConstraint::enableMotor)
		.def("set_motor_target", &vl::physics::HingeConstraint::setMotorTarget)
		.def("set_motor_max_impulse", &vl::physics::HingeConstraint::setMaxMotorImpulse)
		.def("set_axis", &vl::physics::HingeConstraint::setAxis)
		
		.def(python::self_ns::str(python::self_ns::self))
		.def("create", &vl::physics::HingeConstraint::createDynamic)
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
		.def("set_sleeping_thresholds",&vl::physics::RigidBody::setSleepingThresholds)
		.add_property("total_force", &vl::physics::RigidBody::getTotalForce )
		.add_property("total_torque", &vl::physics::RigidBody::getTotalTorque )
		.add_property("center_of_mass_transform", &vl::physics::RigidBody::getCenterOfMassTransform, &vl::physics::RigidBody::setCenterOfMassTransform)
		.add_property("linear_damping", &vl::physics::RigidBody::getLinearDamping, &vl::physics::RigidBody::setLinearDamping)
		.add_property("angular_damping", &vl::physics::RigidBody::getAngularDamping, &vl::physics::RigidBody::setAngularDamping)
		.add_property("linear_velocity", &vl::physics::RigidBody::getLinearVelocity, &vl::physics::RigidBody::setLinearVelocity)
		.add_property("angular_velocity", &vl::physics::RigidBody::getAngularVelocity, &vl::physics::RigidBody::setAngularVelocity)
		.add_property("world_transformation", &vl::physics::RigidBody::getWorldTransform, &vl::physics::RigidBody::setWorldTransform)
		.add_property("shape", &vl::physics::RigidBody::getShape)
		.add_property("user_controlled", &vl::physics::RigidBody::isUserControlled, &vl::physics::RigidBody::setUserControlled)
		.add_property("motion_state", python::make_function(getMotionState_ov1, python::return_value_policy<python::reference_existing_object>()), &vl::physics::RigidBody::setMotionState )
		.add_property("name", python::make_function(&vl::physics::RigidBody::getName, python::return_value_policy<python::copy_const_reference>()) )
		.add_property("mass", &vl::physics::RigidBody::getMass, &vl::physics::RigidBody::setMass)
		.add_property("inertia", &vl::physics::RigidBody::getInertia, &vl::physics::RigidBody::setInertia)
		.add_property("kinematic", &vl::physics::RigidBody::isKinematicObject, &vl::physics::RigidBody::enableKinematicObject)
		.add_property("disable_collisions", &vl::physics::RigidBody::isCollisionsDisabled, &vl::physics::RigidBody::disableCollisions)
		.add_property("anisotropic_friction", &vl::physics::RigidBody::getAnisotropicFriction, &vl::physics::RigidBody::setAnisotropicFriction)
		.add_property("friction", &vl::physics::RigidBody::getFriction, &vl::physics::RigidBody::setFriction)
		.def(python::self_ns::str(python::self_ns::self))
	;

//	vl::Transform (vl::physics::MotionState::*getWorldTransform_ov0)(void) const = &vl::physics::MotionState::getWorldTransform;
//	void (vl::physics::MotionState::*setWorldTransform_ov0)(vl::Transform const &) = &vl::physics::MotionState::setWorldTransform;
	
	// @warning: added due rayresults, this is a thingy called vectorlist (list of vector3 refs)
	
	python::class_< std::vector<Ogre::Vector3> >("VectorList")
		.def(python::vector_indexing_suite< std::vector<Ogre::Vector3> >())
		//.def(python::self_ns::str(python::self_ns::self))
	;
	//python::class_< std::vector<std::string const *> >("NameList")
	//	.def(python::vector_indexing_suite<std::vector<std::string const *>,true >())
	//;
		// @warning: added due raycast, remove if there's problems:
	
	python::class_<vl::physics::RayResult>("RayResult")
		.def_readonly("ray_start", &vl::physics::RayResult::start_point)
		.def_readonly("ray_end", &vl::physics::RayResult::end_point)
		.def_readonly("hit_objects", &vl::physics::RayResult::hit_objects)
		.def_readonly("hit_points", &vl::physics::RayResult::hit_points_world)
		.def_readonly("hit_normals", &vl::physics::RayResult::hit_normals_world)
		.def_readonly("hit_fractions", &vl::physics::RayResult::hit_fractions)
	;
	
	// .add_property("head",
    // make_getter(&Tree::head, return_value_policy<reference_existing_object>()),
    // make_setter(&Tree::head, return_value_policy<reference_existing_object>()))


	/// motion state
	python::class_<vl::physics::MotionState, boost::noncopyable>("MotionState", python::no_init)
		.add_property("node", python::make_function( &vl::physics::MotionState::getNode, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::MotionState::setNode )
		.add_property("position", python::make_function(&vl::physics::MotionState::getPosition, python::return_value_policy<python::copy_const_reference>()), &vl::physics::MotionState::setPosition)
		.add_property("orientation", python::make_function(&vl::physics::MotionState::getOrientation, python::return_value_policy<python::copy_const_reference>()), &vl::physics::MotionState::setOrientation)
		.add_property("world_transformation", python::make_function(&vl::physics::MotionState::getWorldTransform, python::return_value_policy<python::copy_const_reference>()), &vl::physics::MotionState::setWorldTransform)
		.def("set_world_transformation", &vl::physics::MotionState::setWorldTransform)
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::physics::SolverParameters>("PhysicsSolverParameters", python::init<>())
		.def_readwrite("erp", &vl::physics::SolverParameters::erp)
		.def_readwrite("erp2", &vl::physics::SolverParameters::erp2)
		.def_readwrite("global_cfm", &vl::physics::SolverParameters::global_cfm)
		.def_readwrite("restitution", &vl::physics::SolverParameters::restitution)
		.def_readwrite("max_error_reduction", &vl::physics::SolverParameters::max_error_reduction)
		.def_readwrite("internal_time_step", &vl::physics::SolverParameters::internal_time_step)
		.def_readwrite("max_sub_steps", &vl::physics::SolverParameters::max_sub_steps)
	;

	python::class_<std::vector<boost::shared_ptr<vl::physics::Constraint> > >("ConstraintList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<vl::physics::Constraint> >, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// Shared pointer needs Proxies to be turned off
	python::class_<std::vector<boost::shared_ptr<vl::physics::SixDofConstraint> > >("SixDofConstraintList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<vl::physics::SixDofConstraint> >, true>())
		//.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<std::vector<boost::shared_ptr<vl::physics::RigidBody> > >("RigidBodyList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<vl::physics::RigidBody> >, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<std::vector<boost::shared_ptr<vl::physics::Tube> > >("TubeList")
		.def(python::vector_indexing_suite<std::vector<boost::shared_ptr<vl::physics::Tube> >, true>())
		.def(python::self_ns::str(python::self_ns::self))
	;

	/// world
	python::class_<vl::physics::World, vl::physics::WorldRefPtr, boost::noncopyable>("PhysicsWorld", python::no_init)
		.def("createRigidBody", &vl::physics::World::createDynamicRigidBody, createDynamicRigidBody_ov() )
		.def("getRigidBody", &vl::physics::World::getRigidBody)
		.def("hasRigidBody", &vl::physics::World::hasRigidBody)
		// @todo add both overloads
		//.def("removeRigidBody", &vl::physics::World::removeRigidBody)
		.def("createMotionState", &vl::physics::World::createMotionState,
			 createMotionState_ov()[ python::return_value_policy<python::reference_existing_object>() ] )
		.def("addConstraint", &vl::physics::World::addConstraint, addConstraint_ovs() )
		.def("createTube", &vl::physics::World::createTube, createTube_ov())
		.def("createTube", &vl::physics::World::createTubeEx)
		.add_property("bodies", python::make_function(&vl::physics::World::getBodies, python::return_value_policy<python::copy_const_reference>()))
		.add_property("tubes", python::make_function(&vl::physics::World::getTubes, python::return_value_policy<python::copy_const_reference>()))
		.add_property("constraints", python::make_function(&vl::physics::World::getConstraints, python::return_value_policy<python::copy_const_reference>()))
		.add_property("gravity", &vl::physics::World::getGravity, &vl::physics::World::setGravity )
		.add_property("collision_detection_enabled", &vl::physics::World::isCollisionDetectionEnabled, &vl::physics::World::enableCollisionDetection)
		.add_property("solver_parameters", python::make_function(&vl::physics::World::getSolverParameters, python::return_value_policy<python::copy_const_reference>()),
				&vl::physics::World::setSolverParameters)
		.def(python::self_ns::str(python::self_ns::self))
		
		// @warning: this was added due ray cast testing, first commented line is used when returning a reference to rayresults:
		//.def("castRay", &vl::physics::World::castRay, python::return_value_policy<python::reference_existing_object>())
		.def("cast_ray", &vl::physics::World::castRay)
		;

	python::class_<vl::physics::Tube::ConstructionInfo>("TubeConstructionInfo", python::init<>())
		.def_readwrite("start_body", &vl::physics::Tube::ConstructionInfo::start_body)
		.def_readwrite("end_body", &vl::physics::Tube::ConstructionInfo::end_body)
		.def_readwrite("start_frame", &vl::physics::Tube::ConstructionInfo::start_body_frame)
		.def_readwrite("end_frame", &vl::physics::Tube::ConstructionInfo::end_body_frame)
		.def_readwrite("length", &vl::physics::Tube::ConstructionInfo::length)
		.def_readwrite("radius", &vl::physics::Tube::ConstructionInfo::radius)
		.def_readwrite("mass_per_meter", &vl::physics::Tube::ConstructionInfo::mass_per_meter)
		.def_readwrite("stiffness", &vl::physics::Tube::ConstructionInfo::stiffness)
		.def_readwrite("damping", &vl::physics::Tube::ConstructionInfo::damping)
		.def_readwrite("element_size", &vl::physics::Tube::ConstructionInfo::element_size)
		.def_readwrite("material_name", &vl::physics::Tube::ConstructionInfo::material_name)
		.def_readwrite("upper_lim", &vl::physics::Tube::ConstructionInfo::upper_lim)
		.def_readwrite("lower_lim", &vl::physics::Tube::ConstructionInfo::lower_lim)
		.def_readwrite("fixing_upper_lim", &vl::physics::Tube::ConstructionInfo::fixing_upper_lim)
		.def_readwrite("fixing_lower_lim", &vl::physics::Tube::ConstructionInfo::fixing_lower_lim)
		.def_readwrite("spring", &vl::physics::Tube::ConstructionInfo::spring)
		.def_readwrite("inertia_factor", &vl::physics::Tube::ConstructionInfo::inertia_factor)
		.def_readwrite("disable_collisions", &vl::physics::Tube::ConstructionInfo::disable_collisions)
		.def_readwrite("disable_internal_collisions", &vl::physics::Tube::ConstructionInfo::disable_internal_collisions)
		.def_readwrite("body_damping", &vl::physics::Tube::ConstructionInfo::body_damping)
		.def_readwrite("bending_radius", &vl::physics::Tube::ConstructionInfo::bending_radius)
		.def_readwrite("use_instancing", &vl::physics::Tube::ConstructionInfo::use_instancing)
	;


	python::class_<vl::physics::Tube, vl::physics::TubeRefPtr, boost::noncopyable>("Tube", python::no_init)
		.add_property("spring_stiffness", &vl::physics::Tube::getSpringStiffness, &vl::physics::Tube::setSpringStiffness)
		.add_property("spring_damping", &vl::physics::Tube::getSpringDamping, &vl::physics::Tube::setSpringDamping)
		.add_property("mass", &vl::physics::Tube::getMass, &vl::physics::Tube::setMass)
		.add_property("body_damping", &vl::physics::Tube::getDamping, &vl::physics::Tube::setDamping)
		.add_property("lower_limit", python::make_function(&vl::physics::Tube::getLowerLim, python::return_value_policy<python::copy_const_reference>()), &vl::physics::Tube::setLowerLim)
		.add_property("upper_limit", python::make_function(&vl::physics::Tube::getUpperLim, python::return_value_policy<python::copy_const_reference>()), &vl::physics::Tube::setUpperLim)
		.add_property("material", python::make_function(&vl::physics::Tube::getMaterial, python::return_value_policy<python::copy_const_reference>()), &vl::physics::Tube::setMaterial)
		.add_property("show_bounding_boxes", &vl::physics::Tube::isShowBoundingBoxes, &vl::physics::Tube::setShowBoundingBoxes)
		// Setters here would need to modify the meshes, both physics and graphics
		.add_property("element_size", &vl::physics::Tube::getElementSize)
		.add_property("radius", &vl::physics::Tube::getRadius)
		.add_property("leght", &vl::physics::Tube::getLength)
		.def("hide", &vl::physics::Tube::hide)
		.def("show", &vl::physics::Tube::show)
		.def("set_equilibrium", &vl::physics::Tube::setEquilibrium)
		.def("add_fixing", &vl::physics::Tube::addFixingPoint, addFixingPoint_ovs())
		.def("remove_fixing", &vl::physics::Tube::removeFixingPoint)
		.def("get_fixing", &vl::physics::Tube::getFixing)
		.add_property("n_fixings", &vl::physics::Tube::getNFixings)
		.add_property("fixings", python::make_function(&vl::physics::Tube::getFixings, python::return_value_policy<python::copy_const_reference>()))
		.add_property("bodies", python::make_function(&vl::physics::Tube::getBodies, python::return_value_policy<python::copy_const_reference>()))
		.add_property("start_fixing", &vl::physics::Tube::getStartFixing)
		.add_property("end_fixing", &vl::physics::Tube::getEndFixing)
		.def(python::self_ns::str(python::self_ns::self))
		.def("create", &vl::physics::Tube::create)
	;
}

}

/// Physics
/// @todo physics should be in hydra.physics
/// needs a package strucutre
void export_physics(void)
{
	export_physics_objects();
}

