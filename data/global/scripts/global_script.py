# -*- coding: utf-8 -*-

def setVectorActionFromKey( vector_action, kc, mod ):
	key_action = FloatActionMap.create()
	key_action.action = vector_action
	key_action.value = 1
	trigger = game.event_manager.createKeyTrigger( kc, mod )
	trigger.action_down = key_action

	key_action = FloatActionMap.create()
	key_action.action = vector_action
	key_action.value = -1
	trigger.action_up = key_action



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
def createCameraMovements(node, speed = 5, angular_speed = Degree(90)) :
	# TODO stupid string casting, can we somehow remove it?
	print( 'Creating Translation event on ' + str(node) )

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
	trans_action.speed = speed
	trans_action.angular_speed = Radian(angular_speed)
	# Add the real action to the proxies
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
	# TODO add rotation speed
	# Create a FrameTrigger and add the action to that
	trigger = game.event_manager.getFrameTrigger()
	trigger.action.add_action(trans_action)



# TODO add a possibility to speed up movements using SHIFT modifier
# @param speed linear speed of the selection
# @param angular_speed rotation speed of the selection
# @param reference The object whom coordinate system is used for translation, usually camera
def addMoveSelection(speed = 0.3, angular_speed = Degree(40), reference=None) :
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
	trans_action.reference = reference
	trans_action.speed = speed
	trans_action.angular_speed = Radian(angular_speed)

	# Add the real action to the proxy
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
	# Create a FrameTrigger and add the action to that
	# TODO having the frame trigger action replacable causes the
	# move camera to override this.
	trigger = game.event_manager.getFrameTrigger()
	trigger.action.add_action( trans_action )

def mapHeadTracker(name) :
	act = HeadTrackerAction.create()
	act.player = game.player
	# Create a fake tracker trigger
	if(not game.event_manager.hasTrackerTrigger(name)):
		tracker = Tracker.create("FakeHeadTracker")
		tracker.n_sensors = 1
		t = Transform(Vector3(0, 1.5, 0))
		tracker.getSensor(0).default_transform = t
		# Create trigger
		trigger = game.event_manager.createTrackerTrigger(name)
		tracker.getSensor(0).trigger = trigger
		game.tracker_clients.addTracker(tracker)

	trigger = game.event_manager.getTrackerTrigger(name)
	trigger.action = act


# Fine using the new event interface
def addQuitEvent( kc ) :
	print( 'Creating Quit Event to ' + getPythonKeyName(kc) )
#	action = QuitAction.create()
	action = ScriptAction.create()
	action.game = game
	action.script = "quit()"

	trigger = game.event_manager.createKeyTrigger( kc, KEY_MOD.META )
	trigger.action_down = action


def addToggleMusicEvent( kc ) :
	print( 'Creating Toggle Music Event to ' + getPythonKeyName(kc) )
	#action = ToggleMusic.create()
	action = ScriptAction.create()
	action.game = game
	action.script = "game.toggleBackgroundSound()"

	trigger = game.event_manager.createKeyTrigger( kc )
	trigger.action_down = action


def addScreenshotAction( kc ) :
	print( 'Adding screenshot action to ' + getPythonKeyName(kc) )
#	action = ScreenshotAction.create()
	action = ScriptAction.create()
	action.game = game
	action.script = "game.player.takeScreenshot()"
	trigger = game.event_manager.createKeyTrigger( kc )
	trigger.action_down = action



# Change camera toggle
# Use one Key in this case b to change between two active cameras
# Requirement : Camera names must be correct for this to work
#
# If the camera name is incorrect the action will not change the camera
# An error message is printed to std::cerr and the program continues normally
def addToggleActiveCamera( camera1, camera2 ) :
	print( 'Creating Toggle Activate Camera between ' + camera1 + ' and ' + camera2 )
#	action_on = ActivateCamera.create()
	action_on = ScriptAction.create()
	action_on.game = game
	# FIXME not a really nice technique, we need to escape the camera name
	# because if it's simply string python tries to resolve the name and we
	# get undefined variable error.
	action_on.script = "game.player.camera = " + "\'" + camera1 + "\'"

#	action_off = ActivateCamera.create()
	action_off = ScriptAction.create()
	action_off.game = game
	action_off.script = "game.player.camera = " + "\'" + camera2 + "\'"

	trigger = game.event_manager.createKeyTrigger( KC.B )

	toggle = ToggleActionProxy.create()
	toggle.action_on = action_on
	toggle.action_off = action_off

	trigger.action_down = toggle


def addHideEvent(node, kc) :
	print( 'Creating Hide Event for ' + str(node) + ' to ' + getPythonKeyName(kc) )
	hide = HideAction.create()
	hide.scene_node = node
	show = ShowAction.create()
	show.scene_node = node
	trigger = game.event_manager.createKeyTrigger(kc)

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

	trigger.action_down = proxy

def addToggleConsole(kc) :
	print( 'Creating Toggle GUI Console Event to ' + getPythonKeyName(kc) )
	hide = ScriptAction.create()
	hide.game = game
	hide.script = "game.gui.hideConsole()"
	show = ScriptAction.create()
	show.game = game
	trigger = game.event_manager.createKeyTrigger(kc)
	show.script = "game.gui.showConsole()"

	# Create a proxy that handles the toggling between two different actions
	toggle = ToggleActionProxy.create()
	toggle.action_on = show
	toggle.action_off = hide

	trigger.action_down = toggle

def addToggleEditor(kc) :
	print( 'Creating Toggle GUI Editor Event to ' + getPythonKeyName(kc) )
	hide = ScriptAction.create()
	hide.game = game
	hide.script = "game.gui.hideEditor()"
	show = ScriptAction.create()
	show.game = game
	trigger = game.event_manager.createKeyTrigger(kc)
	show.script = "game.gui.showEditor()"

	# Create a proxy that handles the toggling between two different actions
	toggle = ToggleActionProxy.create()
	toggle.action_on = show
	toggle.action_off = hide

	trigger.action_down = toggle


# Add a head tracker support
mapHeadTracker("glassesTrigger")

# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )
addQuitEvent(KC.Q)
addScreenshotAction(KC.F10)
addToggleEditor(KC.F2)
addToggleConsole(KC.GRAVE)

