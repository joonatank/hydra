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

camera = game.scene.getSceneNode("Camera")

class CameraChanger :
	def __init__(self, camera_node) :
		self.view_index = 0
		self.views = []
		self.camera_node = camera_node
		pass

	def next_view(self) :
		self.view_index = self.view_index + 1
		if len(self.views) == self.view_index : self.view_index = 0
		self.camera_node.transformation = self.views[self.view_index]

	def prev_view(self) :
		self.view_index = self.view_index - 1
		if self.view_index < 0 : self.view_index = len(self.views) - 1
		self.camera_node.transformation = self.views[self.view_index]

	def add_view(self, t) :
		self.views.append(t)
		# initialise
		if len(self.views) == 1:
			self.camera_node.transformation = self.views[0]

	# set an unique special view outside of the normal list
	def set_special_view(self, t) :
		self.special_view = t

	# use the special view
	def special_view(self):
		self.camera_node.transformation = self.special_view 

camera_changer = CameraChanger(camera)

# side view
camera_changer.add_view(Transform(Vector3(1, -1, 0), Quaternion(-0.7071, 0, -0.7071, 0)))
# satula
camera_changer.add_view(Transform(Vector3(0, -0.28, 0.8), Quaternion.identity))
# back, not needed
#camera_changer.add_view(Transform(Vector3(0, -1, 2), Quaternion.identity))

# TODO add switching of camera view
trigger = game.event_manager.createKeyTrigger(KC.Z)
trigger.addListener(camera_changer.next_view)
trigger = game.event_manager.createKeyTrigger(KC.X)
trigger.addListener(camera_changer.prev_view)


game.player.camera = "Camera"
# TODO disable camera controller
camera_controller = createCameraMovements(speed=1)
camera_controller.disable()

trigger = game.event_manager.createKeyTrigger(KC.F4)
trigger.addListener(camera_controller.toggle_disable)

# TODO we need to use selection based if we are using Oculus or not
#camera_controller.head_direction = True
# for the clouds we need to increase far clipping
#cam = game.scene.getCamera("Camera")
#cam.far_clip = 5e5

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
# Small parameters so we get decent shadows and since we are not moving at all
game.scene.shadows.max_distance = 5
game.scene.shadows.dir_light_extrusion_distance = 10
# SkyX does not work with reloading the scene
#game.scene.sky.preset = "sunset" #"clear"

ground = create_ground(size=40)
ground.translate(0, -0.5, 0)

game.scene.sky_dome = SkyDomeInfo("CloudySky")

# TODO should add a prob under the bike

sun = create_sun()
sun.orientation = Quaternion(-0.900988, 0.43373, 0, 0)

bike = game.scene.getSceneNode("polkupyora_locomotif")
assert(bike)
bike.rotate(Degree(90), Vector3(-1, 0, 0))


# Table for the different options
#
#### Naming #####
# model is the specific part model like "fancy bell"
# part is the general part like "bell"
#################
#
# Structure
# [array of options]
# Option {name, texture, size}
# If later we do need common names for all the sub parts we can add that then

class Model :

	# @param name
	# @param material (name)
	# @param size in millimeters
	def __init__(self, name = "default", material = "default", size = Vector3(100, 100, 100)) :
		self.size = size
		self.name = name
		self.material = material

	# Create the Scene node and model
	# @param part Part that this model represents
	def create(self, part) :
		size = self.size/1000.0
		print("Creating ", i, "th model", self.name, " with size ", size, "m")
		# swap x and z in size since we have some texture problems
		x = size.z
		size.z = size.x
		size.x = x
		cube = game.mesh_manager.createCube(self.name, size)
		self.node = game.scene.createSceneNode(self.name)
		ent = game.scene.createEntity(self.name, self.name, True)
		self.node.attachObject(ent)
		# Adding child doesn't reset the transform
		part.node.addChild(self.node)
		self.node.transformation = Transform()
		ent.material_name = self.material

	def hide(self) :
		self.node.hide()

	def show(self) :
		self.node.show()

	def translate(self, x, y, z) :
		self.node.translate(x, y, z)

	def reset_transform(self):
		self.node.transformation = Transform()

class Part :
	def __init__(self, name) :
		self.models = []
		self.node = game.scene.createSceneNode(name)
		self.index = 0
		self.name = name

	def add_model(self, model) :
		self.models.append(model)
		# If it's the first model show it if not hide it
		if len(self.models) == 1:
			model.show()
		else :
			model.hide()

	def next_model(self) :
		if len(self.models) == 0: return
		self.models[self.index].hide()
		self.index = self.index + 1
		if self.index >= len(self.models) :
			self.index = 0
		self.models[self.index].show()

	def prev_model(self) :
		if len(self.models) == 0: return
		self.models[self.index].hide()
		self.index = self.index - 1
		if self.index < 0 :
			self.index = len(self.models) -1
		self.models[self.index].show()

	def translate(self, vec) :
		self.node.translate(vec)

	def transform(self, t) :
		self.node.transform(t)

	def add_to_selection(self) :
		game.scene.clearSelection()
		game.scene.addToSelection(self.node)

	# Shows all models in a column
	def show_all_models(self) :
		for i, model in enumerate(self.models) :
			model.show()
			# TODO translate the model in y axis
			# shit need index for this
			model.translate(0, 0.3*i-0.7, 0)

	def hide_extra_models(self) :
		for model in self.models :
			model.hide()
			# TODO move back to y axis
			model.reset_transform()
		# show the only model
		self.models[self.index].show()

	# Main purpose for this function is to easily show all the models
	# in the start of the simulation
	# we can hide the ones not interest to us
	def hide(self) :
		# Hiding all the models here because this can be called after show all
		# models and show next model will anyway show the next model
		#
		# Not a good idea since we need to also move the models
		#
		# Ah this cascades and hides all the models
		# there is no way to preserve the previous state of the model
		self.node.hide()


	def show(self) :
		#assert(self.index < len(self.models))
		#self.models[self.index].show()
		# Ah this cascades and shows all the models
		# there is no way to preserve the previous state of the model
		self.node.show()

	def has_model(self, model_node) :
		print("Trying to find model ", model_node.name)
		for i in self.models:
			if i.node.name == model_node.name :
				return True
		return False

	def create_collision_model(self) :
		print("Part:create_collision_model")
		
		# TODO Get the largest box containing all the objects
		if len(self.models) > 0 :
			model = self.models[0]
			ms = game.physics_world.createMotionState(part.node.world_transformation, part.node)
			shape = BoxShape.create(model.size/1000.0)
			body = game.physics_world.createRigidBody("rb_"+self.name, 1.0, ms, shape, Vector3(1, 1, 1))
			body.kinematic = True

# Need physics for ray casting
game.enablePhysics(True)
#CUSTOM SOLVER PARAMETERS:
solverparams = PhysicsSolverParameters()
#Joint normal error reduction parameter (slow(0.0) - fast(1.0)):
solverparams.erp = 0.8
#Contact error reduction parameter (slow(0.0) - fast(1.0)):
solverparams.erp2 = 0.8
#Constraint force mixing parameter, when set to 0.0 constraints will be hard:
solverparams.global_cfm = 0.0
#On collisions how much the momentum is conserved:
#(totally_inelastic(0.0) - totally_elastic(1.0))
solverparams.restitution = 0.0
#I have no idea of this, so setting it to default:
solverparams.max_error_reduction = game.physics_world.solver_parameters.max_error_reduction
#Internal time step (1/fps)
solverparams.internal_time_step = 1.0 / 120.0
#No idea what substep, has it something to do with interpolation, well higher
#value should be more precise but wastes much more computing power?
solverparams.max_sub_steps = 20
#Now we set the solver parameters to current physics world:
game.physics_world.solver_parameters = solverparams


# 1. Mittarit
# 1a lataus- ja nopeusmittari, valintapainikkeet
# 100 x 60 x 15 mm
regular_meter = Model("regular", "meter/regular", Vector3(100, 60, 15))
#regular_meter = Model("regular", "meter/regular", Vector3(15, 60, 100))
# 1b pyorea mittari
# 50 x 50 x 20 mm
# TODO this meter has some texturing and size problems not sure what's going on
# in there
# actually for size I know what's going on in there
# the extra holding in not measured in the 50mm so it needs to be removed from
# the texture or added to the size
# transparency seems to be fucked though
# actually I used wrong texture nevermind
round_meter= Model("round", "meter/round", Vector3(50, 50, 20))
#round_meter= Model("round", "meter/round", Vector3(20, 50, 50))
# 1c harmaa latausmittari lisukkein
# 70 x 45 x 10 mm
grey_meter = Model("grey", "meter/grey", Vector3(70, 50, 10))
#grey_meter = Model("grey", "meter/grey", Vector3(10, 50, 70))
# 1d monimittari
# 77 x 35 x 10 mm
multimeter = Model("multimeter", "meter/multi", Vector3(77, 45, 10))
#multimeter = Model("multimeter", "meter/multi", Vector3(10, 45, 77))
meters =[regular_meter, round_meter, grey_meter, multimeter]


# 2. Soittokello
# 2a punainen kello
# 56 x 62 x 56 mm
red_bell = Model("red", "bell/red", Vector3(56, 62, 56))

# 2b perinteinen kello
# 55 x 78 x 55 mm
regular_bell = Model("regular bell", "bell/regular", Vector3(55, 78, 55))

# 2c painokello
# 50 x 50 x 50 mm
push_bell = Model("push bell", "bell/push", Vector3(50, 50, 50))

# 2d hassukello
# 66 x 80 x 66 mm
fancy_bell = Model("fancy bell", "bell/fancy", Vector3(66, 80, 66))

bells = [red_bell, regular_bell, push_bell, fancy_bell]

# 5. Navigaattori (optional)

# 5a 
# 55 x 93 x 25 mm
bike_nav = Model("bike navigator", "nav/bike", Vector3(55, 93, 25))
#bike_nav = Model("bike navigator", "nav/bike", Vector3(25, 93, 55))

# 5b vaakasuunt navigaattori
# 120 x 74 x 15 mm
horizontal_nav = Model("horizontal_nav", "nav/horizontal", Vector3(120, 74, 66))

# 5c vaakasuunt navigaattori
# 140 x 80 x 25 mm
horizontal_nav2 = Model("horizontal_nav2", "nav/horizontal2", Vector3(140, 80, 25))

# 5d 
# 110 x 85 x 20 mm
motor_bike_nav = Model("motor_bike_nav ", "nav/motor_bike", Vector3(110, 85, 20))

navigators = [bike_nav, horizontal_nav, horizontal_nav2, motor_bike_nav]

# This is essential doesn't have a model yet though
# 3. Akku
# Battery

#3. Akku 3a, suorakaideakku 400 x 50 x 200 mm
battery = Model("battery", "battery/basic", Vector3(400, 50, 200))

#3. Akku 3b, akku, harmaa 150 x 100 x 400 mm
battery_grey = Model("battery_grey", "battery/grey", Vector3(150, 100, 400))

#3. Akku 3c akku, nelio 250 x 100 x 250 mm
#battery_square = Model("battery_square", "battery/square", Vector3(250, 100, 250))

#3. Akku 3d, akku, termarimalli (lierio) 450 x 120 x 120 m
battery_round = Model("battery_round", "battery/round", Vector3(120, 120, 450))

batteries = [battery, battery_grey, battery_round]


# Optional, not yet models though
# 4. Tavarakori
# Baskets

#4a kori isompana 350 x 300 x 200
basket_large = Model("basket_large", "basket/large", Vector3(350, 300, 200))

#4c kori 350 x 230 x 200
basket_small = Model("basket_small", "basket/small", Vector3(350, 230, 200))

basket_wicker = Model("basket_wicker", "basket/wicker", Vector3(350, 230, 200))

baskets = [basket_large, basket_small, basket_wicker]

# Saddle bags (different from basket)
#4d satulalaukku 300 x 250 x 40
saddle_bags = Model("saddle_bags", "saddle_bags", Vector3(300, 250, 40))

bags = [saddle_bags]

# Lights
#6 valot 6a, metalliputki putken pituus: 250, halkaisija: 50 mm
# Real measurements with the holder are double height
light_tube = Model("light_tube", "light/tube", Vector3(250, 100, 50))

#6b, valo, musta kuvakoko: 180 x 180 mm, lampun syvyys 80
light_black = Model("light_black", "light/black", Vector3(180, 180, 80))

#6c, valo, vaalea putki: 160, halkaisija 80
light_white = Model("light_white", "light/white", Vector3(160, 160, 80))

#6d, valo, retro pituus: 160, halkaisija 120
light_retro = Model("light_retro", "light/retro", Vector3(160, 160, 120))

lights = [light_tube, light_black, light_white, light_retro]

# Options list contains all the different options as arrays
# so it's an array of array of arrays, bit complicated
# Might be preferable to create a small class to contain the option itself
options = [meters, bells, navigators, batteries, baskets, bags, lights]

#print(options)

# Generate cubes with textures
# parts is a list of all scene nodes in the same order as options
# it should be list of master objects though
parts = []
for i, opt in enumerate(options):
	# TODO create the part
	# For now a temporary name
	name = "Part-" + str(i)
	part = Part(name)
	# TODO need to reorient this so that the bike is along x axis not z so we don't need to rotate the parts
	#part_node.rotate(Degree(90), Vector3(0, 1, 0))
	parts.append(part)
	# TODO need to be rotated to the left wall
	#part.translate(Vector3(-0.5, 1, i/2 - len(options)/4))
	# so that's something like
	# [i/2 -len(options)/4, 1, -0.5]
	part.translate(Vector3(i/4 - len(options)/8, 0.6, 1))
	# and they need to be ralative to the camera because we want them to stay on the left wall when we
	# change the camera view (they would otherwise be on the back where we have no wall)
	#camera.addChild(part.node)

	# Actually we need to generate all of them attach them to separate scene nodes and hide the ones not used
	# use a master node for moving and the child nodes for hiding.
	# Testing uvs
	for model in opt :
		model.create(part)
		part.add_model(model)
	
	# Finalise
	part.create_collision_model()
	#opt[0].show()


class PartSelection :
	def __init__(self, parts):
		self.index = 0
		self.parts = parts

	def next_selection(self) :
		self.index = self.index + 1
		if self.index >= len(parts): self.index = 0
		self.parts[self.index].add_to_selection()

	def prev_selection(self) :
		self.index = self.index - 1
		if self.index < 0 : self.index = len(self.parts) - 1
		self.parts[self.index].add_to_selection()

	def next_model(self) :
		# Find the current selected model
		print("Trying to get next model")
		if len(game.scene.selection) == 0:
			print("No selection")
			return
	
		part = self.find_part(game.scene.selection[0])
		
		if part :
			print("Found the part ", part.name)
			part.next_model()
		else :
			print("No part found")

	def find_part(self, part_node):
		for i in self.parts :
			if i.node.name == part_node.name:
				return i
		return None

	def prev_model(self) :
		part = self.get_current_part()
		
		if part :
			print("Found the part ", part.name)
			part.prev_model()
		else :
			print("No part found")
		
	def get_current_part(self) :
		# Find the current selected model
		print("Trying to get previous model")
		if len(game.scene.selection) == 0:
			print("No selection")
			return None
	
		return self.find_part(game.scene.selection[0])
	
	# Rotate current part 90 degrees along y-axis
	def rotate(self):
		part = self.get_current_part()
		
		if part :
			part.rotate(90)
		else :
			print("No part selected")

selection = PartSelection(parts)

# Not using keyboard selection for the moment (only wand selection)
# For testing enabled
#
# Might need these for the start of the simulation for showing all the models
# the other option would be to have two lists of them
# which I rather not implement now
trigger = game.event_manager.createKeyTrigger(KC.TAB)
trigger.addListener(selection.prev_selection)

trigger = game.event_manager.createKeyTrigger(KC.TAB, KEY_MOD.SHIFT)
trigger.addListener(selection.next_selection)

trigger = game.event_manager.createKeyTrigger(KC.C)
trigger.addListener(game.scene.clearSelection)

# For the startup we need to show all the models
class PartInspector :
	def __init__(self, part_list) :
		self.parts = part_list
		self.index = 0

	# show the next part with all it's models and nothing else
	def show_next_part(self) :
		if self.index == len(self.parts) : self.index = 0

		# TODO this will leave the previous part in a weird state
		# does it matter though as long as we don't roll the list
		# multiple times and use clear after we are done

		for part in self.parts:
			#part.hide_extra_models()
			part.hide()

		parts[self.index].show_all_models()
		#parts[self.index].show()

		# increase index at the end so we always start from 0
		self.index = self.index + 1

	# show the prev part with all it's models and nothing else
	# not implemented for the moment
	def show_prev_part(self) :
		pass

	# return to the original state
	def clear(self) :
		for part in parts :
			part.hide_extra_models()

inspector = PartInspector(parts)

trigger = game.event_manager.createKeyTrigger(KC.B)
trigger.addListener(inspector.show_next_part)

trigger = game.event_manager.createKeyTrigger(KC.H)
trigger.addListener(inspector.clear)


# TODO selection should have only moving and rotation 90 deg around y
addMoveSelection(reference = camera)

# TODO add changing the selection from dir pad buttons in a joystick

# TODO add move select that works with joystick
class JoystickSelectionMover :
	def __init__(self, reference = None) :
		self.velocity = 0.01
		self.mov_dir = Vector3(0, 0, 0)
		self.reference = reference

	def joystick_updated(self, evt, i):
		if evt.state.is_button_down(10) :
			x = evt.state.axes[3]
			z = evt.state.axes[2]
			# Splitfish hack
			if(abs(z) < 0.025) : z = 0
			self.mov_dir = Vector3(x, 0, z)
		else :
			# reset the mov dir if button is released to be sure
			self.mov_dir = Vector3(0, 0, 0)
	
	def progress(self, t) :
		for i in  range(len(game.scene.selection)) :
			game.scene.selection[i].translate(self.velocity*self.mov_dir, self.reference)


def joystick_select(evt, i):
	if evt.type == JOYSTICK_EVENT_TYPE.BUTTON_PRESSED:
		if evt.state.is_button_down(5) :
			selection.prev_selection()
		if evt.state.is_button_down(7) :
			selection.next_selection()

def joystick_change_model(evt, i):
	if evt.type == JOYSTICK_EVENT_TYPE.BUTTON_PRESSED:
		if evt.state.is_button_down(4) :
			selection.next_model()
		if evt.state.is_button_down(6) :
			selection.prev_model()

def joystick_print(evt, i):
	#print("joystick callback called : ", evt)
	#print("state : ", evt.state)
	#print("type : "evt.type)

	if evt.state.is_button_down(10) :
		print("Button 10 is down") 
	if evt.state.is_button_down(11) :
		print("Button 11 is down") 
	if evt.state.is_button_down(8) :
		print("Button 8 is down") 
	if evt.state.is_button_down(17) :
		print("Button 17 is down") 
	if evt.state.is_button_down(4) :
		print("Button 4 is down") 
	if evt.state.is_button_down(5) :
		print("Button 5 is down") 
	if evt.state.is_button_down(6) :
		print("Button 6 is down") 
	if evt.state.is_button_down(7) :
		print("Button 7 is down") 

	# Try to mess with buttons
	# print(evt.state.buttons)
	# can't access the buttons because exposing them is bit iffy
joy_selection_controller = JoystickSelectionMover(camera)

trigger = game.event_manager.createJoystickTrigger()
# Disabled it's only for testing
#trigger.addListener(joystick_print)
# Disabled selecting with the joystick because we are using wand for it
# enable if you need it for testing
#trigger.addListener(joystick_select)
# Change model of selected part with the directional pad
trigger.addListener(joystick_change_model)
#trigger.addListener(joy_selection_controller.joystick_updated)

game.event_manager.frame_trigger.addListener(joy_selection_controller.progress)
# Add switching between selected sub parts (model)
trigger = game.event_manager.createKeyTrigger(KC.N)
trigger.addListener(selection.prev_model)

trigger = game.event_manager.createKeyTrigger(KC.M)
trigger.addListener(selection.next_model)

#Of these two functions enable only either not both.
#This will create the ray dragger and "wand":
dragger, joy_handler = createWandRayController("wand", "wand", False)
#This will create the ray dragger and debug controller eg. keyboard controls for moving ray:
#dragger, debug_controller = createWandRayController("wand", "wand", True)

