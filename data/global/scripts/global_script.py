# -*- coding: utf-8 -*-
def addTranslationEvent(node) :
	# TODO should print the node name, conversion to string is not impleted yet
	print 'Creating Translation event on node = '
	event = event_manager.createEvent( 'TransformationEvent' )
	event.scene_node = node
	# Set the movement speed to ten m/s
	event.speed = 10
	# Set the rotation speed to 30 degs/s
	# FIXME this does not work
	#event.angular_speed = 30
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

def addQuitEvent() :
	print 'Creating Quit Event'
	action = event_manager.createAction( 'QuitOperation' )
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
	event.action = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

def addReloadEvent() :
	print 'Creating Reload Scene Event'
	action = event_manager.createAction( 'ReloadScene' )
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
	event.action = action
	# Setting a time limit how often this Event can happen
	event.time_limit = 5
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

def addToggleMusicEvent() :
	print 'Creating Toggle Music Event'
	action = event_manager.createAction( 'ToggleMusic' )
	action.config = config
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	trigger.key = KC.M

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.action = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

# TODO create PrintOperation in python and register it into Event Manager

# Change camera toggle
# Use one Key in this case b to change between two active cameras
# Requirement : Camera names must be correct for this to work
#
# If the camera name is incorrect the action will not change the camera
# An error message is printed to std::cerr and the program continues normally
def addToggleActiveCamera( camera1, camera2 ) :
	print 'Creating Toggle Activate Camera'
	action_on = event_manager.createAction( 'ActivateCamera' )
	action_on.config = config
	action_on.camera = camera1
	action_off = event_manager.createAction( 'ActivateCamera' )
	action_off.config = config
	action_off.camera = camera2
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	trigger.key = KC.B

	# Create the Event
	event = event_manager.createEvent( 'ToggleEvent' )
	event.toggle_on_action = action_on
	event.toggle_off_action = action_off
	event.toggle_state = False
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

def addHideEvent(node) :
	print 'Creating Hide Event'
	hide = event_manager.createAction( 'Hide' )
	hide.scene_node = node
	show = event_manager.createAction( 'Show' )
	show.scene_node = node
	# This works to solve the problem
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	#trigger = event_manager.createTrigger( 'KeyTrigger' )
	trigger.key = KC.H

	event = event_manager.createEvent( 'ToggleEvent' )
	event.toggle_off_action= show
	event.toggle_on_action= hide
	event.toggle_state = False
	event.time_limit = 2	# Secs

	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

# Add some global events that are useful no matter what the scene/project is
print 'Adding config events'
addQuitEvent()
addReloadEvent()
addToggleMusicEvent()