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

# TODO separate the interface and the progress implementation
# so we can have controllers for a group of objects and selections
# as well as single objects.
class Controller:
	def __init__(self, speed = 5, angular_speed = Degree(90), reference=None,
			rotation = Quaternion(1, 0, 0, 0), high_speed = 10):
		self.speed = speed
		self.low_speed = speed
		self.high_speed = high_speed
		self.angular_speed = angular_speed
		self.mov_dir = Vector3.zero
		self.rot_axis = Vector3.zero
		self.ref = reference
		self.rotation = rotation
		self.disabled = False
		self.frame = TS.LOCAL

	def transform(self, nodes, t):
		if self.disabled:
			return;

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
				try:
					if self.ref:
						nodes[i].translate(self.rotation * mov, self.ref)
					else:
						nodes[i].translate(self.rotation * mov, self.frame)
				except:
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

	def toggle_high_speed(self):
		if self.speed < self.high_speed:
			self.speed = self.high_speed
		else:
			self.speed = self.low_speed

	def disable(self):
		self.disabled = True

	def enable(self):
		self.disabled = False


class ObjectController(Controller):
	def __init__(self, node, speed = 5, angular_speed = Degree(90), high_speed=10):
		Controller.__init__(self, speed, angular_speed, node, high_speed=high_speed)
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
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None, high_speed=10):
		Controller.__init__(self, speed, angular_speed, reference, high_speed=high_speed)

	def progress(self, t):
		nodes = [game.player.camera_node]
		self.transform(nodes, t)

# Old interface map it to new one and remove in Hydra-0.4
# node is discarded as we are using active controller now
def createCameraMovements(node = None, speed = 5, angular_speed = Degree(90)) :
	return createCameraController(speed, angular_speed)

# New system using classes and signal callbacks
# TODO add a separate controller for joystick values
# selectable using a flag for example
# @return ActiveCameraController
def createCameraController(speed = 5, angular_speed = Degree(90), high_speed = 10) :
	print( 'Creating Active camera controller.')

	camera_movements = ActiveCameraController(speed, angular_speed, high_speed=high_speed)

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

	# Create speed toggle to both shifts
	triggers = [game.event_manager.createKeyTrigger(KC.LSHIFT),
		game.event_manager.createKeyTrigger(KC.RSHIFT)]
	for t in triggers:
		t.addKeyDownListener(camera_movements.toggle_high_speed)
		t.addKeyUpListener(camera_movements.toggle_high_speed)

	game.event_manager.frame_trigger.addListener(camera_movements.progress)

	return camera_movements


# TODO add a possibility to speed up movements using SHIFT modifier
# @param speed linear speed of the selection
# @param angular_speed rotation speed of the selection
# @param reference The object whom coordinate system is used for translation, usually camera
def addMoveSelection(speed = 0.3, angular_speed = Degree(40), reference=None, rotation = Quaternion(1, 0, 0, 0)) :
	return createSelectionController(speed, angular_speed, reference, rotation)

def createSelectionController(speed = 0.3, angular_speed = Degree(40), reference=None, rotation = Quaternion(1, 0, 0, 0)) :
	selection_movements = SelectionController(speed, angular_speed, reference, rotation)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD6)
	trigger.addKeyDownListener(selection_movements.right)
	trigger.addKeyUpListener(selection_movements.left)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD4)
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

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD6, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.rotate_right)
	trigger.addKeyUpListener(selection_movements.rotate_left)

	trigger = game.event_manager.createKeyTrigger(KC.NUMPAD4, KEY_MOD.CTRL)
	trigger.addKeyDownListener(selection_movements.rotate_left)
	trigger.addKeyUpListener(selection_movements.rotate_right)

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

	return selection_movements;

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

	if game.event_manager.hasTrackerTrigger(tracker_trigger_name):
		tracker_trigger = game.event_manager.getTrackerTrigger(tracker_trigger_name)
		tms = TrackerMoveSelection()
		tracker_trigger.addListener(tms.move)
		trigger.addListener(tms.switch_state)
	else:
		print("ERROR : No such trigger. Not adding tracker move selection.")

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
	if game.gui.console:
		trigger = game.event_manager.createKeyTrigger(kc)
		trigger.addKeyDownListener(game.gui.console.toggle_visible)

# Selection Buffer
class SelectionSet:
	def __init__(self):
		self.selection = []
		self.index = 0

	# Add an array of objects that are to be controlled as one
	def add_objects(self, names):
		self.selection.append([])
		for n in names:
			sn = game.scene.getSceneNode(n)
			if sn:
				self.selection[-1].append(sn)

	def __str__(self):
		s = "SelectionSet :\n"
		for sn_list in self.selection:
			for sn in sn_list:
				s += (" " + sn.name)
			s += "\n"
		return s

	def change_selection(self):
		self.index += 1
		if self.index >= len(self.selection):
			self.index = 0

		game.scene.clearSelection()
		for sn in self.selection[self.index]:
			game.scene.addToSelection(sn)


# Create a basic directional light with a decent angle
# Return the sun scene node
# TODO this should be a bit more complex and use the sky simulator
# so the sun angle and intensity is dependent on the sky state
def create_sun():
	sun = game.scene.createSceneNode("sun")
	sun_l = game.scene.createLight("sun")
	sun_l.type = "directional"
	sun.attachObject(sun_l)
	sun.orientation = Quaternion(-0.4114, 0.9114, 0, 0)

	return sun

# Creates basic plane ground for demos
# Return ground scene node
def create_ground(size=40):
	ground_mesh = game.mesh_manager.createPlane("ground", size, size)
	ground_node = game.scene.createSceneNode("ground")
	ground = game.scene.createEntity("ground", 'ground', True)
	ground_node.attachObject(ground)
	ground.material_name = "ground/bump_mapped/shadows"
	ground.cast_shadows = False

	return ground_node

def toggle_pause():
	# Game can be in both stopped and paused state so test for played instead
	# of paused
	if game.playing:
		game.pause()
	else:
		game.play()

# Add a head tracker support
mapHeadTracker("glassesTrigger")


# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )
addScreenshotAction(KC.F10)
addToggleConsole(KC.GRAVE)
addToggleStereo(KC.F12)

#addQuitEvent(KC.ESCAPE)

class FrustumToggler:
	def __init__(self):
		game.player.head_frustum_x = False
		game.player.head_frustum_y = True
		game.player.head_frustum_z = False
		#self.cyclop_vector = Vector3(0, 0, -0.15)
		self.cyclop_vector = Vector3(0, 0, 0)
		self.cyclop = False

		game.player.asymmetric_stereo_frustum = True

		self.all_on()

	def all_on(self):
		self.on = True

		game.player.cyclop_transformation = Transform(self.cyclop_vector)
		self.cyclop = True
		game.player.head_frustum_x = True
		game.player.head_frustum_y = True
		game.player.head_frustum_z = True

	def all_off(self):
		self.on = False

		game.player.head_frustum_x = False
		game.player.head_frustum_y = True
		game.player.head_frustum_z = False
		game.player.cyclop_transformation = Transform()
		self.cyclop = False
		

	def toggle_all(self):
		if self.on:
			self.all_off()
		else:
			self.all_on()

	def toggle_asymmetric_frustum(self):
		game.player.asymmetric_stereo_frustum = not game.player.asymmetric_stereo_frustum

	def toggle_head_frustum(self):
		game.player.head_frustum_x = not game.player.head_frustum_x
		game.player.head_frustum_z = not game.player.head_frustum_z

	def toggle_cyclop(self):
		if self.cyclop:
			game.player.cyclop_transformation = Transform(self.cyclop_vector)
		else:
			game.player.cyclop_transformation = Transform()
		self.cyclop = not self.cyclop

print("Creating frustum toggler")
toggler = FrustumToggler()
trigger = game.event_manager.createKeyTrigger(KC.F5, KEY_MOD.CTRL)
trigger.addListener(toggler.toggle_asymmetric_frustum)

trigger = game.event_manager.createKeyTrigger(KC.F6, KEY_MOD.CTRL)
trigger.addListener(toggler.toggle_head_frustum)

trigger = game.event_manager.createKeyTrigger(KC.F7, KEY_MOD.CTRL)
trigger.addListener(toggler.toggle_cyclop)

trigger = game.event_manager.createKeyTrigger(KC.F5)
trigger.addListener(toggler.toggle_all)

