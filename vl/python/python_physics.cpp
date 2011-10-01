/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file python/python_physics.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

/// Interface
#include "python_module.hpp"

#include "typedefs.hpp"

// Physics
#include "physics/physics_events.hpp"
#include "physics/physics_world.hpp"
#include "physics/rigid_body.hpp"
#include "physics/shapes.hpp"
#include "physics/physics_constraints.hpp"
#include "physics/tube.hpp"
#include "physics/motion_state.hpp"

/// Physics world member overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createRigidBody_ov, createRigidBody, 4, 5 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( addConstraint_ovs, addConstraint, 1, 2 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( setLimit_ovs, setLimit, 2, 5 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createMotionState_ov, createMotionState, 0, 2 )
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( createTube_ov, createTube, 3, 5 )

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

	/// Abstract master class for all physics constraints
	python::class_<vl::physics::Constraint, vl::physics::ConstraintRefPtr, boost::noncopyable>("Constraint", python::no_init)
	;

	/// 6dof constraint
	python::class_<vl::physics::SixDofConstraint, vl::physics::SixDofConstraintRefPtr, python::bases<vl::physics::Constraint>, boost::noncopyable>("PSixDofConstraint", python::no_init)
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
		.def("create", &vl::physics::SliderConstraint::create)
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
		.add_property("world_transformation", &vl::physics::RigidBody::getWorldTransform, &vl::physics::RigidBody::setWorldTransform)
		.add_property("shape", &vl::physics::RigidBody::getShape)
		.add_property("user_controlled", &vl::physics::RigidBody::isUserControlled, &vl::physics::RigidBody::setUserControlled)
		.add_property("motion_state", python::make_function(getMotionState_ov1, python::return_value_policy<python::reference_existing_object>()), &vl::physics::RigidBody::setMotionState )
		.add_property("name", python::make_function(&vl::physics::RigidBody::getName, python::return_value_policy<python::copy_const_reference>()) )
		.add_property("mass", &vl::physics::RigidBody::getMass, &vl::physics::RigidBody::setMass)
		.def(python::self_ns::str(python::self_ns::self))
	;

	vl::Transform (vl::physics::MotionState::*getWorldTransform_ov0)(void) const = &vl::physics::MotionState::getWorldTransform;

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
		.def("createTube", &vl::physics::World::createTube, createTube_ov())
		.def("createTube", &vl::physics::World::createTubeEx)
		.add_property("gravity", &vl::physics::World::getGravity, &vl::physics::World::setGravity )
		.def(python::self_ns::str(python::self_ns::self))
	;

	python::class_<vl::physics::Tube::ConstructionInfo>("TubeConstructionInfo", python::init<>())
		.def_readwrite("start_body", &vl::physics::Tube::ConstructionInfo::start_body)
		.def_readwrite("end_body", &vl::physics::Tube::ConstructionInfo::end_body)
		.def_readwrite("start_frame", &vl::physics::Tube::ConstructionInfo::start_body_frame)
		.def_readwrite("end_frame", &vl::physics::Tube::ConstructionInfo::end_body_frame)
		.def_readwrite("length", &vl::physics::Tube::ConstructionInfo::length)
		.def_readwrite("radius", &vl::physics::Tube::ConstructionInfo::radius)
		.def_readwrite("mass", &vl::physics::Tube::ConstructionInfo::mass)
		.def_readwrite("stiffness", &vl::physics::Tube::ConstructionInfo::stiffness)
		.def_readwrite("damping", &vl::physics::Tube::ConstructionInfo::damping)
		.def_readwrite("element_size", &vl::physics::Tube::ConstructionInfo::element_size)
		.def_readwrite("material_name", &vl::physics::Tube::ConstructionInfo::material_name)
		.def_readwrite("upper_lim", &vl::physics::Tube::ConstructionInfo::upper_lim)
		.def_readwrite("lower_lim", &vl::physics::Tube::ConstructionInfo::lower_lim)
		.def_readwrite("spring", &vl::physics::Tube::ConstructionInfo::spring)
		.def_readwrite("inertia", &vl::physics::Tube::ConstructionInfo::inertia)
		.def_readwrite("disable_collisions", &vl::physics::Tube::ConstructionInfo::disable_collisions)
		.def_readwrite("body_damping", &vl::physics::Tube::ConstructionInfo::body_damping)
	;

	python::class_<vl::physics::Tube, vl::physics::TubeRefPtr, boost::noncopyable>("Tube", python::no_init)
		.add_property("stiffness", &vl::physics::Tube::getStiffness)
		.add_property("element_size", &vl::physics::Tube::getElementSize)
		.add_property("radius", &vl::physics::Tube::getRadius)
		.add_property("leght", &vl::physics::Tube::getLength)
		.add_property("mass", &vl::physics::Tube::getMass)
		.def("hide", &vl::physics::Tube::hide)
		.def("show", &vl::physics::Tube::show)
		.def("set_equilibrium", &vl::physics::Tube::setEquilibrium)
	;
}

void export_physics_actions(void)
{
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

	python::class_<vl::physics::ApplyForce, boost::noncopyable, python::bases<vl::BasicAction> >("ApplyForce", python::no_init )
		.add_property("body", python::make_function( &vl::physics::ApplyForce::getRigidBody, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::ApplyForce::setRigidBody )
		.add_property("force", python::make_function( &vl::physics::ApplyForce::getForce, python::return_value_policy<python::copy_const_reference>() ),
					  &vl::physics::ApplyForce::setForce )
		.add_property("local", &vl::physics::ApplyForce::getLocal, &vl::physics::ApplyForce::setLocal)
		.def("create",&vl::physics::ApplyForce::create,
			 python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;

	python::class_<vl::physics::ApplyTorque, boost::noncopyable, python::bases<vl::BasicAction> >("ApplyTorque", python::no_init )
		.add_property("body", python::make_function( &vl::physics::ApplyTorque::getRigidBody, python::return_value_policy< python::reference_existing_object>() ),
					  &vl::physics::ApplyTorque::setRigidBody )
		.add_property("torque", python::make_function( &vl::physics::ApplyTorque::getTorque, python::return_value_policy<python::copy_const_reference>() ),
					  &vl::physics::ApplyTorque::setTorque )
		.def("create",&vl::physics::ApplyTorque::create,
			 python::return_value_policy<python::reference_existing_object>() )
		.staticmethod("create")
	;
}

}

/// Physics
/// @todo physics should be in hydra.physics
/// needs a package strucutre
void export_physics(void)
{
	export_physics_objects();
	export_physics_actions();
}
