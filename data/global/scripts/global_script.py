# -*- coding: utf-8 -*-

# TODO using the old interface
def addTranslationEvent(node) :
	# TODO should print the node name, conversion to string is not impleted yet
	print 'Creating Translation event on node = '
	event = game.event_manager.createEvent( 'TransformationEvent' )
	event.scene_node = node
	# Set the movement speed to ten m/s
	event.speed = 10
	# Set the rotation speed to 30 degs/s
	# FIXME this does not work
	#event.angular_speed = 30
	# TODO this is bit verbose and repeated often move it to python function
	trigger_pos = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.D
	trigger_neg.key = KC.A
	event.setTransXtrigger( trigger_pos, trigger_neg )
	trigger_pos = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.PGUP
	trigger_neg.key = KC.PGDOWN
	event.setTransYtrigger( trigger_pos, trigger_neg )
	trigger_pos = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.S
	trigger_neg.key = KC.W
	event.setTransZtrigger( trigger_pos, trigger_neg )

	# TODO yaw missing, but the Application does not allow yaw for cameras because
	# it looks funny in multi wall systems
	trigger_pos = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = game.event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.LEFT
	trigger_neg.key = KC.RIGHT
	event.setRotYtrigger( trigger_pos, trigger_neg )

	if not game.event_manager.addEvent(event) :
		print 'Python : could not add event'



# Fine using the new event interface
def addQuitEvent() :
	print 'Creating Quit Event'
	action = QuitAction.create()
	print 'Python Action type = ' + action.type
	action.game = game

	trigger = game.event_manager.createKeyPressedTrigger( KC.Q )
	print 'Python Trigger type = ' + trigger.type
	trigger.action = action


# Fine using the new event interface
def addReloadEvent() :
	print 'Creating Reload Scene Event'
	# Create the action
	action = ReloadScene.create()
	print 'Python Action type = ' + action.type
	action.scene = game.scene

	# Create the Time limit
	proxy = TimerActionProxy.create()
	print 'Python Proxy type = ' + proxy.type
	proxy.action = action
	proxy.time_limit = 5 # Seconds

	# Create the trigger
	trigger = game.event_manager.createKeyPressedTrigger( KC.R )
	print 'Python Trigger type = ' + trigger.type
	trigger.action = proxy



# Fine using the new event interface
def addToggleMusicEvent() :
	print 'Creating Toggle Music Event'
	action = ToggleMusic.create()
	print 'Python Action type = ' + action.type
	action.game = game
	trigger = game.event_manager.createKeyPressedTrigger( KC.M )
	trigger.action = action

# TODO create PrintOperation in python and register it into Event Manager

# Change camera toggle
# Use one Key in this case b to change between two active cameras
# Requirement : Camera names must be correct for this to work
#
# If the camera name is incorrect the action will not change the camera
# An error message is printed to std::cerr and the program continues normally
# Fine using the new event interface
def addToggleActiveCamera( camera1, camera2 ) :
	print 'Creating Toggle Activate Camera'
	action_on = ActivateCamera.create()
	action_on.player = game.player
	action_on.camera = camera1

	action_off = ActivateCamera.create()
	action_off.player = game.player
	action_off.camera = camera2

	trigger = game.event_manager.createKeyPressedTrigger( KC.B )

	toggle = ToggleActionProxy.create()
	toggle.action_on = action_on
	toggle.action_off = action_off

	trigger.action = toggle


# Fine using the new event interface
def addHideEvent(node) :
	print 'Creating Hide Event'
	hide = HideAction.create()
	hide.scene_node = node
	show = ShowAction.create()
	show.scene_node = node
	trigger = game.event_manager.createKeyPressedTrigger( KC.H )
	#trigger = game.event_manager.createTrigger( 'KeyReleasedTrigger' )
	#trigger.key = KC.H

	# Create a proxy that handles the toggling between two different actions
	toggle = ToggleActionProxy.create()
	toggle.action_on = hide
	toggle.action_off = show

	# Create a proxy for slowing down the toggling
	# NOTE This can be done the otherway around also i.e. create two timer
	# proxys and assign them to the toggle proxy
	# This allows individual control on the wait time from one state to another
	proxy = TimerActionProxy.create()
	proxy.action = toggle
	proxy.time_limit = 2 # Seconds

	trigger.action = proxy


# Add some global events that are useful no matter what the scene/project is
print 'Adding game events'
addQuitEvent()
addReloadEvent()
addToggleMusicEvent()