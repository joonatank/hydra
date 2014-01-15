# -*- coding: utf-8 -*-

import math

# Setup player
camera_name = "Camera"
camera = game.scene.createSceneNode(camera_name)
camera.position = Vector3(-0.77, 0, 4)
cam = game.scene.createCamera(camera_name)
camera.attachObject(cam)
create_camera_controller()
game.player.camera = camera_name

# Setup environment
create_sun()

ground = physics_create_ground()

# Enable collision detection before creating any bodies
# Loading a HSF file should work with this enabled but it uses the attributes
# in the XML file instead of the auto creation of KinematicWorld
#game.kinematic_world.collision_detection_enabled = True

zoom = game.kinematic_world.get_constraint("zoom")
nosto = game.kinematic_world.get_constraint("nosto")
kaanto = game.kinematic_world.get_constraint("kaanto")
pulttaus = game.kinematic_world.get_constraint("pulttaus")
motor = game.kinematic_world.get_constraint("motor")

assert(zoom and nosto and kaanto and pulttaus and motor)

# Hide links
# TODO this should be in the HSF file
#game.scene.hideSceneNodes("nivel*")

# TODO this needs support for naming constraints
#pulttaus_hinge = game.kinematics_world.getConstraint("pulttaus")

# Joystick control
# Falls back to game joysticks, if none exists the movements
# are just disabled...
try :
	left_joy = game.event_manager.getJoystick("COM4:0")
	right_joy = game.event_manager.getJoystick("COM4:1")
	left_joy.zero_size = 0.1
	right_joy.zero_size = 0.1

	joy_handler = ConstraintJoystickHandler.create()
	joy_handler.velocity_multiplier = 0.4
	joy_handler.set_axis_constraint(zoom, 1)
	joy_handler.set_axis_constraint(pulttaus, 0)
	joy_handler.set_axis_constraint(motor, 1, 0)
	left_joy.add_handler(joy_handler)

	joy_handler = ConstraintJoystickHandler.create()
	joy_handler.velocity_multiplier = 0.4
	joy_handler.set_axis_constraint(nosto, 1)
	joy_handler.set_axis_constraint(kaanto, 0)
	right_joy.add_handler(joy_handler)
except :
	# Lets just assume that the serial port doesn't exist as the c++
	# exceptions has not been exposed to python
	# Should expose the exceptions as it can throw because there was no such
	# serial port and because there wasn't enough joysticks in there.
	print("No COM4 serial port. Creating GameJoystick interface.")
	# Create game joystick
	# The game joystick needs completely different mapping and is
	# alone where the serial joysticks are together
	joy = game.event_manager.getJoystick()
	joy.zero_size = 0.1

	joy_handler = ConstraintJoystickHandler.create()
	joy_handler.velocity_multiplier = 0.4
	joy_handler.set_axis_constraint(nosto, 1)
	joy_handler.set_axis_constraint(kaanto, 0)
	joy_handler.set_axis_constraint(zoom, 1, 0)
	joy_handler.set_axis_constraint(pulttaus, 0, 0)
	joy_handler.set_axis_constraint(motor, 1, 1)
	joy.add_handler(joy_handler)

def boom_up():
	nosto.target = Radian(-1)
	nosto.speed = Radian(0.5)

def boom_down():
	nosto.target = Radian(0)
	nosto.speed = Radian(0.5)

def boom_incorrect_down():
	nosto.target = Radian(-1)
	nosto.speed = Radian(0.5)

# For testing the kinematic response
trigger = game.event_manager.createKeyTrigger(KC.F)
trigger.addKeyDownListener(boom_up)

trigger = game.event_manager.createKeyTrigger(KC.G)
trigger.addKeyDownListener(boom_down)

trigger = game.event_manager.createKeyTrigger(KC.H)
trigger.addKeyDownListener(boom_down)


# TODO tubes are not working with this version
"""
# Add tube simulation
# Tie the ends to SceneNodes
tube_info = TubeConstructionInfo()
# Some test bodies
# TODO create the bodies using the SceneNodes as references
shape = BoxShape.create(Vector3(0.1, 0.1, 0.1))
ms = game.physics_world.createMotionState(kaantokappale.world_transformation, kaantokappale)
start_body = game.physics_world.createRigidBody('kaantokappale', 0, ms, shape)
ms = game.physics_world.createMotionState(vaantomoottori.world_transformation, vaantomoottori)
end_body = game.physics_world.createRigidBody('vaantomoottori', 0, ms, shape)
tube_info.start_body = start_body
tube_info.end_body = end_body 
tube_info.start_frame = Transform(Vector3(0.4, 0, 0), Quaternion(0.7071, 0.7071, 0, 0))
tube_info.end_frame = Transform(Vector3(0, 0, 0), Quaternion(1., 0., 0, 0))
# Leght between vaantomoottori and kaantokappale is bit over 3m
tube_info.length = 4
tube_info.mass = 50
tube_info.radius = 0.05
#tube_info.stiffness = 0.6
#tube_info.damping = 0.3
tube_info.element_size = 0.1

tube = game.physics_world.createTube(tube_info)
tube.create()

def setBodyTransform(t):
	end_body.world_transform = t

vaantomoottori.addListener(setBodyTransform)
"""

# Add some primitives and possibility to move them for testing the collision
# detection
pos = Vector3(-3, 3, -0.5)
box = addBox("user_box", "finger_sphere/green", pos, mass=10)
box.user_controlled = True
#addRigidBodyController(box)

# Static box
pos = Vector3(-2, 0.75, -2.5)
box = addBox("static_box", "finger_sphere/blue", pos, mass=0)

game.auto_start = False

trigger = game.event_manager.createKeyTrigger(KC.SPACE)
trigger.addKeyUpListener(toggle_pause)

# TODO test with separate directory
#game.save_scene("test.hsf")

# TODO the loader should work with absolute path names
# We need to fix the resource manager to load with absolute paths
hsf_file = "C:/jotu/software_development/hydra_source/hydra_0.4/data/hsf_test_project/resources/perapora_new.hsf"

hsf_name = "perapora_new.hsf"

def reload() :
	game.load_scene(hsf_name, LOADER_FLAG.OVERWRITE)

trigger = game.event_manager.createKeyTrigger(KC.F5)
trigger.addKeyUpListener(reload)

