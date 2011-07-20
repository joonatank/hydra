# -*- coding: utf-8 -*-

camera = game.scene.createSceneNode("camera")
#camera.position = Vector3(0, 3, 15)
cam = game.scene.createCamera("camera")
camera.attachObject(cam)
createCameraMovements(camera, speed=10)

game.player.camera = "camera"

addMoveSelection(speed=3, angular_speed=Degree(60))

#ogre = game.scene.getSceneNode("ogre")
#ogre.position = Vector3(0, 2.5, 0)

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

wall_ent = game.scene.createEntity('wall', "ground", True)
wall = game.scene.createSceneNode("wall")
wall.attachObject(wall_ent)
wall_ent.material_name = 'ground/flat/shadows'
wall_ent.cast_shadows = False
wall.orientation = Quaternion(0.7071, 0.7071, 0, 0)
wall.position = Vector3(0, 20, -20)

# TODO create an ogre entity

athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "anthene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale = Vector3(1,1,1)*0.05;

# TODO multiple athene entities use clone

game.scene.shadows.enable()

# Test transparency
# TODO create a semi-transparent glass surface say alpha 0.8

# TODO create a textured semi-transparent surface with different alpha values
# in the texture

# TODO add hemi light for shader testing

# TODO add directional sun light for shader testing

# Test spotlight
headlight = game.scene.createLight("headlight")
headlight.type = "spot"
headlight.attenuation = LightAttenuation(100, 0.9, 0.1, 0)
headlight_n = game.scene.createSceneNode("headlight")
headlight_n.attachObject(headlight)
camera.addChild(headlight_n)

#game.scene.addToSelection(headlight_n)

