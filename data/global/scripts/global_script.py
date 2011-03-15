# -*- coding: utf-8 -*-

def setVectorActionFromKey( vector_action, kc, mod ):
	key_action = FloatActionMap.create()
	key_action.action = vector_action
	key_action.value = 1
	trigger = game.event_manager.createKeyPressedTrigger( kc, mod )
	trigger.addAction( key_action )

	key_action = FloatActionMap.create()
	key_action.action = vector_action
	key_action.value = -1
	trigger = game.event_manager.createKeyReleasedTrigger( kc, mod )
	trigger.addAction( key_action )



def addKeyActionsForAxis( trans_action, axis, kc_pos, kc_neg, mod = KEY_MOD.NONE ) :
	float_action = VectorActionMap.create()
	float_action.axis = axis
	setVectorActionFromKey( float_action, kc_pos, mod )
	float_action.action = trans_action

	float_action = VectorActionMap.create()
	float_action.axis = -axis
	setVectorActionFromKey( float_action, kc_neg, mod )
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
	# This is not useful, maybe using Q and E
	addKeyActionsForAxis( rot_action_proxy, Vector3(0, 1, 0), KC.Q, KC.E )

	# Create the real action
	trans_action = MoveNodeAction.create()
	trans_action.scene_node = node
	trans_action.speed = 5
	trans_action.angular_speed = Radian(Degree(90))
	# Add the real action to the proxies
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
	# TODO add rotation speed
	# Create a FrameTrigger and add the action to that
	trigger = game.event_manager.getFrameTrigger()
	trigger.addAction( trans_action )



def addMoveSelection() :
	print( 'Creating Move selection event' )

	trans_action_proxy = MoveActionProxy.create()
	trans_action_proxy.enableTranslation()
	addKeyActionsForAxis( trans_action_proxy, Vector3(1, 0, 0), KC.NUMPAD4, KC.NUMPAD6 )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 1, 0), KC.NUMPAD7, KC.NUMPAD9 )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 0, 1), KC.NUMPAD8, KC.NUMPAD5 )

	# Create the rotation action using a proxy
	rot_action_proxy = MoveActionProxy.create()
	rot_action_proxy.enableRotation()
	addKeyActionsForAxis( rot_action_proxy, Vector3(0, 1, 0), KC.NUMPAD4, KC.NUMPAD6, KEY_MOD.CTRL )
	addKeyActionsForAxis( rot_action_proxy, Vector3(1, 0, 0), KC.NUMPAD7, KC.NUMPAD9, KEY_MOD.CTRL )
	addKeyActionsForAxis( rot_action_proxy, Vector3(0, 0, 1), KC.NUMPAD8, KC.NUMPAD5, KEY_MOD.CTRL )

	# Create the real action
	trans_action = MoveSelectionAction.create()
	trans_action.scene = game.scene
	trans_action.local = False
	trans_action.speed = 0.3

	# Add the real action to the proxy
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
	# Create a FrameTrigger and add the action to that
	trigger = game.event_manager.getFrameTrigger()
	trigger.addAction( trans_action )


# Fine using the new event interface
def addQuitEvent( kc ) :
	print( 'Creating Quit Event to ' + getPythonKeyName(kc) )
	action = QuitAction.create()
	action.game = game

	trigger = game.event_manager.createKeyPressedTrigger( kc, KEY_MOD.META )
	trigger.addAction( action )


# Fine using the new event interface
def addReloadEvent( kc ) :
	print( 'Creating Reload Scene Event to ' + getPythonKeyName(kc) )
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
	print( 'Creating Toggle Music Event to ' + getPythonKeyName(kc) )
	action = ToggleMusic.create()
	action.game = game
	trigger = game.event_manager.createKeyPressedTrigger( kc )
	trigger.addAction( action )


def addScreenshotAction( kc ) :
	# TODO add printing the kc
	print( 'Adding screenshot action to ' + getPythonKeyName(kc) )
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


def addHideEvent(node, kc) :
	print( 'Creating Hide Event to ' + getPythonKeyName(kc) )
	hide = HideAction.create()
	hide.scene_node = node
	show = ShowAction.create()
	show.scene_node = node
	trigger = game.event_manager.createKeyPressedTrigger(kc)

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

def addToggleConsole(kc) :
	print( 'Creating Toggle GUI Console Event to ' + getPythonKeyName(kc) )
	hide = HideConsole.create()
	hide.gui = game.gui
	show = ShowConsole.create()
	show.gui = game.gui
	trigger = game.event_manager.createKeyPressedTrigger(kc)

	# Create a proxy that handles the toggling between two different actions
	toggle = ToggleActionProxy.create()
	toggle.action_on = show
	toggle.action_off = hide

	trigger.addAction( toggle )

def addToggleEditor(kc) :
	print( 'Creating Toggle GUI Editor Event to ' + getPythonKeyName(kc) )
	hide = HideEditor.create()
	hide.gui = game.gui
	show = ShowEditor.create()
	show.gui = game.gui
	trigger = game.event_manager.createKeyPressedTrigger(kc)

	# Create a proxy that handles the toggling between two different actions
	toggle = ToggleActionProxy.create()
	toggle.action_on = show
	toggle.action_off = hide

	trigger.addAction( toggle )


# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )
addQuitEvent(KC.Q)
addReloadEvent(KC.R)
addScreenshotAction(KC.F10)
addToggleEditor(KC.F2)
addToggleConsole(KC.GRAVE)
