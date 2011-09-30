# -*- coding: utf-8 -*-

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
#spot.position = camera.position + Vector3(0, 1.5, 0)
spot.position = Vector3(-5, 25, 20)

light = game.scene.createSceneNode("light")
light_l = game.scene.createLight("light")
light_l.diffuse = ColourValue(0.6, 0.6, 0.8)
light.attachObject(light_l)
light.position = Vector3(0, 20, -5)


# Create physics
game.enablePhysics( True )
world = game.physics_world

ground_node = game.scene.createSceneNode("ground")
ground = game.scene.createEntity("ground", PF.PLANE)
ground_node.attachObject(ground)
ground.material_name = "ground/bump_mapped/shadows"
ground.cast_shadows = False

print('Physics : Adding ground plane')
ground_mesh = game.mesh_manager.loadMesh("prefab_plane")
ground_shape = StaticTriangleMeshShape.create(ground_mesh)
g_motion_state = world.createMotionState(Transform(Vector3(0, 0, 0)), ground_node)
ground_body = world.createRigidBody('ground', 0, g_motion_state, ground_shape)

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

