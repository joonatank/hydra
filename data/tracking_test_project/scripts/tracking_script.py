# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

print 'Getting Camera SceneNode'
camera = config.getSceneNode("CameraNode")
addTranslationEvent(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )

# Try out Tracker Trigger to SceneNode Transformation mapping
for i in range(0, 20) :
	name = 'cube' + str(i)
	cube = config.getSceneNode( name )
	action = event_manager.createAction( 'SetTransformation' )
	action.scene_node = cube
	trigger = config.getTrackerTrigger( name + 'Trigger' )
	trigger.action = action
