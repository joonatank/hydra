# -*- coding: utf-8 -*-

# Create a translation joint between two bodies along z-axis
# min and max defines the freedom of the joint in meters
# Depending on wether the original coordinates have positive or negative
# z forward, you need to use positive or negative velocity for driving
# positive z forward positive velocity
def createTranslationConstraint(body0, body1, transform, min, max) :
	# SliderConstraint works on x-axis, so we rotate the reference
	# z-axis to x-axis
	trans = transform #*Transform(Quaternion(-0.7071, 0, 0, 0.7071))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	constraint = SliderConstraint.create(body0, body1, trans)
	constraint.lower_limit = min
	constraint.upper_limit = max
	game.addConstraint(constraint)
	return constraint

# Define a hinge constraint using objects +y axis
# +y because the objects were modeled in Blender with +z as the rotation axis
# but the exporter flips y and z
def createHingeConstraint(body0, body1, transform) :
	trans = transform #*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	constraint = HingeConstraint.create(body0, body1, trans)
	game.addConstraint(constraint)
	return constraint

def createFixedConstraint(body0, body1, transform) :
	trans = transform #*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	constraint = FixedConstraint.create(body0, body1, trans)
	game.addConstraint(constraint)
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
#spot_l.type = "spot"
spot_l.diffuse = ColourValue(0.6, 0.6, 0.6)
spot.attachObject(spot_l)
spot.position = Vector3(0, 1.5, 3)
#spot.orientation = Quaternion(0, 0, 0.7071, 0.7071)
camera.addChild(spot)

light = game.scene.createSceneNode("light")
light_l = game.scene.createLight("light")
light_l.diffuse = ColourValue(0.6, 0.6, 0.8)
light.attachObject(light_l)
light.position = Vector3(0, 20, 0)

ground_node = game.scene.createSceneNode("ground")
ground = game.scene.createEntity("ground", PF.PLANE)
ground_node.attachObject(ground)
ground.material_name = "ground/Basic"
ground.cast_shadows = False

kiinnityslevy= game.scene.getSceneNode("cb_kiinnityslevy")
ristikpl_kaantosyl = game.scene.getSceneNode("cb_ristikpl_kaantosyl")
nivel_klevy2 = game.scene.getSceneNode("nivel_klevy2_rotz")
transform = nivel_klevy2.world_transformation
#nivel_klevy2_hinge = createHingeConstraint(kiinnityslevy, ristikpl_kaantosyl, transform)
#fixed_joint = createFixedConstraint(kiinnityslevy, ristikpl_kaantosyl, transform)
# For debuging
game.scene.addToSelection(ristikpl_kaantosyl)
game.scene.addToSelection(kiinnityslevy)

# Sylinteri kaanto
# Without this cylinder the system will be lopsided :D
# Too heavy weight for the cylinder rotation to hold alone
# TODO a small positional offset in the piston relative to the tube
syl_kaanto_varsi = game.scene.getSceneNode("cb_syl_kaanto_varsi")
# reference joint
nivel_kaantosyl2_rotz = game.scene.getSceneNode("nivel_kaantosyl2_rotz")
transform = nivel_kaantosyl2_rotz.world_transformation
kaanto_hinge = createHingeConstraint(ristikpl_kaantosyl, syl_kaanto_varsi, transform)
#createFixedConstraint(ristikpl_kaantosyl, syl_kaanto_varsi, transform)
#syl_kaanto_varsi.hide()

transform = syl_kaanto_varsi.world_transformation
syl_kaanto_putki = game.scene.getSceneNode("cb_syl_kaanto_putki")
kaanto_joint = createTranslationConstraint(syl_kaanto_varsi, syl_kaanto_putki, transform, -0.3, 0.3)
#syl_kaanto_putki.hide()

nivel_klevy1 = game.scene.getSceneNode("nivel_klevy1_rotz")
transform = nivel_klevy1.world_transformation
kaantokappale = game.scene.getSceneNode("cb_kaantokappale")
createHingeConstraint(kiinnityslevy, kaantokappale, transform)
#createFixedConstraint(kiinnityslevy, kaantokappale, transform)
#kaantokappale.hide()

# Sylinteri nosto
nivel_sylnosto_rotz = game.scene.getSceneNode("nivel_nostosyl2_rotz")
transform = nivel_sylnosto_rotz.world_transformation
syl_nosto_varsi = game.scene.getSceneNode("cb_syl_nosto_varsi")
createHingeConstraint(kaantokappale, syl_nosto_varsi, transform)
#createFixedConstraint(kaantokappale, syl_nosto_varsi, transform)

syl_nosto_putki = game.scene.getSceneNode("cb_syl_nosto_putki")
transform = syl_nosto_varsi.world_transformation
nosto_joint = createTranslationConstraint(syl_nosto_varsi, syl_nosto_putki, transform, -1.3, 1.3)

# Ulkoputki constraints:
# Needs IK solver for adding more than one constraint
# kaantokappale, nostosylinteri, kaantosylinteri and sisaputki
ulkoputki = game.scene.getSceneNode("cb_ulkoputki")
nivel_puomi = game.scene.getSceneNode("nivel_puomi_rotz")
#createHingeConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation)
#ulkoputki.hide()

nivel_nostosyl1 = game.scene.getSceneNode("nivel_nostosyl1_rotz")
createHingeConstraint(syl_nosto_putki, ulkoputki, nivel_nostosyl1.world_transformation)

nivel_kaantosyl1 = game.scene.getSceneNode("nivel_kaantosyl1_rotz")
#createHingeConstraint(syl_kaanto_putki, ulkoputki, nivel_kaantosyl1.world_transformation)

sisaputki = game.scene.getSceneNode("cb_sisaputki")
nivel_telesk = game.scene.getSceneNode("nivel_telesk_trz")
teleskooppi_joint = createTranslationConstraint(ulkoputki, sisaputki, nivel_telesk.world_transformation, -1, 1)

# enable motors for cylinders
#kaanto_joint.actuator = True
#kaanto_joint.max_lin_motor_force = 100
nosto_joint.actuator = True
#nosto_joint.max_lin_motor_force = 100
nosto_joint.speed = 0.1
#teleskooppi_joint.actuator = True
#teleskooppi_joint.max_lin_motor_force = 100
#teleskooppi_joint.motor_velocity = 0.5
kaanto_hinge.actuator = True
kaanto_hinge.speed = Radian(0.1)
kaanto_hinge.lower_limit = Radian(0.1)
kaanto_hinge.upper_limit = Radian(0)
kaanto_hinge.target = Radian(1)

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

action = ScriptAction.create()
action.game = game
action.script = "nosto_joint.target = -2"
trigger = game.event_manager.createKeyTrigger(KC.Z)
trigger.action_down = action

action = ScriptAction.create()
action.game = game
action.script = "nosto_joint.target = 2"
trigger = game.event_manager.createKeyTrigger(KC.X)
trigger.action_down = action

# Add force action
# This is correct
# TODO this needs a motor action
# This can not work without the motor, because the joint is alligned in
# different coordinates that the body, so if we try to use the bodies local
# coordinates we are using incorrect coordinate system
"""
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
"""

