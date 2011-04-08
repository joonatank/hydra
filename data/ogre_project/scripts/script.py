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
ogre = game.scene.getSceneNode("ogre")
addHideEvent(ogre, KC.H)
addMoveSelection()
game.scene.addToSelection(ogre)
ogre.position = Vector3(0, 2.5, 0)

print( 'Getting Camera SceneNode' )
camera = game.scene.getSceneNode("CameraNode")
camera.position = camera.position + Vector3(0, 3, 0)
createCameraMovements(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )

game.createBackgroundSound("The_Dummy_Song.ogg")
addToggleMusicEvent(KC.M)

# Hack for some lighting before we can modify lights dynamically
game.scene.ambient_light = ColourValue(0.2,0.2,0.2)

# Create ground plane
ground_ent = game.scene.createEntity('ground', PF.PLANE)
ground = game.scene.createSceneNode('ground')
ground.addEntity(ground_ent)
ground.orientation = Quaternion(-0.7071, 0.7071, 0, 0)
ground_ent.material_name = 'ground'

sphere_ent = game.scene.createEntity('sphere', PF.SPHERE)
sphere_ent.material_name = 'debug_red'
sphere = game.scene.createSceneNode('sphere')
sphere.addEntity(sphere_ent)
sphere.position = Vector3(4, 2.5, 0)
sphere.scale *= 0.003

