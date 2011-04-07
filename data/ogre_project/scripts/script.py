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

print( 'Getting Camera SceneNode' )
camera = game.scene.getSceneNode("CameraNode")
createCameraMovements(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )

game.createBackgroundSound("The_Dummy_Song.ogg")
addToggleMusicEvent(KC.M)

# Some nodes for SceneNode creation and hierarchy modification examples
node1 = game.scene.createSceneNode("node1")
node2 = game.scene.createSceneNode("node2")
node1.addChild(ogre)
node1.addChild(node2)
