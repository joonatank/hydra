# -*- coding: utf-8 -*-

camera = game.scene.createSceneNode("camera")
camera.position = Vector3(0, 3, 15)
cam = game.scene.createCamera("camera")
camera.attachObject(cam)
createCameraMovements(camera, speed=10)

game.player.camera = "camera"

addMoveSelection(speed=3, angular_speed=Degree(60))

# Create ground plane
# Create a large plane for shader testing
# This shows the usage of the new mesh manager
ground_length = 40;
ground_mesh = game.mesh_manager.createPlane("ground", ground_length, ground_length)
print(ground_mesh)
ground_ent = game.scene.createEntity('ground', "ground", True)
ground = game.scene.createSceneNode('ground')
ground.attachObject(ground_ent)
# Shader material with shadows
#ground_ent.material_name = 'ground/bump_mapped/shadows'
ground_ent.material_name = 'ground/flat/shadows'
ground_ent.cast_shadows = False

# Can't use the new mesh manager, there are some attributes in the mesh file
# that make it impossible
ogre_ent = game.scene.createEntity("ogre", "ogre.mesh", False)
ogre = game.scene.createSceneNode("ogre")
ogre.attachObject(ogre_ent)
ogre.position = Vector3(3, 2.5, -5)

game.scene.shadows.enable()

# spotlight
spot = game.scene.createLight("spot")
spot.type = "spot"
spot.attenuation = LightAttenuation(100, 0.9, 0.1, 0)
spot_n = game.scene.createSceneNode("spot")
spot_n.attachObject(spot)
spot_n.position = Vector3(0, 25, 35)
spot_n.orientation = Quaternion(0.974, -0.225, 0.025, 0.03)

game.scene.addToSelection(spot_n)

# Test serial joystick and signals
joy = SerialJoystick.create("COM5")

# FIXME
# This is really slow, STEP time increases by 120ms for one callback function
# For both debug and release versions
# TODO add a c++ version and do timings for it
def joystick_handler(e) :
	camera.translate(Vector3(e.axis_x, 0, e.axis_y))

joy.doOnValueChanged(joystick_handler)

game.addInputDevice(joy)

