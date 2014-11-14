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

# globals 
JOYSTICK_TOLERANCE = 0.025

# Dividing into Base controller which has the speed implementation
# and Keyboard controller which has the keyboard implementation
# Keyboard controller is the one that should be inherited by all the old
# Controller implementations
#
# Proper way would be divide the Controller implementation and the
# driver (keyboard, joystick etc.) but that takes too much redesign atm.
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
		#Reference frame (transform) used for scene node translations?
		self.frame = TS.LOCAL
		#Reference used for scene node rotations?
		self.rotation_frame = TS.LOCAL

	# @param nodes is a list of objects to be moved
	# @param t is the time since last frame
	def transform(self, nodes, t):
		if self.disabled:
			return
			
		v = self.mov_dir
		mov_len = v.length()
		
		# TODO this is really bad in so many ways
		# we should check for zero sure then we should clamp
		# the speed without dividing by extremely small number
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
				if self.ref:
					nodes[i].translate(self.rotation * mov, self.ref)
				else:
					nodes[i].translate(self.rotation * mov, self.frame)


		axis = self.rot_axis
		axis_len = axis.normalise()
		angle = Radian(self.angular_speed*float(t)*min(axis_len, 1.0))
		q = Quaternion(angle, axis)
		
		# TODO this shouldn't be called if rot is zero
		for i in range(len(nodes)):
			nodes[i].rotate(q, self.rotation_frame)
		


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
		self.reset_rotation()
		self.reset_translation()

	def enable(self):
		self.disabled = False

	def toggle_disable(self) :
		if not self.disabled:
			self.disable()
		else :
			self.enable()

class KeyboardController(Controller):
	def __init__(self, speed = 5, angular_speed = Degree(90), reference=None,
			rotation = Quaternion(1, 0, 0, 0), high_speed = 10):
		Controller.__init__(self, speed, angular_speed,
				reference=reference, high_speed=high_speed)

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
		

# Keyboard Object Controller
class ObjectController(KeyboardController):
	def __init__(self, node, speed = 5, angular_speed = Degree(90),
			reference=None, high_speed=10):
		KeyboardController.__init__(self, speed, angular_speed,
				reference=reference, high_speed=high_speed)
		self.node = node

	def progress(self, t):
		nodes = [self.node]
		self.transform(nodes, t)

# Keyboard Selection Controller
class SelectionController(KeyboardController):
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None, rotation = Quaternion(1, 0, 0, 0)):
		KeyboardController.__init__(self, speed, angular_speed, reference, rotation)

	def progress(self, t):
		self.transform(game.scene.selection, t)

class RigidBodyController(KeyboardController):
	def __init__(self, body):
		KeyboardController.__init__(self)
		self.body = body

	def progress(self, t):
		bodies = [self.body]
		self.transform(bodies, t)

# Separate controllers for Keyboard, Razer, Joystick
# Clumping them into one is messy, bad design and error prone,
# rather have some weird speeds with all three controllers enabled and used
# at the same time.
#
# Keyboard Camera Controller
#
# TODO add a FPS camera version that uses both mouse and keyboard
class CameraController(KeyboardController):
	def __init__(self, speed = 0.5, angular_speed = Degree(30), reference=None, high_speed=10, head_direction = False):
		KeyboardController.__init__(self, speed, angular_speed, reference, high_speed=high_speed)
		self.head_direction = head_direction

	def progress(self, t):
		# TODO this is Oculus specific need to use oculus cb for it
		# not a boolean switch
		#
		# need to figure out the priority if we have tracked controller
		# and Oculus which one will we use?
		if self.head_direction:
			self.rotation = game.player.head_transformation.quaternion
		nodes = [game.player.camera_node]
		self.transform(nodes, t)


# Razer Controller
# for camera now
# showing the razer controllers (both of them) as ray objects
#
# Controls
# Left joystick axis controls camera to the direction you point
# TODO add rotation
# high speed movement
# object picking
class RazerCameraController(Controller) :
	def __init__(self, speed = 5, angular_speed = Degree(30), reference=None, high_speed=10):
		Controller.__init__(self, speed, angular_speed, reference, high_speed=high_speed)
		self.head_direction = False

		# Should never be called without razer hydra
		assert(game.razer_hydra)

		game.razer_hydra.add_listener(self.razer_hydra_cb)

		# Create a ray object to show where the controller is
		# FIXME the ray objects have a huge performance hit.
		# It halves the FPS on both release and debug versions
		# we need to improve the performance of the ray object
		# and we might need to add switches here so they aren't created if not
		# needed.
		mat_name = "finger_sphere/red"
		ray = game.scene.createRayObject("razer_ray_left", mat_name)
		ray.direction = Vector3(0, 0, -1)
		ray.sphere_radius = 0.2
		#ray.length = 20
		# Doesn't affect performance that much if the spheres are on or off
		ray.draw_collision_sphere = True
		ray.collision_detection = True
		self.left_ray = game.scene.createSceneNode("razer_hydra_left")
		self.left_ray.attachObject(ray)
		self.left_ray.hide()

		mat_name = "finger_sphere/green"
		ray = game.scene.createRayObject("razer_ray_right", mat_name)
		ray.direction = Vector3(0, 0, -1)
		ray.sphere_radius = 0.2
		#ray.position = Vector3(0, 0, 10)
		#ray.length = 20
		# Doesn't affect performance that much if the spheres are on or off
		ray.draw_collision_sphere = True
		ray.collision_detection = True
		self.right_ray = game.scene.createSceneNode("razer_hydra_right")
		self.right_ray.attachObject(ray)
		self.right_ray.hide()

		self.controller_t_left = Transform()
		self.controller_t_right = Transform()


	def razer_hydra_cb(self, evt):
		if evt.joystick == RH_JOYSTICK.LEFT :
			# dunno if this is good with two axes, might be easier to controll
			# with just forward axis
			v = Vector3(evt.axis_x, 0, -evt.axis_y)
			self.mov_dir = evt.transform.quaternion * v
			# TODO add ray casting object
			# should be drawn with position ofset from the camera
			# with rotation using the quaternion
			#
			# save the joystick event for 
			# for some reason this is really messy
			self.controller_t_left = Transform(evt.transform)
			self.left_ray.show()
		elif evt.joystick == RH_JOYSTICK.RIGHT :
			self.controller_t_right = Transform(evt.transform)
			self.right_ray.show()


	def progress(self, t):
		# TODO this is Oculus specific need to use oculus cb for it
		# not a boolean switch
		#
		# need to figure out the priority if we have tracked controller
		# and Oculus which one will we use?
		#if self.head_direction:
		#	self.rotation = game.player.head_transformation.quaternion
		nodes = [game.player.camera_node]
		self.transform(nodes, t)

		# copy the camera position to ray object position
		# also add the controller position to it
		# set the orientation from controller
		# 
		# There is a problem with accessing transform since it seems to
		# copy the reference and not make a copy of it when it's changed
		# like it should
		# 
		# TODO what is the difference between head and the magnetic tracking
		# system?
		# it is not head height (0, 1.5, 0)
		# I think we need to have offset from ground in the RazerHydra object
		# then it's head_position - tracker_base_position
		# this is good, just need to replace them with variables
		# and add options for setting those variables in config file
		head_offset = Vector3(0, 1.5, 0) - Vector3(0.45, 0.5, 0)
		p = Vector3(game.player.camera_node.position) + head_offset
		q = Quaternion(game.player.camera_node.orientation)

		self.left_ray.position = p + self.controller_t_left.position
		self.left_ray.orientation = q * self.controller_t_left.quaternion

		# right joystick
		self.right_ray.position = p + self.controller_t_right.position
		self.right_ray.orientation = q * self.controller_t_right.quaternion

# TODO this needs to be divided into multiple different controllers
#
# One handed for Aimon (and similar controllers)
#
# PSController (for playstation controllers and similar e.g. RumblePad)
#
# Base class for all joysticks
# needs a more sophisticated event system for joystick messages
# to be really used.
class JoystickCameraController(Controller):
	def __init__(self, speed = 5, angular_speed = Degree(90), reference=None,
			rotation = Quaternion(1, 0, 0, 0), high_speed = 10):
		Controller.__init__(self, speed, angular_speed,
				reference=reference, high_speed=high_speed)

		# Add a listener for events
		trigger = game.event_manager.createJoystickTrigger()
		trigger.add_listener(self.update_joystick)

		self.head_direction = False

	# Inherit and define to use the base class
	#def update_joystick(self, evt, i) :

	def progress(self, t):
		# TODO this is Oculus specific need to use oculus cb for it
		# not a boolean switch
		#
		# need to figure out the priority if we have tracked controller
		# and Oculus which one will we use?
		if self.head_direction:
			self.rotation = game.player.head_transformation.quaternion
		nodes = [game.player.camera_node]
		self.transform(nodes, t)

# WandCameraController
# one handed joystick, uses tracking if possible?
# The basic version is working for Aimon controllers
# Also works for RumblePad (the wired model at least)
# TODO not tested with Splitfish
# 
# TODO add tracking support
# TODO add rotation
# TODO add Oculus support
class WandCameraController(JoystickCameraController):
	def __init__(self, speed = 5, angular_speed = Degree(90), reference=None,
			rotation = Quaternion(1, 0, 0, 0), high_speed = 10):
		JoystickCameraController.__init__(self, speed, angular_speed,
				reference=reference, high_speed=high_speed)


	def update_joystick(self, evt, i) :
		if evt.type == JOYSTICK_EVENT_TYPE.AXIS:
			# TODO this causes problems if we release axis first
			if evt.state.no_buttons_down :
				# For some reason this doesn't reset to zero properly
				# TODO we need to have these configurable for now hard coded
				# for ps2 one handed joystick
				# TODO we need to clamp the result to zero if it's small enough
				# fix for splitfish (has a static error in z axis)
				x = evt.state.axes[AXIS_W]
				z = evt.state.axes[AXIS_Z]

				if(abs(z) < JOYSTICK_TOLERANCE) : z = 0

				self.mov_dir = Vector3(x, 0, z)

			# TODO add rotation to a button + axis
	

from functools import partial

def create_camera_controller() :
	print('Creating Active camera controller using config')
	print('Options : ', game.options)

	# copy the options
	opt = game.options.camera
	print(opt)

	controller = CameraController(opt.speed, opt.angular_speed,
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
	if opt.joystick_enabled:
		print("JOYSTICK camera controller enabled")
		# TODO we need to select the controller based on the connected device
		# this of course removes any kind of hotplug but has to be done
		# so we don't get one handed and two handed joysticks mixed up
		# not to mention proper joysticks not gamepads
		#
		# We can add multiple controller support with DeviceID/VendorID
		# dunno if it's worth though
		# since a cleaner inplementation would be to separate them in the c++
		# interface instead and map devices to callbacks and not a generic
		# joystick event.
		#
		# Selecting here which joystick controller we use
		# would require us to first get the event message which tells us
		# what joystick is connected.
		# For now I'd say leave it but for later it might be something
		# worthy to consider.
		joy_controller = WandCameraController()
		game.event_manager.frame_trigger.add_listener(joy_controller.progress)

	# TODO should have an option for this also
	if game.razer_hydra :
		print("Creating Razer Camera Controller")
		rzr_hydra = RazerCameraController()
		game.event_manager.frame_trigger.add_listener(rzr_hydra.progress)

	game.event_manager.frame_trigger.add_listener(controller.progress)

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

	game.event_manager.frame_trigger.add_listener(controller.progress)

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
		tracker_trigger.add_listener(tms.move)
		trigger.add_listener(tms.switch_state)
	else:
		print("ERROR : No such trigger. Not adding tracker move selection.")

# Used in robot_project, generated_physics, perapora_physics, perapora_kinematics
def addRigidBodyController(body):
	controller = RigidBodyController(body)

	trig_list = [(controller.right, KC.K),
			(controller.left, KC.H),
			(controller.forward, KC.U),
			(controller.backward, KC.J),
			(controller.up, KC.O),
			(controller.down, KC.Y),
			]

	for obj in trig_list:
		# Function key pair (dunno might need to be key function pair)
		f1 = partial(obj[0], val=1)
		f2 = partial(obj[0], val=-1)
		trigger = game.event_manager.createKeyTrigger(obj[1])
		trigger.addKeyDownListener(f1)
		trigger.addKeyUpListener(f2)

	game.event_manager.frame_trigger.add_listener(controller.progress)

	# TODO this should probably be relative to camera for most use cases
	# using PARENT because otherwise rotation of the body would cause problems
	controller.frame = TS.PARENT
	return controller



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
	trigger.add_listener(stereo.toggle)

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
	trigger.add_listener(setHeadTransform)


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

