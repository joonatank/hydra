# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

# config.getSceneNode gets a reference to already created SceneNode
# For now it's not possible to create SceneNodes from python
# So use this function to get a SceneNode created from .scene file.
# All SceneNodes in .scene file are created and can be retrieved here.
# Tangent space lighting does not work on the ogre_ent


print('Creating Camera SceneNode')
camera = game.scene.createSceneNode("Camera")
cam = game.scene.createCamera("Camera")
cam.near_clip = 0.001
camera.attachObject(cam)
camera.position = Vector3(0, 3, 15)
game.player.camera = camera.name
#create_camera_controller()

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
game.scene.shadows.max_distance = 50
game.scene.shadows.dir_light_extrusion_distance = 100

ground = create_ground(size=100)

sun = create_sun()
sun.orientation = Quaternion(-0.900988, 0.43373, 0, 0)

# Helper class for Assembly
# handles resetting the part properly when sub assemblies are changed
class Part:
	def __init__(self, node):
		self.node = node
		self.transform = node.transformation

	def reset(self):
		# TODO this might cause odd stuff if the parent is moved
		# on the other hand this should not break anything if we reset the whole
		# assembly at once (or just move one part at a time)
		self.node.transformation = self.transform

	def hide(self):
		assert(self.node)
		self.node.hide()

	def show(self):
		assert(self.node)
		self.node.show()

	def select(self):
		assert(self.node)
		print("should select part : ", self.node.name)
		#game.scene.clearSelection()
		#game.scene.addToSelection(self.node)
		self.node.show_bounding_box = True

	def deselect(self):
		self.node.show_bounding_box = False

	def move(self, v):
		assert(self.node)
		self.node.translate(v, TS.WORLD)

class Assembly:
	def __init__(self, root_node = None):
		self.clear()
		if root_node:
			self.create_assembly(root_node)

	# Add a part
	# @param part is a SceneNode
	def add_part(self, part):
		self.parts.append(Part(part))

		# Selecting is done in reverse order
		# starting from self.index = len(self.parts)-1
		# because we want to be unassembling
		#
		# Update this whenever part is added so we can allow part addition
		# outside of create_assembly
		self.index = len(self.parts) -1

	# Create an assembly from scratch
	# @param root_node is the node whose direct children the assembly parts are
	# resets all previous settings
	def create_assembly(self, root_node):
		self.clear()
		for i in range(len(root_node.children)):
			self.add_part(root_node.children[i])


	def clear(self):
		self.parts = []
		self.index = 0	# Current part

	def select_next_part(self):
		# Not doing ring buffer because we might ran into some trouble
		# also it's not intuitive when assembling
		if self.index + 1 < len(self.parts):
			# we need to reset the old parts position
			self.parts[self.index].reset()
			self.parts[self.index].deselect()
			self.index += 1
			self.parts[self.index].show()
			self.parts[self.index].select()

	def select_prev_part(self):
		if self.index > 0:
			# we need to reset the old parts position
			self.parts[self.index].reset()
			self.parts[self.index].hide()
			self.parts[self.index].deselect()
			self.index -= 1
			# select removes the previous selection
			self.parts[self.index].select()

	def move_part(self, v):
		self.parts[self.index].move(v)

	def current_part(self):
		return self.parts[self.index]

	# Show the whole assembly (like it was complete)
	def show_all(self):
		for part in self.parts:
			part.show()

	# Hide all the assemblies that are not yet processed
	def hide_future_steps(self):
		for part in self.parts[self.index+1:]:
			part.hide()
			

# Create assembly
assembly_root = game.scene.getSceneNode("IRB6650S_90-390_M2005_REV1_01")
# list all subnodes in root node for the assembly

assembly = Assembly(assembly_root)


class AssemblyController(Controller):
	def __init__(self, assembly, camera):
		Controller.__init__(self)
		self.assembly = assembly
		self.camera = camera
		self.assembly_control_on = False
		self.camera_initial_wt = self.camera.world_transformation
		self.assembly_shown = False

	def reset_camera(self):
		self.camera.world_transformation = self.camera_initial_wt

	# evt is the event
	# i is index of the component which caused the triggering
	def update_joystick(self, evt, i):
		# TODO add controls
		# - Move the camera from axes (can we use the one in cameracontrol or do we
		# need to implement another one?
		# - reset camera position (select) DONE
		#
		# - changing assembly (index finger buttons) DONE
		# - move assembly DONE
		#	index finger button + axes
		if evt.type == JOYSTICK_EVENT_TYPE.BUTTON_PRESSED:
			if evt.state.is_button_down(5):
				# change selection up
				self.assembly.select_next_part()
			if evt.state.is_button_down(7):
				self.assembly.select_prev_part()
				# change selection down
			if evt.state.is_button_down(9):
				self.reset_camera()
			if evt.state.is_button_down(8):
				if self.assembly_shown:
					self.assembly.hide_future_steps()
				else:
					self.assembly.show_all()
				self.assembly_shown = not self.assembly_shown

		# TODO need to move this to a controller
		# indeed we can't get speed or even consistant movement
		# because this is not called if joystick values do not change

		x = evt.state.axes[0]
		y = evt.state.axes[1]
		z = evt.state.axes[2]
		w = evt.state.axes[3]
		delta = 0.05
		if(abs(x) < delta) : x = 0
		if(abs(y) < delta) : y = 0
		if(abs(z) < delta) : z = 0
		if(abs(w) < delta) : w = 0
		self.mov_dir = Vector3(w, -x, z)
		# switch between moving camera and moving assembly
		# outside of event type thingy because we use axes here
		if evt.state.is_button_down(4):
			self.assembly_control_on = True
			self.speed = 1
			self.rot_axis = Vector3.zero
		else:
			self.assembly_control_on = False
			self.speed = 10
			# Only cameras have rotation
			self.rot_axis = Vector3(0, 1, 0)
			self.angular_speed = Radian(-y)

	def progress(self, t):
		if self.assembly_control_on:
			nodes = [self.assembly.current_part().node]
			self.rotation = Quaternion.identity
		else:
			nodes = [self.camera]
			# We want to move to the Oculus head tracker direction
			self.rotation = game.player.head_transformation.quaternion
		self.transform(nodes, t)


# Buttons on RumblePad (wired)
# 4 (upper) and 6 (lower) left index finger
# 5 (upper) and 7 (lower) right index finger
# 0 - 3 on the pad
# 10 and 11 on the joysticks
# 8 and 9 in the middle (named 9 and 10)
#
def joystick_print(evt, i):
	#print("joystick callback called : ", evt)
	#print("state : ", evt.state)
	#print("type : ", evt.type)

	#if evt.state.is_button_down(10) :
	#	print("Button 10 is down") 
	if evt.state.any_button_down:
		print("button down : ", evt)

	# Try to mess with buttons
	# print(evt.state.buttons)
	# can't access the buttons because exposing them is bit iffy



# TODO add keyboard or GUI controls
# - hide assembly and changing the order of assemblies

controller = AssemblyController(assembly, camera)

trigger = game.event_manager.createJoystickTrigger()
trigger.add_listener(controller.update_joystick)
game.event_manager.frame_trigger.add_listener(controller.progress)

# For testing and documenting buttons
#trigger.add_listener(joystick_print)

