# -*- coding: utf-8 -*-

# Project specific definitions, like object creations

print('Creating Camera SceneNode')
camera_n = game.scene.createSceneNode("Camera")
camera = game.scene.createCamera("Camera")
camera_n.attachObject(camera)
createCameraMovements()
camera_n.position = Vector3(0, 5, 20)

game.player.camera = "Camera"

game.scene.sky_dome = SkyDomeInfo("CloudySky")
game.scene.shadows.enable()
game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)

# Create physics
# TODO these should be in the project file
game.enablePhysics( True )
world = game.physics_world

create_sun()


ground = physics_create_ground()

# TODO add some boxes
box1 = addBox("box1", "finger_sphere/blue", Vector3(5.0, 1, -5), mass=10)
# These comments are probably not the case anymore because we use primitives now
# FIXME Mass 30 causes the box to go through the ground plane
# Also size 3 causes lots of accuracy problems in the collision detection
# size 2 causes less accuracy problems but still does, probably a problem
# with scaling a ConvexHull
box2 = addBox("box2", "finger_sphere/blue", Vector3(-5.0, 10, -5), size=Vector3(1, 1, 1), mass=20)

user_sphere = addSphere("user_sphere", "finger_sphere/blue", Vector3(5.0, 20, 0), 20)
user_sphere.user_controlled = True


sphere_fixed = addSphere("sphere_fixed", "finger_sphere/green", Vector3(-5.0, 5, 0), 0)

sphere_3_body = addSphere("sphere3", "finger_sphere/red", Vector3(3, 7, 3))
constraint = PSliderConstraint.create(user_sphere, sphere_fixed, Transform(), Transform(), False)
constraint.lower_lin_limit = -5
constraint.upper_lin_limit = 5
#constraint.lower_ang_limit = 0
#constraint.upper_ang_limit = 1
#world.addConstraint(constraint)

six_dof = PSixDofConstraint.create(user_sphere, sphere_fixed, Transform(), Transform(), False)
six_dof.setLinearLowerLimit(Vector3(-10, -5, -5))
six_dof.setLinearUpperLimit(Vector3(10, 5, 5))
#six_dof.setAngularLowerLimit(Vector3(1, 1, 1))
#six_dof.setAngularUpperLimit(Vector3(1, 1, 1))
#world.addConstraint(six_dof)

# Add force action
def force_to_sphere():
	user_sphere.applyForce(Vector3(0, 2500, 0), Vector3(0,0,0))

def torque_to_sphere():
	user_sphere.applyTorque(Vector3(0, 500, 0))

def set_sphere_velocity():
	user_sphere.setLinearVelocity(Vector3(1, 0, 0))

print('Adding Force action to KC_F')
trigger = game.event_manager.createKeyTrigger( KC.F )
trigger.addListener(force_to_sphere)

print('Adding Torque action to KC_G')
trigger = game.event_manager.createKeyTrigger( KC.G )
trigger.addListener(torque_to_sphere)

print('Adding set Liner velocity action to KC_T')
trigger = game.event_manager.createKeyTrigger(KC.T)
trigger.addListener(set_sphere_velocity)

addRigidBodyController(user_sphere)

cylinder = addCylinder('cylinder', "finger_sphere/green", Vector3(.0, 5, -5), mass=10)

capsule = addCapsule('capsule', "finger_sphere/green", Vector3(.0, 5, 5), mass=10)

print('Creating a tube')
tube_info = TubeConstructionInfo()
tube_info.start_body = user_sphere
tube_info.end_body = sphere_fixed
tube_info.start_frame = Transform(Vector3(0, 0, 0.5))
tube_info.end_frame = Transform(Vector3(0, 0, -0.5))
tube_info.length = 8
tube_info.mass = 50
tube_info.radius = 0.1
tube_info.spring = True
tube_info.stiffness = 70
tube_info.bending_radius = 0.3
#tube_info.damping = 0.3
#tube_info.body_damping = 0.2
# TODO should enable collision detection
# TODO the tube should be stiffer

tube = game.physics_world.createTube(tube_info)
tube.create()

