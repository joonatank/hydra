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

def addTranslationEvent(node) :
	# TODO should print the node name, conversion to string is not impleted yet
	print 'Creating Translation event on node = '
	event = event_manager.createEvent( 'TransformationEvent' )
	event.scene_node = node
	# TODO this is bit verbose and repeated often move it to python function
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.D
	trigger_neg.key = KC.A
	event.setTransXtrigger( trigger_pos, trigger_neg )
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.PGUP
	trigger_neg.key = KC.PGDOWN
	event.setTransYtrigger( trigger_pos, trigger_neg )
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.S
	trigger_neg.key = KC.W
	event.setTransZtrigger( trigger_pos, trigger_neg )

	# TODO yaw missing, but the Application does not allow yaw for cameras because
	# it looks funny in multi wall systems
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.RIGHT
	trigger_neg.key = KC.LEFT
	event.setRotYtrigger( trigger_pos, trigger_neg )

	if not event_manager.addEvent(event) :
		print 'Python : could not add event'

def addHideEvent(node) :
	print 'Creating Hide Event'
	hide = event_manager.createOperation( 'HideOperation' )
	hide.scene_node = node 
	show = event_manager.createOperation( 'ShowOperation' )
	show.scene_node = node 
	# This works to solve the problem
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	#trigger = event_manager.createTrigger( 'KeyTrigger' )
	trigger.key = KC.H

	event = event_manager.createEvent( 'ToggleEvent' )
	event.toggle_off_action= show 
	event.toggle_on_action= hide 
	event.toggle_state = False
	# Works fine provide an example
	#event.time_limit = 2	# Secs

	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'


def addQuitEvent() :
	print 'Creating Quit Event'
	action = event_manager.createOperation( 'QuitOperation' )
	print 'Python Action type = ' + action.type
	action.config = config 
	# Example of using pressed or released key instead of both (the default)
	# you need to create either KeyReleasedTrigger or KeyPressedTrigger
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	print 'Python Trigger type = ' + trigger.type
	trigger.key = KC.Q
	# KeyTrigger will respond to both

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.operation = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

def addReloadEvent() :
	print 'Creating Reload Scene Event'
	action = event_manager.createOperation( 'ReloadScene' )
	print 'Python Action type = ' + action.type
	action.config = config 
	trigger = event_manager.createTrigger( 'KeyTrigger' )
	# FIXME this segfaults
	print 'Python Trigger type = ' + trigger.type
	trigger.key = KC.R
	# Example of using released key instead of pressed (which is default)
	# These are moved to different class now
	#trigger.released = True

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.operation = action
	# Setting a time limit how often this Event can happen
	event.time_limit = 5
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

def addToggleMusicEvent() :
	print 'Creating Toggle Music Event'
	action = event_manager.createOperation( 'ToggleMusic' )
	action.config = config 
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	trigger.key = KC.M

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.operation = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'
	
# TODO create PrintOperation in python and register it into Event Manager

print 'In python init script'
print 'Adding config events'
addQuitEvent()
addReloadEvent()
addToggleMusicEvent()

print 'Creating Ogre SceneNode'
ogre = SceneNode.create("ogre")
config.addSceneNode(ogre)
addHideEvent(ogre)
addRotationEvent(ogre)

print 'Creating Camera SceneNode'
camera = SceneNode.create("CameraNode")
config.addSceneNode(camera)
addTranslationEvent(camera)

