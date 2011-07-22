# -*- coding: utf-8 -*-

import math

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
def createHingeConstraint(body0, body1, transform, min = Radian(), max = Radian()) :
	trans = transform #*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	#local0_trans = body0.transform_to_local(trans)
	#local1_trans = body1.transform_to_local(trans)
	constraint = HingeConstraint.create(body0, body1, trans)
	constraint.lower_limit = min
	constraint.upper_limit = max
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
ground.material_name = "ground/bump_mapped/shadows"
ground.cast_shadows = False

kiinnityslevy= game.scene.getSceneNode("cb_kiinnityslevy")
ristikpl_kaantosyl = game.scene.getSceneNode("cb_ristikpl_kaantosyl")
nivel_klevy2 = game.scene.getSceneNode("nivel_klevy2_rotz")
transform = nivel_klevy2.world_transformation
#nivel_klevy2_hinge = createHingeConstraint(kiinnityslevy, ristikpl_kaantosyl, transform)
#fixed_joint = createFixedConstraint(kiinnityslevy, ristikpl_kaantosyl, transform)
# For debuging
#game.scene.addToSelection(ristikpl_kaantosyl)
#game.scene.addToSelection(kiinnityslevy)

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
# TODO can't create another joint because we already have one for ulkoputki
#kaanto_joint = createTranslationConstraint(syl_kaanto_varsi, syl_kaanto_putki, transform, -0.3, 0.3)
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
nosto_joint = createTranslationConstraint(syl_nosto_varsi, syl_nosto_putki, transform, -0.3, 0.3)

# Ulkoputki constraints:
# Needs IK solver for adding more than one constraint
# kaantokappale, nostosylinteri, kaantosylinteri and sisaputki
ulkoputki = game.scene.getSceneNode("cb_ulkoputki")
nivel_puomi = game.scene.getSceneNode("nivel_puomi_rotz")
puomi_hinge = createHingeConstraint(kaantokappale, ulkoputki, nivel_puomi.world_transformation, Radian(-1), Radian(0))
#ulkoputki.hide()

nivel_nostosyl1 = game.scene.getSceneNode("nivel_nostosyl1_rotz")
createHingeConstraint(syl_nosto_putki, ulkoputki, nivel_nostosyl1.world_transformation)

nivel_kaantosyl1 = game.scene.getSceneNode("nivel_kaantosyl1_rotz")
#createHingeConstraint(syl_kaanto_putki, ulkoputki, nivel_kaantosyl1.world_transformation)

sisaputki = game.scene.getSceneNode("cb_sisaputki")
nivel_telesk = game.scene.getSceneNode("nivel_telesk_trz")
teleskooppi = createTranslationConstraint(ulkoputki, sisaputki, nivel_telesk.world_transformation, 0., 1.05)

# Add rest of the objects with fixed joints
syl_tilt_putki = game.scene.getSceneNode("cb_syl_tilt_putki")
syl_tilt_varsi = game.scene.getSceneNode("cb_syl_tilt_varsi")
nivel_tiltsyl1 = game.scene.getSceneNode("nivel_tiltsyl1_rotz")
nivel_tiltsyl2 = game.scene.getSceneNode("nivel_tiltsyl2_rotz")
createFixedConstraint(sisaputki, syl_tilt_putki, nivel_tiltsyl1.world_transformation)
createFixedConstraint(syl_tilt_putki, syl_tilt_varsi, nivel_tiltsyl2.world_transformation)

tiltkappale = game.scene.getSceneNode("cb_tiltkappale")
nivel_tilt = game.scene.getSceneNode("nivel_tilt_rotz")
createFixedConstraint(sisaputki, tiltkappale, nivel_tilt.world_transformation)

syl_swing_putki = game.scene.getSceneNode("cb_syl_swing_putki")
syl_swing_varsi = game.scene.getSceneNode("cb_syl_swing_varsi")
nivel_swingsyl1 = game.scene.getSceneNode("nivel_swingsyl1_rotz")
createFixedConstraint(sisaputki, syl_swing_putki, nivel_swingsyl1.world_transformation)
createFixedConstraint(syl_swing_putki, syl_swing_varsi, syl_swing_varsi.world_transformation)

# TODO add constraint to riskikappale_swing
ristikpl_swing = game.scene.getSceneNode("cb_ristikpl_swing")
nivel_swingsyl2 = game.scene.getSceneNode("nivel_swingsyl2_rotz")
createFixedConstraint(syl_swing_putki, ristikpl_swing, nivel_tilt.world_transformation)

swingkappale = game.scene.getSceneNode("cb_swingkappale")
nivel_tiltkpl = game.scene.getSceneNode("nivel_tiltkpl_rotz")
createFixedConstraint(tiltkappale, swingkappale, nivel_tiltkpl.world_transformation)

vaantomoottori = game.scene.getSceneNode("cb_vaantomot")
nivel_vaantomoottori = game.scene.getSceneNode("nivel_vaantomot_rotz")
motor_hinge = createHingeConstraint(swingkappale, vaantomoottori, nivel_vaantomoottori.world_transformation, Radian(-math.pi/2), Radian(math.pi/2))

pulttausnivel = game.scene.getSceneNode("cb_pulttausnivel")
nivel_vaantomoottori2 = game.scene.getSceneNode("nivel_vaantomot2_rotz")
createFixedConstraint(vaantomoottori, pulttausnivel, nivel_vaantomoottori2.world_transformation)

pulttaus_varsi = game.scene.getSceneNode("cb_syl_pulttaus_varsi")
nivel_pultsyl1 = game.scene.getSceneNode("nivel_pulttsyl1_rotz")
createFixedConstraint(pulttausnivel, pulttaus_varsi, nivel_pultsyl1.world_transformation)

pulttaus_putki = game.scene.getSceneNode("cb_syl_pulttaus_putki")
nivel_pultsyl2 = game.scene.getSceneNode("nivel_pulttsyl2_rotz")
createFixedConstraint(pulttaus_varsi, pulttaus_putki, nivel_pultsyl2.world_transformation)

kehto = game.scene.getSceneNode("cb_kehto")
pulttaus_hinge = createHingeConstraint(pulttausnivel, kehto, nivel_vaantomoottori2.world_transformation, Radian(0), Radian(math.pi/4))
pulttaus_hinge.axis = Vector3(1, 0, 0)

syl_syotto_putki = game.scene.getSceneNode("cb_syl_syotto_putki")
syl_syotto_varsi = game.scene.getSceneNode("cb_syl_syotto_varsi")
createFixedConstraint(kehto, syl_syotto_putki, syl_syotto_putki.world_transformation)
createFixedConstraint(syl_syotto_putki, syl_syotto_varsi, syl_syotto_putki.world_transformation)

raide = game.scene.getSceneNode("cb_syottol_kiskot")
createFixedConstraint(kehto, raide, raide.world_transformation)

ohjuri_slaite = game.scene.getSceneNode("cb_ohjuri_slaite")
kiinnike1_slaite = game.scene.getSceneNode("cb_kiinnike1_slaite")
porakelkka = game.scene.getSceneNode("cb_porakelkka")
porakanki = game.scene.getSceneNode("cb_porakanki")
createFixedConstraint(raide, ohjuri_slaite, raide.world_transformation)
createFixedConstraint(raide, kiinnike1_slaite, raide.world_transformation)
createFixedConstraint(raide, porakelkka, raide.world_transformation)
createFixedConstraint(raide, porakanki, raide.world_transformation)

# enable motors for cylinders
#kaanto_joint.actuator = True
#kaanto_joint.max_lin_motor_force = 100
nosto_joint.actuator = True
#nosto_joint.max_lin_motor_force = 100
nosto_joint.speed = 0.1
teleskooppi.actuator = True
teleskooppi.speed = 0.1
puomi_hinge.actuator = True
puomi_hinge.speed = Radian(0.1)
motor_hinge.actuator = True
motor_hinge.speed = Radian(0.1)
pulttaus_hinge.actuator = True
pulttaus_hinge.speed = Radian(0.1)


kaanto_hinge.actuator = True
kaanto_hinge.speed = Radian(0.1)
kaanto_hinge.lower_limit = Radian(0.1)
kaanto_hinge.upper_limit = Radian(0)
#kaanto_hinge.target = Radian(1)

# Hide links
game.scene.hideSceneNodes("nivel*")

# some test code
pulttaus_hinge.target = Radian(1)

# CB mapping
game.scene.mapCollisionBarriers()

#kiinnityslevy.translate(Vector3(0, 2, 0))

# Add a motor action to nosto cylinder
# Note this action does not repeat so it's executed once for every key down 
toggle = ToggleActionProxy.create()

# Movements for puomi_hinge
action = ScriptAction.create()
action.script = "teleskooppi.target = teleskooppi.upper_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "teleskooppi.target = teleskooppi.position"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.U)
trigger.action_down = toggle


toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "teleskooppi.target = teleskooppi.lower_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "teleskooppi.target = teleskooppi.position"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.I)
trigger.action_down = toggle


# Movements for puomi_hinge
# The axis for the puomi is flipped so up and low limits are flipped
toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "puomi_hinge.target = puomi_hinge.lower_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "puomi_hinge.target = puomi_hinge.angle"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.J)
trigger.action_down = toggle

toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "puomi_hinge.target = puomi_hinge.upper_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "puomi_hinge.target = puomi_hinge.angle"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.K)
trigger.action_down = toggle


# Movements for motor_hinge
toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "motor_hinge.target = motor_hinge.upper_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "motor_hinge.target = motor_hinge.angle"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.N)
trigger.action_down = toggle

toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "motor_hinge.target = motor_hinge.lower_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "motor_hinge.target = motor_hinge.angle"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.M)
trigger.action_down = toggle


# Movements for motor_hinge
toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "pulttaus_hinge.target = pulttaus_hinge.upper_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "pulttaus_hinge.target = pulttaus_hinge.angle"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.T)
trigger.action_down = toggle

toggle = ToggleActionProxy.create()

action = ScriptAction.create()
action.script = "pulttaus_hinge.target = pulttaus_hinge.lower_limit"
action.game = game
toggle.action_on = action

action = ScriptAction.create()
action.script = "pulttaus_hinge.target = pulttaus_hinge.angle"
action.game = game
toggle.action_off = action

trigger = game.event_manager.createKeyTrigger(KC.Y)
trigger.action_down = toggle

