# -*- coding: utf-8 -*-

# Setup player
camera_name = "Camera"
camera = game.scene.createSceneNode(camera_name)
camera.position = Vector3(-0.77, 0, 4)
cam = game.scene.createCamera(camera_name)
camera.attachObject(cam)
createCameraMovements(camera)
game.player.camera = camera_name

# Setup environment
game.scene.sky_dome = SkyDomeInfo("CloudySky")
game.scene.shadows.enable()
create_sun()


# Setup physics
game.enablePhysics( True )
world = game.physics_world

physics_create_ground()

print('Physics: adding kiinnityslevy joint')
kiinnityslevy = game.physics_world.getRigidBody("cb_kiinnityslevy")
ristikpl_kaantosyl = game.physics_world.getRigidBody("cb_ristikpl_kaantosyl")
nivel_klevy2 = game.scene.getSceneNode("nivel_klevy2_rotz")
transform = nivel_klevy2.world_transformation
nivel_klevy2_hinge = createHingeConstraint(kiinnityslevy, ristikpl_kaantosyl, transform, True, min=Degree(-45), max=Degree(45))
#createFixedConstraint(kiinnityslevy, ristikpl_kaantosyl, transform, True)

print('Physics: adding kaanto sylinteri joint')
# Sylinteri kaanto
# Without this cylinder the system will be lopsided :D
# Too heavy weight for the cylinder rotation to hold alone
# TODO a small positional offset in the piston relative to the tube
syl_kaanto_varsi = game.physics_world.getRigidBody("cb_syl_kaanto_varsi")
# reference joint
nivel_kaantosyl2_rotz = game.scene.getSceneNode("nivel_kaantosyl2_rotz")
transform = nivel_kaantosyl2_rotz.world_transformation
createHingeConstraint(ristikpl_kaantosyl, syl_kaanto_varsi, transform, True, min=Degree(-45), max=Degree(45))
#createFixedConstraint(ristikpl_kaantosyl, syl_kaanto_varsi, transform, True)

print('Physics: adding kaanto sylinteri')
transform = syl_kaanto_varsi.world_transformation
syl_kaanto_putki = game.physics_world.getRigidBody("cb_syl_kaanto_putki")
kaanto = createTranslationConstraint(syl_kaanto_varsi, syl_kaanto_putki, transform, -0.3, 0.6)

nivel_klevy1 = game.scene.getSceneNode("nivel_klevy1_rotz")
transform = nivel_klevy1.world_transformation
kaantokappale = game.physics_world.getRigidBody("cb_kaantokappale")
createHingeConstraint(kiinnityslevy, kaantokappale, transform, True, min=Degree(-45), max=Degree(45))
#createFixedConstraint(kiinnityslevy, kaantokappale, transform, True)

# Sylinteri nosto
nivel_sylnosto_rotz = game.scene.getSceneNode("nivel_nostosyl2_rotz")
transform = nivel_sylnosto_rotz.world_transformation
syl_nosto_varsi = game.physics_world.getRigidBody("cb_syl_nosto_varsi")
createHingeConstraint(kaantokappale, syl_nosto_varsi, transform, True, min=Degree(-45), max=Degree(45))
#createFixedConstraint(kaantokappale, syl_nosto_varsi, transform, True)

syl_nosto_putki = game.physics_world.getRigidBody("cb_syl_nosto_putki")
transform = syl_nosto_varsi.world_transformation
nosto = createTranslationConstraint(syl_nosto_varsi, syl_nosto_putki, transform, -0.3, 0.6)

# Ulkoputki constraints:
# kaantokappale, nostosylinteri, kaantosylinteri and sisaputki
ulkoputki = game.physics_world.getRigidBody("cb_ulkoputki")
nivel_puomi = game.scene.getSceneNode("nivel_puomi_rotz")
createHingeConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation, True, min=Degree(-45), max=Degree(45))
#createFixedConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation, True)

nivel_nostosyl1 = game.scene.getSceneNode("nivel_nostosyl1_rotz")
createHingeConstraint(syl_nosto_putki, ulkoputki, nivel_nostosyl1.world_transformation, True, min=Degree(-45), max=Degree(45))
#createFixedConstraint(syl_nosto_putki, ulkoputki, nivel_nostosyl1.world_transformation, True)

nivel_kaantosyl1 = game.scene.getSceneNode("nivel_kaantosyl1_rotz")
#createHingeConstraint(syl_kaanto_putki, ulkoputki, nivel_kaantosyl1.world_transformation, True, min=Degree(-45), max=Degree(45))
createFixedConstraint(syl_kaanto_putki, ulkoputki, nivel_kaantosyl1.world_transformation, True)

sisaputki = game.physics_world.getRigidBody("cb_sisaputki")
nivel_telesk = game.scene.getSceneNode("nivel_telesk_trz")
zoom = createTranslationConstraint(ulkoputki, sisaputki, nivel_telesk.world_transformation, -1, 1)

# enable motors for cylinders
kaanto.powered_lin_motor = True
kaanto.max_lin_motor_force = 200
nosto.powered_lin_motor = True
nosto.max_lin_motor_force = 200
nosto.target_lin_motor_velocity = -0.5
zoom.powered_lin_motor = True
zoom.max_lin_motor_force = 10
zoom.target_lin_motor_velocity = 0.5

sphere = addSphere("sphere1", "finger_sphere/blue", Vector3(5.0, 20, 0), radius=0.5, mass=10)
sphere.user_controlled = True
addRigidBodyController(sphere)

# Add force action, just as an example
# This does of course not work as we would like because applyForce is an
# external force.
# To correctly model the cylinders we would need to use actuators/motors.

def nosto_more_force():
	# needs two vectors as a parameter
	# (first is the force, second the local position)
	syl_nosto_putki.applyForce(Vector3(0, 2000, 0), Vector3(0, 0, 0))

def nosto_less_force():
	syl_nosto_putki.applyForce(Vector3(0, -2000, 0), Vector3(0, 0, 0))

trigger = game.event_manager.createKeyTrigger( KC.F )
trigger.addListener(nosto_more_force)

trigger = game.event_manager.createKeyTrigger( KC.G )
trigger.addListener(nosto_less_force)

