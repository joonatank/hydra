# -*- coding: utf-8 -*-

def addRotationEvent(node) :
	# TODO should print the node name, conversion to string is not impleted yet
	print 'Creating Rotation event on node = '
	event = event_manager.createEvent( 'TransformationEvent' )
	event.scene_node = node
	# TODO should have different nodes for yaw and pitch so that the Ogre would
	# rotate around global axes
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.NUMPAD6
	trigger_neg.key = KC.NUMPAD4
	event.setRotYtrigger( trigger_pos, trigger_neg )
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.NUMPAD8
	trigger_neg.key = KC.NUMPAD5
	event.setRotZtrigger( trigger_pos, trigger_neg )
	print 'Adding ogre transformation event to event stack'
	if not event_manager.addEvent(event) :
		print 'Python : could not add ogre event'

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.
print 'In python init script'

print 'Getting Ogre SceneNode'
# config.getSceneNode gets a reference to already created SceneNode
# For now it's not possible to create SceneNodes from python
# So use this function to get a SceneNode created from .scene file.
# All SceneNodes in .scene file are created and can be retrieved here.
ogre = config.getSceneNode("ogre")
addHideEvent(ogre)
addRotationEvent(ogre)

print 'Getting Camera SceneNode'
camera = config.getSceneNode("CameraNode")
addTranslationEvent(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )
