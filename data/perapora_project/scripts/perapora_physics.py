# -*- coding: utf-8 -*-

# Create a translation joint between two bodies along z-axis
# min and max defines the freedom of the joint in meters
# Depending on wether the original coordinates have positive or negative
# z forward, you need to use positive or negative velocity for driving
# positive z forward positive velocity
def createTranslationConstraint(body0, body1, transform, min, max, disableCollision = True) :
	# SliderConstraint works on x-axis, so we rotate the reference
	# z-axis to x-axis
	trans = transform*Transform(Quaternion(-0.7071, 0, 0, 0.7071))
	local0_trans = body0.transform_to_local(trans)
	local1_trans = body1.transform_to_local(trans)
	constraint = PSliderConstraint.create(body0, body1, local0_trans, local1_trans, False)
	constraint.lower_lin_limit = min
	constraint.upper_lin_limit = max
	constraint.lower_ang_limit = 0
	constraint.upper_ang_limit = 0
	game.physics_world.addConstraint(constraint, disableCollision)
	return constraint

# Define a hinge constraint using objects +y axis
# +y because the objects were modeled in Blender with +z as the rotation axis
# but the exporter flips y and z
def createHingeConstraint(body0, body1, transform, disableCollision = True) :
	trans = transform*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	local0_trans = body0.transform_to_local(trans)
	local1_trans = body1.transform_to_local(trans)
	constraint = PHingeConstraint.create(body0, body1, local0_trans, local1_trans, False)
	game.physics_world.addConstraint(constraint, disableCollision)
	return constraint

camera_name = "Camera"
camera = game.scene.createSceneNode(camera_name)
camera.position = Vector3(-0.77, 0, 4)
cam = game.scene.createCamera(camera_name)
camera.attachObject(cam)
createCameraMovements(camera)
game.player.camera = camera_name

game.scene.sky = SkyDomeInfo("CloudySky")

# Create light
spot = game.scene.createSceneNode("spot")
spot_l = game.scene.createLight("spot")
# does not work for some reason
#spot_l.type = "spot"
spot_l.diffuse = ColourValue(1.0, 1.0, 0.6)
spot.attachObject(spot_l)
#spot.orientation = Quaternion(0, 0, 0.7071, 0.7071)
#camera.addChild(spot)
spot.position = camera.position + Vector3(0, 1.5, 0)

light = game.scene.createSceneNode("light")
light_l = game.scene.createLight("light")
light_l.diffuse = ColourValue(0.6, 0.6, 0.8)
light.attachObject(light_l)
light.position = Vector3(0, 20, 0)


# Create physics
game.enablePhysics( True )
world = game.physics_world

ground_node = game.scene.createSceneNode("ground")
ground = game.scene.createEntity("ground", PF.PLANE)
ground_node.attachObject(ground)
ground.material_name = "ground/Basic"
ground.cast_shadows = False

print('Physics : Adding ground plane')
ground_mesh = game.mesh_manager.loadMesh("prefab_plane")
ground_shape = StaticTriangleMeshShape.create(ground_mesh)
g_motion_state = world.createMotionState(Transform(Vector3(0, 0, 0)), ground_node)
ground_body = world.createRigidBody('ground', 0, g_motion_state, ground_shape)

print('Physics: adding kiinnityslevy joint')
kiinnityslevy = game.physics_world.getRigidBody("cb_kiinnityslevy")
ristikpl_kaantosyl = game.physics_world.getRigidBody("cb_ristikpl_kaantosyl")
nivel_klevy2_rotz = game.scene.getSceneNode("nivel_klevy2_rotz")
transform = nivel_klevy2_rotz.world_transformation
nivel_klevy2_hinge = createHingeConstraint(kiinnityslevy, ristikpl_kaantosyl, transform, True)
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
createHingeConstraint(ristikpl_kaantosyl, syl_kaanto_varsi, transform, True)
#createFixedConstraint(ristikpl_kaantosyl, syl_kaanto_varsi, transform, True)

print('Physics: adding kaanto sylinteri')
transform = syl_kaanto_varsi.world_transformation
syl_kaanto_putki = game.physics_world.getRigidBody("cb_syl_kaanto_putki")
kaanto_joint = createTranslationConstraint(syl_kaanto_varsi, syl_kaanto_putki, transform, -0.3, 0.3)

nivel_klevy1_rotz = game.scene.getSceneNode("nivel_klevy1_rotz")
transform = nivel_klevy1_rotz.world_transformation
kaantokappale = game.physics_world.getRigidBody("cb_kaantokappale")
createHingeConstraint(kiinnityslevy, kaantokappale, transform, True)
#createFixedConstraint(kiinnityslevy, kaantokappale, transform, True)

# Sylinteri nosto
nivel_sylnosto_rotz = game.scene.getSceneNode("nivel_nostosyl2_rotz")
transform = nivel_sylnosto_rotz.world_transformation
syl_nosto_varsi = game.physics_world.getRigidBody("cb_syl_nosto_varsi")
#createHingeConstraint(kaantokappale, syl_nosto_varsi, transform, True)
createFixedConstraint(kaantokappale, syl_nosto_varsi, transform, True)

syl_nosto_putki = game.physics_world.getRigidBody("cb_syl_nosto_putki")
transform = syl_nosto_varsi.world_transformation
nosto_joint = createTranslationConstraint(syl_nosto_varsi, syl_nosto_putki, transform, -0.3, 0.3)

# Ulkoputki constraints:
# kaantokappale, nostosylinteri, kaantosylinteri and sisaputki
ulkoputki = game.physics_world.getRigidBody("cb_ulkoputki")
nivel_puomi = game.scene.getSceneNode("nivel_puomi_rotz")
#createHingeConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation, True)
createFixedConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation, True)

nivel_nostosyl1 = game.scene.getSceneNode("nivel_nostosyl1_rotz")
#createHingeConstraint(syl_nosto_putki, ulkoputki, nivel_nostosyl1.world_transformation, True)
createFixedConstraint(syl_nosto_putki, ulkoputki, nivel_nostosyl1.world_transformation, True)

nivel_kaantosyl1 = game.scene.getSceneNode("nivel_kaantosyl1_rotz")
#createHingeConstraint(syl_kaanto_putki, ulkoputki, nivel_kaantosyl1.world_transformation, True)
createFixedConstraint(syl_kaanto_putki, ulkoputki, nivel_kaantosyl1.world_transformation, True)

sisaputki = game.physics_world.getRigidBody("cb_sisaputki")
nivel_telesk = game.scene.getSceneNode("nivel_telesk_trz")
teleskooppi_joint = createTranslationConstraint(ulkoputki, sisaputki, nivel_telesk.world_transformation, -1, 1)

# enable motors for cylinders
kaanto_joint.powered_lin_motor = True
kaanto_joint.max_lin_motor_force = 200
nosto_joint.powered_lin_motor = True
nosto_joint.max_lin_motor_force = 200
nosto_joint.target_lin_motor_velocity = -0.5
teleskooppi_joint.powered_lin_motor = True
teleskooppi_joint.max_lin_motor_force = 10
teleskooppi_joint.target_lin_motor_velocity = 0.5

sphere = addSphere("sphere1", "finger_sphere/blue", Vector3(5.0, 20, 0), size=0.5, mass=10)
sphere.user_controlled = True
addDynamicAction(sphere, reference=camera)


#kiinnityslevy.translate(Vector3(0, 2, 0))

# Add a motor action to nosto cylinder
# Note this action does not repeat so it's executed once for every key down 
"""
action = SliderMotorAction.create()
action.velocity = 0.01
action.constraint = nosto_joint
trigger = game.event_manager.createKeyTrigger(KC.Z)
trigger.action_down = action

action = SliderMotorAction.create()
action.velocity = -0.01
action.constraint = nosto_joint
trigger = game.event_manager.createKeyTrigger(KC.X)
trigger.action_down = action
"""

# Add force action
# This is correct
# TODO this needs a motor action
# This can not work without the motor, because the joint is alligned in
# different coordinates that the body, so if we try to use the bodies local
# coordinates we are using incorrect coordinate system
action = ApplyForce.create()
action.body = syl_nosto_putki
#action.force = Vector3(0, 0, 200)
action.force = Vector3(0, 2000, 0)
action.local = True
trigger = game.event_manager.createKeyTrigger( KC.F )
trigger.action_down = action

action = ApplyForce.create()
action.body = syl_nosto_putki
#action.force = Vector3(0, 0, -200)
action.force = Vector3(0, -2000, 0)
action.local = True
trigger = game.event_manager.createKeyTrigger( KC.G )
trigger.action_down = action

