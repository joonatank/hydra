# -*- coding: utf-8 -*-

def setVectorActionFromKey( vector_action, kc ):
	key_action = FloatActionMap.create()
	key_action.action = vector_action
	key_action.value = 1
	trigger = game.event_manager.createKeyPressedTrigger( kc )
	trigger.addAction( key_action )

	key_action = FloatActionMap.create()
	key_action.action = vector_action
	key_action.value = -1
	trigger = game.event_manager.createKeyReleasedTrigger( kc )
	trigger.addAction( key_action )



def addKeyActionsForAxis( trans_action, axis, kc_pos, kc_neg ) :
	float_action = VectorActionMap.create()
	float_action.axis = axis
	setVectorActionFromKey( float_action, kc_pos )
	float_action.action = trans_action

	float_action = VectorActionMap.create()
	float_action.axis = -axis
	setVectorActionFromKey( float_action, kc_neg )
	float_action.action = trans_action


# Fine using the new event interface
def createCameraMovements(node) :
	# TODO should print the node name, conversion to string is not impleted yet
	print( 'Creating Translation event on node = ' )

	# Create the translation action using a proxy
	trans_action_proxy = MoveActionProxy.create()
	trans_action_proxy.enableTranslation()
	addKeyActionsForAxis( trans_action_proxy, Vector3(1, 0, 0), KC.D, KC.A )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 0, 1), KC.S, KC.W )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 1, 0), KC.PGUP, KC.PGDOWN )

	# Create the rotation action using a proxy
	rot_action_proxy = MoveActionProxy.create()
	rot_action_proxy.enableRotation()
	addKeyActionsForAxis( rot_action_proxy, Vector3(0, 1, 0), KC.LEFT, KC.RIGHT )

	# Create the real action
	trans_action = MoveAction.create()
	trans_action.scene_node = node
	trans_action.speed = 5
	# Add the real action to the proxies
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
	# TODO add rotation speed
	# Create a FrameTrigger and add the action to that
	trigger = game.event_manager.getFrameTrigger()
	trigger.addAction( trans_action )



# Fine using the new event interface
def addQuitEvent( kc ) :
	print( 'Creating Quit Event' )
	action = QuitAction.create()
	action.game = game

	trigger = game.event_manager.createKeyPressedTrigger( kc )
	trigger.addAction( action )


# Fine using the new event interface
def addReloadEvent( kc ) :
	print( 'Creating Reload Scene Event' )
	# Create the action
	action = ReloadScene.create()
	action.scene = game.scene

	# Create the Time limit
	proxy = TimerActionProxy.create()
	proxy.action = action
	proxy.time_limit = 5 # Seconds

	# Create the trigger
	trigger = game.event_manager.createKeyPressedTrigger( kc )
	trigger.addAction( proxy )



# Fine using the new event interface
def addToggleMusicEvent( kc ) :
	print( 'Creating Toggle Music Event' )
	action = ToggleMusic.create()
	action.game = game
	trigger = game.event_manager.createKeyPressedTrigger( kc )
	trigger.addAction( action )


def addScreenshotAction( kc ) :
	# TODO add printing the kc
	print( 'Adding screenshot action to KC_F10' )
	action = ScreenshotAction.create()
	action.player = game.player
	trigger = game.event_manager.createKeyPressedTrigger( kc )
	trigger.addAction( action )


# TODO create PrintOperation in python and register it into Event Manager

# Change camera toggle
# Use one Key in this case b to change between two active cameras
# Requirement : Camera names must be correct for this to work
#
# If the camera name is incorrect the action will not change the camera
# An error message is printed to std::cerr and the program continues normally
# Fine using the new event interface
def addToggleActiveCamera( camera1, camera2 ) :
	print( 'Creating Toggle Activate Camera' )
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

	trigger.addAction( toggle )


# Fine using the new event interface
def addHideEvent(node) :
	print( 'Creating Hide Event' )
	hide = HideAction.create()
	hide.scene_node = node
	show = ShowAction.create()
	show.scene_node = node
	trigger = game.event_manager.createKeyPressedTrigger( KC.H )

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

	trigger.addAction( proxy )


# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )
addQuitEvent(KC.Q)
addReloadEvent(KC.R)
addToggleMusicEvent(KC.M)
addScreenshotAction(KC.F10)
