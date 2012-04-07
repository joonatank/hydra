# -*- coding: utf-8 -*-

import math

# Setup player
camera_name = "Camera"
camera = game.scene.createSceneNode(camera_name)
#camera.position = Vector3(-0.77, 0, 4)
camera.position = Vector3(-3.3, 0, -7)
camera.orientation = Quaternion(0, 0, -1, 0)
cam = game.scene.createCamera(camera_name)
camera.attachObject(cam)
createCameraMovements(camera)
game.player.camera = camera_name

# Setup environment
game.scene.sky_dome = SkyDomeInfo("CloudySky")
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
create_sun()

# For collision detection
game.enablePhysics(True)

ground = physics_create_ground()

# Enable collision detection before creating any bodies
game.kinematic_world.collision_detection_enabled = True

# Create the kinematics
#nivel_klevy2 = game.scene.getSceneNode("nivel_klevy2_rotz")

kaantokappale = _create_body("cb_kaantokappale")

# Nosto
ulkoputki = _create_body("cb_ulkoputki")
nivel_puomi = game.scene.getSceneNode("nivel_puomi_rotz")
wt = nivel_puomi.world_transformation
puomi_hinge = hinge_constraint(kaantokappale, ulkoputki, wt, min=Radian(-1), max=Radian(0))
puomi_hinge.axis = Vector3(0, 1, 0)

# Cylinder nosto
nivel_nostosyl1 = _create_body("nivel_nostosyl1_rotz")
nivel_nostosyl2 = _create_body("nivel_nostosyl2_rotz")
fixed_constraint(ulkoputki, nivel_nostosyl1)
fixed_constraint(kaantokappale, nivel_nostosyl2)

# Not creating constraints but a Cylinder handler class
syl_nosto_varsi = game.scene.getSceneNode("cb_syl_nosto_varsi")
syl_nosto_putki = game.scene.getSceneNode("cb_syl_nosto_putki")

syl_nosto = Cylinder(syl_nosto_putki, syl_nosto_varsi, nivel_nostosyl1, nivel_nostosyl2)
syl_nosto.up_axis = Vector3(0, 1, 0)



kiinnityslevy = _create_body("cb_kiinnityslevy")

# Kaanto
nivel_klevy1 = _create_body("nivel_klevy1_rotz")
wt = nivel_klevy1.world_transformation
#transform = Transform(Quaternion(0.7071, 0, 0.7071, 0)) * 
kaanto_hinge = hinge_constraint(kiinnityslevy, kaantokappale, wt, min=Radian(-1), max=Radian(1))
kaanto_hinge.axis = Vector3(0, 1, 0)

fixed_constraint(kaantokappale, nivel_klevy1)

# Cylinder kaanto
nivel_kaantosyl1 = _create_body("nivel_kaantosyl1_rotz")
nivel_kaantosyl2 = _create_body("nivel_kaantosyl2_rotz")
fixed_constraint(ulkoputki, nivel_kaantosyl1)
fixed_constraint(kaantokappale, nivel_kaantosyl2)

# Not creating constraints but a Cylinder handler class
syl_kaanto_varsi = game.scene.getSceneNode("cb_syl_kaanto_varsi")
syl_kaanto_putki = game.scene.getSceneNode("cb_syl_kaanto_putki")

syl_kaanto = Cylinder(syl_kaanto_putki, syl_kaanto_varsi, nivel_kaantosyl1, nivel_kaantosyl2)
syl_kaanto.up_axis = Vector3(0, 1, 0)



# Create constraint for keeping the cylinder piston fixed
# Adding this will screw up anything that uses SceneNodes
# We need to move that code to use KinematicBodies instead

# Zoom
sisaputki = _create_body("cb_sisaputki")
nivel_telesk = game.scene.getSceneNode("nivel_telesk_trz")
zoom = slider_constraint(ulkoputki, sisaputki, nivel_telesk.world_transformation, min=0., max=1.05)
zoom.axis = Vector3(0, 1, 0)

# Add rest of the objects with fixed joints
syl_tilt_putki = _create_body("cb_syl_tilt_putki")
syl_tilt_varsi = _create_body("cb_syl_tilt_varsi")
#nivel_tiltsyl1 = game.scene.getSceneNode("nivel_tiltsyl1_rotz")
#nivel_tiltsyl2 = game.scene.getSceneNode("nivel_tiltsyl2_rotz")
fixed_constraint(sisaputki, syl_tilt_putki)
fixed_constraint(syl_tilt_putki, syl_tilt_varsi)

tiltkappale = _create_body("cb_tiltkappale")
#nivel_tilt = game.scene.getSceneNode("nivel_tilt_rotz")
fixed_constraint(sisaputki, tiltkappale)

syl_swing_putki = _create_body("cb_syl_swing_putki")
syl_swing_varsi = _create_body("cb_syl_swing_varsi")
#nivel_swingsyl1 = game.scene.getSceneNode("nivel_swingsyl1_rotz")
fixed_constraint(sisaputki, syl_swing_putki)
fixed_constraint(syl_swing_putki, syl_swing_varsi)

# TODO add constraint to riskikappale_swing
ristikpl_swing = _create_body("cb_ristikpl_swing")
#nivel_swingsyl2 = game.scene.getSceneNode("nivel_swingsyl2_rotz")
fixed_constraint(syl_swing_putki, ristikpl_swing)

swingkappale = _create_body("cb_swingkappale")
#nivel_tiltkpl = game.scene.getSceneNode("nivel_tiltkpl_rotz")
fixed_constraint(tiltkappale, swingkappale)

vaantomoottori = _create_body("cb_vaantomot")
nivel_vaantomoottori = game.scene.getSceneNode("nivel_vaantomot_rotz")
wt = nivel_vaantomoottori.world_transformation
motor_hinge = hinge_constraint(swingkappale, vaantomoottori, wt, Radian(-math.pi), Radian(math.pi))
motor_hinge.axis = Vector3(0, 1, 0)

pulttausnivel = _create_body("cb_pulttausnivel")
fixed_constraint(vaantomoottori, pulttausnivel)

pulttaus_varsi = _create_body("cb_syl_pulttaus_varsi")
#nivel_pultsyl1 = game.scene.getSceneNode("nivel_pulttsyl1_rotz")
fixed_constraint(pulttausnivel, pulttaus_varsi)

pulttaus_putki = _create_body("cb_syl_pulttaus_putki")
#nivel_pultsyl2 = game.scene.getSceneNode("nivel_pulttsyl2_rotz")
fixed_constraint(pulttaus_varsi, pulttaus_putki)

kehto = _create_body("cb_kehto")
nivel_vaantomoottori2 = game.scene.getSceneNode("nivel_vaantomot2_rotz")
wt = nivel_vaantomoottori2.world_transformation
pulttaus_hinge = hinge_constraint(pulttausnivel, kehto, wt, min=Radian(0), max=Radian(math.pi/4))
pulttaus_hinge.axis = Vector3(0, 1, 0)

syl_syotto_putki = _create_body("cb_syl_syotto_putki")
syl_syotto_varsi = _create_body("cb_syl_syotto_varsi")
fixed_constraint(kehto, syl_syotto_putki)
fixed_constraint(syl_syotto_putki, syl_syotto_varsi)

raide = _create_body("cb_syottol_kiskot")
fixed_constraint(kehto, raide)

ohjuri_slaite = _create_body("cb_ohjuri_slaite")
kiinnike1_slaite = _create_body("cb_kiinnike1_slaite")
porakelkka = _create_body("cb_porakelkka")
porakanki = _create_body("cb_porakanki")
fixed_constraint(raide, ohjuri_slaite)
fixed_constraint(raide, kiinnike1_slaite)
fixed_constraint(raide, porakelkka)
fixed_constraint(raide, porakanki)

zoom.actuator = True
puomi_hinge.actuator = True
kaanto_hinge.actuator = True
motor_hinge.actuator = True
pulttaus_hinge.actuator = True

# Hide links
game.scene.hideSceneNodes("nivel*")

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
	joy_handler.set_axis_constraint(pulttaus_hinge, 0)
	joy_handler.set_axis_constraint(motor_hinge, 1, 0)
	left_joy.add_handler(joy_handler)

	joy_handler = ConstraintJoystickHandler.create()
	joy_handler.velocity_multiplier = 0.4
	joy_handler.set_axis_constraint(puomi_hinge, 1)
	joy_handler.set_axis_constraint(kaanto_hinge, 0)
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
	joy_handler.set_axis_constraint(puomi_hinge, 1)
	joy_handler.set_axis_constraint(kaanto_hinge, 0)
	joy_handler.set_axis_constraint(zoom, 1, 0)
	joy_handler.set_axis_constraint(pulttaus_hinge, 0, 0)
	joy_handler.set_axis_constraint(motor_hinge, 1, 1)
	joy.add_handler(joy_handler)

def boom_up():
	puomi_hinge.target = Radian(-1)
	puomi_hinge.speed = Radian(0.5)

def boom_down():
	puomi_hinge.target = Radian(0)
	puomi_hinge.speed = Radian(0.5)

def boom_incorrect_down():
	puomi_hinge.target = Radian(-1)
	puomi_hinge.speed = Radian(0.5)

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
addRigidBodyController(box)

# Static box
pos = Vector3(-2, 0.75, -2.5)
box = addBox("static_box", "finger_sphere/blue", pos, mass=0)

game.auto_start = False

trigger = game.event_manager.createKeyTrigger(KC.SPACE)
trigger.addKeyUpListener(toggle_pause)

