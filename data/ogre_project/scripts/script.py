# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

print( 'Getting Ogre SceneNode' )
# config.getSceneNode gets a reference to already created SceneNode
# For now it's not possible to create SceneNodes from python
# So use this function to get a SceneNode created from .scene file.
# All SceneNodes in .scene file are created and can be retrieved here.
# Tangent space lighting does not work on the ogre_ent
ogre = game.scene.getSceneNode("ogre")
ogre_ent = game.scene.getEntity("Ogre")
addHideEvent(ogre, KC.H)
addMoveSelection()
game.scene.addToSelection(ogre)
ogre.position = Vector3(0, 2.5, 0)

print( 'Getting Camera SceneNode' )
camera = game.scene.getSceneNode("CameraNode")
camera.position = Vector3(0, 3, 15)
createCameraMovements(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )
game.player.camera = "Omakamera"

game.createBackgroundSound("The_Dummy_Song.ogg")
addToggleMusicEvent(KC.M)

# Create ground plane
# TODO should create the mesh using MeshManager so the size can be assigned
ground_ent = game.scene.createEntity('ground', PF.PLANE)
ground = game.scene.createSceneNode('ground')
ground.attachObject(ground_ent)
# Shader material, with shadows
ground_ent.material_name = 'ground/bump_mapped/shadows'
ground_ent.cast_shadows = False

sphere_ent = game.scene.createEntity('sphere', PF.SPHERE)
sphere_ent.material_name = 'finger_sphere/red'
sphere = game.scene.createSceneNode('sphere')
sphere.attachObject(sphere_ent)
sphere.position = Vector3(4, 2.5, 0)
sphere.scale = sphere.scale*0.003
sphere_ent.cast_shadows = True

athene = game.scene.createSceneNode("athene")
athene_ent = game.scene.createEntity("athene", "athene.mesh")
athene_ent.material_name = "anthene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale = Vector3(1,1,1)*0.05;

# enable shadows
shadows = ShadowInfo("texture_additive_integrated")
# Colour is useless for additive shadows
#shadows.colour = ColourValue(0.0, 0.5, 0.0)
game.scene.shadows = shadows

if( game.scene.hasSceneNode("spot") ):
	light = game.scene.getSceneNode("spot")
	game.scene.addToSelection(light)

game.scene.removeFromSelection(ogre)

#if( game.scene.hasLight("spot") ):
#	game.scene.getLight("spot").visible = False

if( game.scene.hasSceneNode("Ambient_light") ):
	ambient_light = game.scene.getSceneNode("Ambient_light")

""" We use the default spot for now
l = game.scene.createLight("test_light")
l.type = "spot"
l_node = game.scene.createSceneNode("test_light")
l_node.position = Vector3(5, 10, -10)
l_node.orientation = Quaternion(0, 0, 0.7071, 0.7071)
l_node.attachObject(l)
game.scene.addToSelection(l_node)
"""

# Create spotlight and hehkulamppu objects
spotti_n = game.scene.createSceneNode("spotti")
spotti = game.scene.createEntity("spotti", "spotlight.mesh")
spotti.material_name = "editor/spotlight_material"
spotti_n.position = Vector3(5, 4, 0)
spotti_n.attachObject(spotti)

lightpulp_n = game.scene.createSceneNode("hehkulamppu")
lightpulp = game.scene.createEntity("hehkulamppu", "hehkulamppu.mesh")
lightpulp.material_name = "editor/hehkulamppu_material"
lightpulp_n.position = Vector3(5, 4, -4)
lightpulp_n.attachObject(lightpulp)

