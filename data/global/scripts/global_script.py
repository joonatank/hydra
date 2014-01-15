# -*- coding: utf-8 -*-

def eulerToQuat(x, y, z):
	q_x = Quaternion(Degree(x), Vector3(1, 0, 0))
	q_y = Quaternion(Degree(y), Vector3(0, 1, 0))
	q_z = Quaternion(Degree(z), Vector3(0, 0, 1))
	return q_x*q_y*q_z

# AXIS defines
AXIS_X = 0
AXIS_Y = 1
AXIS_Z = 2
AXIS_W = 3

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
		#Translational signal:
		self.mov_dir = Vector3.zero
		#Rotational signal:
		self.rot_axis = Vector3.zero
		#I believe this is reference object for rigidbody translation?
		self.ref = reference
		#Initial rotation eg. misalignment:
		self.rotation = rotation
		self.disabled = False
		# Default to disabled joystick because they are same for all objects
		# Enable them for camera only when creating camera controller
		self.joystick_disabled = True
		#Reference frame (transform) used for scene node translations?
		self.frame = TS.LOCAL
		#Reference used for scene node rotations?
		self.rotation_frame = TS.LOCAL

		# Error tolerance for joystick zero
		self.joystick_tolerance = 0.025

	def transform(self, nodes, t):
		if self.disabled:
			return
			
		v = self.mov_dir
		mov_len = v.length()
		
		if mov_len != 0:
			v /= mov_len
			#Now the signal is being clamped to 1.0.
			#Basically the multiplier is signal size
			#if it's smaller than 1.0 or 1.0 if it's
			#greater than it. Same for rotation.
			mov = self.speed*float(t)*v*min(mov_len, 1.0)
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
		axis_len = axis.normalise()
		angle = Radian(self.angular_speed*float(t)*min(axis_len, 1.0))
		q = Quaternion(angle, axis)
		
		#Bullet rigidbodies doesn't have rotate method so this won't work
		#--> rigidbody controller won't work
		for i in range(len(nodes)):
			nodes[i].rotate(q, self.rotation_frame)
		

	def up(self, val):
		self.mov_dir.y += val

	def down(self, val):
		self.mov_dir.y -= val

	def left(self, val):
		self.mov_dir.x -= val
		
	def right(self, val):
		self.mov_dir.x += val

	def forward(self, val):
		self.mov_dir.z -= val

	def backward(self, val):
		self.mov_dir.z += val

	def rotate_right(self, val):
		self.rot_axis.y -= val

	def rotate_left(self, val):
		self.rot_axis.y += val

	def rotate_up(self, val):
		self.rot_axis.x += val

	def rotate_down(self, val):
		self.rot_axis.x -= val

	def roll_right(self, val):
		self.rot_axis.z -= val

	def roll_left(self, val):
		self.rot_axis.z += val
		
	# TODO need to add a boolean to enable/disable the joystick
	# now it will enable it for all objects not just camera
	# Actually it needs to be enabled when it's created
	# only camera controller will enable it
	def update_joystick(self, evt, i) :
		if self.joystick_disabled:
			return

		# Override the move dir
		# dunno if this is the best way to do it
		# might need to add and clamp to avoid issues with using both
		# joystick and keyboard
		if evt.type == JOYSTICK_EVENT_TYPE.AXIS:
			# TODO this causes problems if we release axis first
			if evt.state.no_buttons_down :
				# For some reason this doesn't reset to zero properly
				# TODO we need to have these configurable for now hard coded
				# for ps2 one handed joystick
				# TODO we need to clamp the result to zero if it's small enough
				# fix for splitfish (has a static error in z axis)
				x = evt.state.axes[AXIS_Z]
				z = evt.state.axes[AXIS_Y]
				if(abs(z) < self.joystick_tolerance) : z = 0

				self.mov_dir = Vector3(x, 0, z)
	
	def reset_rotation(self):
		self.rot_axis = Vector3.zero

	def reset_translation(self):
		self.mov_dir = Vector3.zero

	def toggle_high_speed(self):
		if self.speed < self.high_speed:
			self.speed = self.high_speed
		else:
			self.speed = self.low_speed

	def disable(self):
		self.disabled = True
		# TODO shouldn't these reset rotations and translations

	def enable(self):
		self.disabled = False

	def toggle_disable(self) :
		if not self.disabled:
			self.disable()
		else :
			self.enable()


class ObjectController(Controller):
	def __init__(self, node, speed = 5, angular_speed = Degree(90),
			reference=None, high_speed=10):
		Controller.__init__(self, speed, angular_speed,
				reference=reference, high_speed=high_speed)
		self.node = node

	def progress(self, t):
		nodes = [self.node]
		self.transform(nodes, t)

class SelectionController(Controller):
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None, rotation = Quaternion(1, 0, 0, 0)):
		Controller.__init__(self, speed, angular_speed, reference, rotation)

	def progress(self, t):
		self.transform(game.scene.selection, t)

# FIXME This doesn't work because there's no rotate method for rigidbodies
# (defined in the end of Controller transform method)
class RigidBodyController(Controller):
	def __init__(self, body):
		Controller.__init__(self)
		self.body = body

	def progress(self, t):
		bodies = [self.body]
		self.transform(bodies, t)

class ActiveCameraController(Controller):
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None, high_speed=10, head_direction = False):
		Controller.__init__(self, speed, angular_speed, reference, high_speed=high_speed)
		self.head_direction = head_direction

	def progress(self, t):
		if self.head_direction:
			self.rotation = game.player.head_transformation.quaternion
		nodes = [game.player.camera_node]
		self.transform(nodes, t)

from functools import partial

def create_camera_controller() :
	print('Creating Active camera controller using config')
	print('Options : ', game.options)

	# copy the options
	opt = game.options.camera
	print(opt)

	controller = ActiveCameraController(opt.speed, opt.angular_speed,
			high_speed=opt.high_speed)

	# Configure
	controller.head_direction = opt.head_direction
	controller.disabled = opt.controller_disabled

	# Use partial from functools to make this repetive task a little less daunting
	#
	# syntax would have controller.right = partial(controller.right, 1)
	# and controller.left = partial(controller.right, -1)
	#
	# TODO
	# This can be extended to cover all controllers with a single mapper function
	# that takes a list of tuples to it and maps keys to functions

	trig_list = [(controller.right, opt.right),
			(controller.left, opt.left),
			(controller.forward, opt.forward),
			(controller.backward, opt.backward),
			(controller.up, opt.up),
			(controller.down, opt.down),
			(controller.rotate_right, opt.rotate_right),
			(controller.rotate_left, opt.rotate_left),
			]

	for obj in trig_list:
		# Function key pair (dunno might need to be key function pair)
		f1 = partial(obj[0], val=1)
		f2 = partial(obj[0], val=-1)
		trigger = game.event_manager.createKeyTrigger(obj[1])
		trigger.addKeyDownListener(f1)
		trigger.addKeyUpListener(f2)

	# Create speed toggle to both shifts
	if not opt.high_speed_disabled:
		if opt.high_speed < opt.speed :
			print("Warning : camera high speed is lower than normal speed")

		triggers = [game.event_manager.createKeyTrigger(KC.LSHIFT),
			game.event_manager.createKeyTrigger(KC.RSHIFT)]
		for t in triggers:
			# This is kinda odd, since it allows us to enable high
			# speed with key up and disable it with key down
			t.addKeyDownListener(controller.toggle_high_speed)
			t.addKeyUpListener(controller.toggle_high_speed)

	# Joystick
	# TODO need to change the controller itself to enable configuring the joystick
	controller.joystick_disabled = not opt.joystick_enabled
	if not controller.joystick_disabled :
		trigger = game.event_manager.createJoystickTrigger()
		trigger.addListener(controller.update_joystick)

	game.event_manager.frame_trigger.addListener(controller.progress)

	return controller 


# Uses game.options.selection for configuring
#
# @param reference The object whom coordinate system is used for translation, usually camera
#
# Doesn't need high speed imo
#
# TODO reference and orientation should be inside options
# reference should use the current active camera by default
# if that's enough for all cases we should disable changing of it
def create_selection_controller(reference=None, rotation = Quaternion(1, 0, 0, 0)) :
	opt = game.options.selection
	print("Selection Controller options : ", opt)

	controller = SelectionController(opt.speed, opt.angular_speed,
			reference, rotation)

	# TODO need to add rotations if they are not disabled
	# TODO we should use KC.UNASSIGNED to disable specific direction
	trig_list = [(controller.right, opt.right, KEY_MOD.NONE),
			(controller.left, opt.left, KEY_MOD.NONE),
			(controller.forward, opt.forward, KEY_MOD.NONE),
			(controller.backward, opt.backward, KEY_MOD.NONE),
			(controller.up, opt.up, KEY_MOD.NONE),
			(controller.down, opt.down, KEY_MOD.NONE),
			(controller.rotate_right, opt.right, KEY_MOD.CTRL),
			(controller.rotate_left, opt.left, KEY_MOD.CTRL),
			(controller.roll_left, opt.forward, KEY_MOD.CTRL),
			(controller.roll_right, opt.backward, KEY_MOD.CTRL),
			(controller.rotate_down, opt.up, KEY_MOD.CTRL),
			(controller.rotate_up, opt.down, KEY_MOD.CTRL),
			]

	for obj in trig_list:
		# Function key pair (dunno might need to be key function pair)
		f1 = partial(obj[0], val=1)
		f2 = partial(obj[0], val=-1)
		trigger = game.event_manager.createKeyTrigger(obj[1], obj[2])
		trigger.addKeyDownListener(f1)
		trigger.addKeyUpListener(f2)

	game.event_manager.frame_trigger.addListener(controller.progress)

	return controller;

# TODO where is this used?
#
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

# TODO is this used in anywhere else than generated_physics? if not move it there
# Used in robot_project, generated_physics, perapora_physics, perapora_kinematics
#
# FIXME doesn't work at the moment because RigidBody doesn't have rotate method
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
# TODO this is not properly named
# a selection set is the thing that is game.scene.selection
# this is a SelectionBuffer that allows switching between SelectionSets
# 
# We still need to implement SelectionSet that replaces the one in game.scene.selection
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
def create_sun():
	sun = game.scene.createSceneNode("sun")
	sun_l = game.scene.createLight("sun")
	sun_l.type = "directional"
	sun_l.diffuse = ColourValue(0.5, 0.5, 0.5)
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
	# Bump mapped do not work with deferred shading yet
	#ground.material_name = "ground/bump_mapped/shadows"
	ground.material_name = "ground/flat/shadows"
	ground.cast_shadows = False

	return ground_node

def toggle_pause():
	# Game can be in both stopped and paused state so test for played instead
	# of paused
	if game.playing:
		game.pause()
	else:
		game.play()


#### Head Tracking stuff ####
# Callback for setting head transformation
def setHeadTransform(t):
	game.player.head_transformation = t

def create_fake_tracker(name):
	# Create fake tracker
	assert(not game.event_manager.hasTrackerTrigger(name))

	print("Creating a fake tracker trigger")
	tracker = Tracker.create("FakeHeadTracker")
	tracker.n_sensors = 1
	t = Transform(Vector3(0, 1.5, 0))
	tracker.getSensor(0).default_transform = t
	# Create trigger
	trigger = game.event_manager.createTrackerTrigger(name)
	tracker.getSensor(0).trigger = trigger
	game.tracker_clients.addTracker(tracker)

def map_head_tracker(name) :
	trigger = game.event_manager.getTrackerTrigger(name)
	trigger.addListener(setHeadTransform)


# Starts a head tracker and maps it to head
# Selects between Oculus and Traditional tracking systems
# Using glassesTrigger for traditional tracking (optical and magnetic)
# Uses headTrigger for Oculus 
#
# TODO this does not allow us to change trackers at run time
def initialise_head_tracker():
	oculus_name = "headTrigger"
	t_name = "glassesTrigger"
	if game.event_manager.hasTrackerTrigger(oculus_name) :
		map_head_tracker(oculus_name)
	elif game.event_manager.hasTrackerTrigger(t_name) :
		map_head_tracker(t_name)
	else :
		create_fake_tracker(t_name)
		map_head_tracker(t_name)

	# has traditional head tracker
	# does not have head tracker
#### Head Tracking stuff ####


initialise_head_tracker()


# Add some global events that are useful no matter what the scene/project is
print( 'Adding game events' )

trigger = game.event_manager.createKeyTrigger(KC.F10)
trigger.addKeyDownListener(game.player.takeScreenshot)
addToggleConsole(KC.GRAVE)
addToggleStereo(KC.F12)

#trigger = game.event_manager.createKeyTrigger(KC.ESCAPE)
#trigger.addKeyDownListener(game.quit)

# TODO Should only be enabled if we have HMD connected
def enable_hmd_distortion():
	print("Enabling HMD distortion.")
	renderer.hmd_distortion_enabled = True

def disable_hmd_distortion():
	print("Disabling HMD distortion.")
	renderer.hmd_distortion_enabled = False

trigger = game.event_manager.createKeyTrigger(KC.F5)
trigger.addKeyDownListener(enable_hmd_distortion)

trigger = game.event_manager.createKeyTrigger(KC.F6)
trigger.addKeyDownListener(disable_hmd_distortion)

