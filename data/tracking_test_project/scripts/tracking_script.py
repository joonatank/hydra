# -*- coding: utf-8 -*-

# Try out Tracker Trigger to SceneNode Transformation mapping
def mapTrackers():
	for i in range(0, 20) :
		name = 'cube' + str(i)
		cube = game.scene.getSceneNode( name )
		action = SetTransformation.create() #game.event_manager.createAction( '' )
		action.scene_node = cube
		# TODO there is no checking here
		# Add checking that the object exists
		if( game.event_manager.hasTrackerTrigger( name + 'Trigger' ) ) :
			trigger = game.event_manager.getTrackerTrigger( name + 'Trigger' )
			trigger.action = action
		else :
			print 'Tracker trigger ' + name + 'Trigger' ' not found in Game.'

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

print 'Getting Camera SceneNode'
camera = game.scene.getSceneNode("CameraNode")
addTranslationEvent(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )
mapTrackers()
