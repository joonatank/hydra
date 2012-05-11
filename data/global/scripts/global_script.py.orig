# -*- coding: utf-8 -*-

def eulerToQuat(x, y, z):
	q_x = Quaternion(Degree(x), Vector3(1, 0, 0))
	q_y = Quaternion(Degree(y), Vector3(0, 1, 0))
	q_z = Quaternion(Degree(z), Vector3(0, 0, 1))
	return q_x*q_y*q_z

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

# Old function that uses the action system and proxies
def createCameraMovementsOld(node, speed = 5, angular_speed = Degree(90)) :
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

# TODO separate the interface and the progress implementation
# so we can have controllers for a group of objects and selections
# as well as single objects.
class Controller:
	def __init__(self, speed = 5, angular_speed = Degree(90), reference=None, rotation = Quaternion(1, 0, 0, 0)):
		self.speed = speed
		self.angular_speed = angular_speed
		self.mov_dir = Vector3.zero
		self.rot_axis = Vector3.zero
		self.ref = reference
		self.rotation = rotation

	def transform(self, nodes, t):
		# Normalises the move dir, this works for keyboard but it
		# does not work for joysticks
		# for joysticks we need to clip the length at 1
		if self.mov_dir.length() != 0:
			v = self.mov_dir/self.mov_dir.length()
			mov = self.speed*float(t)*v
			# can not use for n in nodes because of missing
			# by-value converter.
			for i in range(len(nodes)):
				# Hack to handle rigid bodies
				# Should use a bit more convenient system
				# as the none reference is used to indicate
				# world coordinates
				# but on the other hand local reference is
				# impossible to indicate if the list contains
				# more than one element
				if self.ref:
					nodes[i].translate(self.rotation * mov, self.ref)
				else:
					nodes[i].translate(self.rotation * mov)

		axis = self.rot_axis
		if axis.length() != 0:
			axis.normalise()
			angle = Radian(self.angular_speed*float(t))
			q = Quaternion(angle, axis)
			for i in range(len(nodes)):
				nodes[i].rotate(q)

	def up(self):
		self.mov_dir += Vector3.unit_y

	def down(self):
		self.mov_dir -= Vector3.unit_y

	def left(self):
		self.mov_dir -= Vector3.unit_x
		
	def right(self):
		self.mov_dir += Vector3.unit_x

	def forward(self):
		self.mov_dir -= Vector3.unit_z

	def backward(self):
		self.mov_dir += Vector3.unit_z

	def rotate_right(self):
		self.rot_axis -= Vector3.unit_y

	def rotate_left(self):
		self.rot_axis += Vector3.unit_y

	def rotate_up(self):
		self.rot_axis -= Vector3.unit_x

	def rotate_down(self):
		self.rot_axis += Vector3.unit_x

	def roll_right(self):
		self.rot_axis -= Vector3.unit_z

	def roll_left(self):
		self.rot_axis += Vector3.unit_z


class ObjectController(Controller):
	def __init__(self, node, speed = 5, angular_speed = Degree(90)):
		Controller.__init__(self, speed, angular_speed, node)
		self.node = node

	def progress(self, t):
		nodes = [self.node]
		self.transform(nodes, t)

class SelectionController(Controller):
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None, rotation = Quaternion(1, 0, 0, 0)):
		Controller.__init__(self, speed, angular_speed, reference, rotation)

	def progress(self, t):
		self.transform(game.scene.selection, t)

class RigidBodyController(Controller):
	def __init__(self, body):
		Controller.__init__(self)
		self.body = body

	def progress(self, t):
		bodies = [self.body]
		self.transform(bodies, t)

class ActiveCameraController(Controller):
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None):
		Controller.__init__(self, speed, angular_speed, reference)

	def progress(self, t):
		nodes = [game.player.camera_node]
		self.transform(nodes, t)

# New system using classes and signal callbacks
# TODO add a separate controller for joystick values
# selectable using a flag for example
def createCameraMovements(node = None, speed = 5, angular_speed = Degree(90)) :
	# TODO stupid string casting, can we somehow remove it?
	print( 'Creating Move Active camera event.')

	camera_movements = ActiveCameraController(speed, angular_speed)

	trigger = game.event_manager.createKeyTrigger(KC.D)
	trigger.addKeyDownListener(camera_movements.right)
	trigger.addKeyUpListener(camera_movements.left)

	trigger = game.event_manager.createKeyTrigger(KC.A)
	trigger.addKeyDownListener(camera_movements.left)
	trigger.addKeyUpListener(camera_movements.right)

	trigger = game.event_manager.createKeyTrigger(KC.S)
	trigger.addKeyDownListener(camera_movements.backward)
	trigger.addKeyUpListener(camera_movements.forward)

	trigger = game.event_manager.createKeyTrigger(KC.W)
	trigger.addKeyDownListener(camera_movements.forward)
	trigger.addKeyUpListener(camera_movements.backward)

	trigger = game.event_manager.createKeyTrigger(KC.PGUP)
	trigger.addKeyDownListener(camera_movements.up)
	trigger.addKeyUpListener(camera_movements.down)

	trigger = game.event_manager.createKeyTrigger(KC.PGDOWN)
	trigger.addKeyDownListener(camera_movements.down)
	trigger.addKeyUpListener(camera_movements.up)

	trigger = game.event_manager.createKeyTrigger(KC.Q)
	trigger.addKeyDownListener(camera_movements.rotate_left)
	trigger.addKeyUpListener(camera_movements.rotate_right)

	trigger = game.event_manager.createKeyTrigger(KC.E)
	trigger.addKeyDownListener(camera_movements.rotate_right)
	trigger.addKeyUpListener(camera_movements.rotate_left)

	game.event_manager.frame_trigger.addListener(camera_movements.progress)


# TODO add a possibility to speed up movements using SHIFT modifier
# @param speed linear speed of the selection
# @param angular_speed rotation speed of the selection
# @param reference The object whom coordinate system is used for translation, usually camera
def addMoveSelection(speed = 0.3, angular_speed = Degree(40), reference=None, rotation = Quaternion(1, 0, 0, 0)) :
	selection_movements = SelectionController(speed, angular_speed, reference, rotation)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD4)
	trigger.addKeyDownListener(selection_movements.right)
	trigger.addKeyUpListener(selection_movements.left)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD6)
	trigger.addKeyDownListener(selection_movements.left)
	trigger.addKeyUpListener(selection_movements.right)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD5)
	trigger.addKeyDownListener(selection_movements.backward)
	trigger.addKeyUpListener(selection_movements.forward)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD8)
	trigger.addKeyDownListener(selection_movements.forward)
	trigger.addKeyUpListener(selection_movements.backward)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD9)
	trigger.addKeyDownListener(selection_movements.up)
	trigger.addKeyUpListener(selection_movements.down)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD7)
	trigger.addKeyDownListener(selection_movements.down)
	trigger.addKeyUpListener(selection_movements.up)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD4, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.rotate_left)
	trigger.addKeyUpListener(selection_movements.rotate_right)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD6, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.rotate_right)
	trigger.addKeyUpListener(selection_movements.rotate_left)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD8, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.roll_right)
	trigger.addKeyUpListener(selection_movements.roll_left)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD5, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.roll_left)
	trigger.addKeyUpListener(selection_movements.roll_right)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD9, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.rotate_up)
	trigger.addKeyUpListener(selection_movements.rotate_down)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD7, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.rotate_down)
	trigger.addKeyUpListener(selection_movements.rotate_up)

	game.event_manager.frame_trigger.addListener(selection_movements.progress)

# @param tracker the tracker used for moving the objects
# @param trigger the trigger used to trigger start moving and stop moving
def addTrackerMoveSelection(tracker_trigger_name, trigger) :
	class TrackerMoveSelection :
		def __init__(self) :
			self.on_move = False

		def switch_state(self) :
			self.on_move = not self.on_move

		def move(self, t) :
			if self.on_move:
				for i in range(len(game.scene.selection)) :
					game.scene.selection[i].transformation = t

	tms = TrackerMoveSelection()
	tracker_trigger = game.event_manager.getTrackerTrigger(tracker_trigger_name)
	tracker_trigger.addListener(tms.move)
	trigger.addListener(tms.switch_state)

def addRigidBodyController(body):
	controller = RigidBodyController(body)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD6)
	trigger.addKeyDownListener(controller.right)
	trigger.addKeyUpListener(controller.left)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD4)
	trigger.addKeyDownListener(controller.left)
	trigger.addKeyUpListener(controller.right)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD8)
	trigger.addKeyDownListener(controller.forward)
	trigger.addKeyUpListener(controller.backward)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD5)
	trigger.addKeyDownListener(controller.backward)
	trigger.addKeyUpListener(controller.forward)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD9)
	trigger.addKeyDownListener(controller.up)
	trigger.addKeyUpListener(controller.down)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD7)
	trigger.addKeyDownListener(controller.down)
	trigger.addKeyUpListener(controller.up)

	game.event_manager.frame_trigger.addListener(controller.progress)


def mapHeadTracker(name) :
	def setHeadTransform(t):
		game.player.head_transformation = t

	if not game.event_manager.hasTrackerTrigger(name):
		print("Creating a fake tracker trigger")
		tracker = Tracker.create("FakeHeadTracker")
		tracker.n_sensors = 1
		t = Transform(Vector3(0, 1.5, 0))
		tracker.getSensor(0).default_transform = t
		# Create trigger
		trigger = game.event_manager.createTrackerTrigger(name)
		tracker.getSensor(0).trigger = trigger
		game.tracker_clients.addTracker(tracker)

	trigger = game.event_manager.getTrackerTrigger(name)
	trigger.addListener(setHeadTransform)


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

def addToggleStereo(kc) :
	class StereoToggle:
		def __init__(self) :
			self.ipd = 0

		def toggle(self):
			tmp = game.player.ipd
			game.player.ipd = self.ipd
			self.ipd = tmp

	stereo = StereoToggle() 
	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.addListener(stereo.toggle)

def addToggleConsole(kc) :
	print( 'Creating Toggle GUI Console Event to ' + getPythonKeyName(kc) )
	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.addKeyDownListener(game.gui.console.toggle_visible)

def addToggleEditor(kc) :
	print( 'Creating Toggle GUI Editor Event to ' + getPythonKeyName(kc) )
	trigger = game.event_manager.createKeyTrigger(kc)
	trigger.addKeyDownListener(game.gui.editor.toggle_visible)

# Add a head tracker support
mapHeadTracker("glassesTrigger")


# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )
addScreenshotAction(KC.F10)
addToggleEditor(KC.F2)
addToggleConsole(KC.GRAVE)
addToggleStereo(KC.F12)

#addQuitEvent(KC.ESCAPE)

