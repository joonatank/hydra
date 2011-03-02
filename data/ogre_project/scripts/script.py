# -*- coding: utf-8 -*-

def addOgreRotations(node) :
	# TODO should print the node name, conversion to string is not impleted yet
	print( 'Creating Rotation event on node = ' )

	trans_action_proxy = MoveActionProxy.create()
	trans_action_proxy.enableTranslation()
	addKeyActionsForAxis( trans_action_proxy, Vector3(1, 0, 0), KC.NUMPAD4, KC.NUMPAD6 )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 0, 1), KC.NUMPAD8, KC.NUMPAD5 )

	# Create the rotation action using a proxy
	rot_action_proxy = MoveActionProxy.create()
	rot_action_proxy.enableRotation()
	addKeyActionsForAxis( rot_action_proxy, Vector3(0, 1, 0), KC.NUMPAD4, KC.NUMPAD6, KEY_MOD.CTRL )
	addKeyActionsForAxis( rot_action_proxy, Vector3(0, 0, 1), KC.NUMPAD8, KC.NUMPAD5, KEY_MOD.CTRL )

	# Create the real action
	trans_action = MoveAction.create()
	trans_action.scene_node = node
	# TODO add rotation speed
	# Add the real action to the proxy
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
	# Create a FrameTrigger and add the action to that
	trigger = game.event_manager.getFrameTrigger()
	trigger.addAction( trans_action )

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
addOgreRotations(ogre)
game.scene.addToSelection(ogre)

print( 'Getting Camera SceneNode' )
camera = game.scene.getSceneNode("CameraNode")
createCameraMovements(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )
