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
	addKeyActionsForAxis(trans_action_proxy, Vector3(1, 0, 0), KC.D, KC.A)
	addKeyActionsForAxis(trans_action_proxy, Vector3(0, 0, 1), KC.S, KC.W)
	addKeyActionsForAxis(trans_action_proxy, Vector3(0, 1, 0), KC.PGUP, KC.PGDOWN)

	# Create the rotation action using a proxy
	rot_action_proxy = MoveActionProxy.create()
	rot_action_proxy.enableRotation()
	# This is not useful, maybe using Q and E
	addKeyActionsForAxis(rot_action_proxy, Vector3(0, 1, 0), KC.Q, KC.E)

	# Create the real action
	trans_action = MoveNodeAction.create()
	trans_action.scene_node = node
	trans_action.speed = speed
	trans_action.angular_speed = Radian(angular_speed)
	# Add the real action to the proxies
	trans_action_proxy.action = trans_action
	rot_action_proxy.action = trans_action
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
def addQuitEvent( kc, key_mod = KEY_MOD.NONE ) :
	trigger = game.event_manager.createKeyTrigger(kc, key_mod)
	trigger.addKeyDownListener(game.quit)

def addToggleMusicEvent( kc ) :
	print( 'Creating Toggle Music Event to ' + getPythonKeyName(kc) )
	trigger = game.event_manager.createKeyTrigger( kc )
	trigger.addKeyDownListener(game.toggleBackgroundSound)


def addScreenshotAction( kc ) :
	print( 'Adding screenshot action to ' + getPythonKeyName(kc) )
	trigger = game.event_manager.createKeyTrigger( kc )
	trigger.addKeyDownListener(game.player.takeScreenshot)


# Class to handle camera switching between two cameras
class ChangeCamera :
	def __init__(self, cam1, cam2):
	    self.camera1 = cam1
	    self.camera2 = cam2
	    self.on = True
	    self.switch()

	def switch(self):
		if(self.on):
			game.player.camera = self.camera1
		else:
			game.player.camera = self.camera2
		self.on = not self.on

g_change_camera = None

# Change camera toggle
# Use one Key in this case b to change between two active cameras
# Requirement : Camera names must be correct for this to work
#
# If the camera name is incorrect the action will not change the camera
# An error message is printed to std::cerr and the program continues normally
def addToggleActiveCamera( camera1, camera2 ) :
	print( 'Creating Toggle Activate Camera between ' + camera1 + ' and ' + camera2 )
	# Needs to use global because otherwise it would go out of scope
	g_change_camera = ChangeCamera(camera1, camera2)
	trigger = game.event_manager.createKeyTrigger(KC.B)
	trigger.addKeyDownListener(g_change_camera.switch)

# TODO replace with a general purpose toggle
# converts void parameter to bool parameter so we can map
# void events (like key presses) to bool functions (like setVisible)
class HideToggle:
	def __init__(self, node):
		self.on = False
		self.node = node

	def switch(self):
		if self.on:
			self.node.show()
		else:
			self.node.hide()
		self.on = not self.on

g_hide_toggles = []

def addHideEvent(node, kc) :
	print( 'Creating Hide Event for ' + str(node) + ' to ' + getPythonKeyName(kc) )
	# Does not have Delay between changes, but only works on key down
	# so user would need to press key up and down multiple times
	toggle = HideToggle(node)
	# Uses global storage so they don't go out of scope
	g_hide_toggles.append(toggle)
	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.addKeyDownListener(toggle.switch)

original_ipd = 0
def stereo_off():
	if(game.player.ipd > 0):
		original_ipd = game.player.ipd
	
	game.player.ipd = 0

def stereo_on():
	game.player.ipd = original_ipd

def addToggleStereo(kc) :
	off_action = ScriptAction.create()
	off_action.game = game
	off_action.script = "stereo_on()"

	on_action = ScriptAction.create()
	on_action.game = game
	on_action.script = "stereo_off()"

	toggle = ToggleActionProxy.create()
	toggle.action_on = on_action
	toggle.action_off = off_action

	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.action_down = toggle

def addToggleConsole(kc) :
	print( 'Creating Toggle GUI Console Event to ' + getPythonKeyName(kc) )
	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.addKeyDownListener(game.gui.toggleConsole)

def addToggleEditor(kc) :
	print( 'Creating Toggle GUI Editor Event to ' + getPythonKeyName(kc) )
	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.addKeyDownListener(game.gui.toggleEditor)

# Add a head tracker support
mapHeadTracker("glassesTrigger")


# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )
addScreenshotAction(KC.F10)
addToggleEditor(KC.F2)
addToggleConsole(KC.GRAVE)
# Not working yet
#addToggleStereo(KC.F12)

#addQuitEvent(KC.ESCAPE)

