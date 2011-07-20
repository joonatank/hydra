# -*- coding: utf-8 -*-

camera = game.scene.getSceneNode("CameraNode")
camera.position = Vector3(0, 3, 15)
createCameraMovements(camera, speed=10)

ogre = game.scene.getSceneNode("ogre")
addMoveSelection(speed=3, angular_speed=Degree(60), reference=camera)
ogre.position = Vector3(0, 2.5, 0)

game.player.camera = "Omakamera"

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

athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "anthene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale = Vector3(1,1,1)*0.05;

game.scene.shadows.enable()

if game.scene.hasSceneNode("spot"):
	spot = game.scene.getLight("spot")
	spot.setSpotRange(Radian(1.5), Radian(1.7), 0.7)
	spot.attenuation = LightAttenuation(35, 0.9, 0.09, 0.01)
	spot_n = game.scene.getSceneNode("spot")
	spot_n.position = Vector3(0, 20, 0)
	spot_n.hide()
	# Test code for lights at a distance
	#spot_n.position = Vector3(0, 100, 0)
	#spot.attenuation = LightAttenuation(200, 0.9, 0.09, 0.01)
	game.scene.addToSelection(spot_n)

# TODO add hemi light for shader testing

# TODO add directional sun light for shader testing

headlight = game.scene.createLight("headlight")
headlight.type = "spot"
headlight_n = game.scene.createSceneNode("headlight")
headlight_n.attachObject(headlight)
#camera.addChild(headlight_n)
headlight_n.position = Vector3(17, 16, -8)
#headlight_n.orientation = Quaternion(0, 0, 0.7071, 0.7071)
headlight_n.orientation = Quaternion(0.16, 0.25, 0.766, 0.57)

game.scene.addToSelection(headlight_n)

