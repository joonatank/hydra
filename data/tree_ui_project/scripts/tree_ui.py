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

# TODO create camera since we don't have a camera node in the original file
# alternatively we could use Editor/Camera
camera = game.scene.getSceneNode("editor/perspective")

def reset_camera():
	camera.position = Vector3(1.6, -0.5, 0.5)
	camera.orientation = Quaternion(-0.7071, 0, -0.7071, 0)

reset_camera()

trigger = game.event_manager.createKeyTrigger(KC.X)
trigger.add_listener(reset_camera)

camera_controller = create_camera_controller()

trigger = game.event_manager.createKeyTrigger(KC.F4)
trigger.add_listener(camera_controller.toggle_disable)

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
# Small parameters so we get decent shadows and since we are not moving at all
game.scene.shadows.max_distance = 5
game.scene.shadows.dir_light_extrusion_distance = 10
# SkyX does not work with reloading the scene
#game.scene.sky.preset = "sunset" #"clear"

ground = create_ground(size=40)

game.scene.sky_dome = SkyDomeInfo("CloudySky")

# TODO should add a prob under the bike

sun = create_sun()
# What is up with the sun orientation though
sun.orientation = Quaternion(-0.900988, 0.43373, 0, 0)
sun.rotate(Degree(60), Vector3(0, 1, 0))

hidden_colour = ColourValue(1.0, 0.0, 0)
visible_colour = ColourValue(0.0, 1.0, 0)

# Table for the different options
#
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
	# Separate method for the moment, not really necessary though
	def create(self) :
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

	def add_to_selection(self) :
		game.scene.clearSelection()
		game.scene.addToSelection(self.node)


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
battery_grey = Model("battery_grey", "battery/grey", Vector3(400, 100, 150))

#3. Akku 3d, akku, termarimalli (lierio) 450 x 120 x 120 m
battery_round = Model("battery_round", "battery/round", Vector3(450, 120, 120))

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
options = [meters, bells, navigators, batteries, baskets, lights]

# Generate cubes with textures
# models is a list of all scene nodes in the same order as options
models = []
for i, opt in enumerate(options):
	for j, model in enumerate(opt):
		model.create()
		models.append(model)
		# just use a hard coded formula that works for these objects
		# in a real case we should calculate the max size and use it instead
		model.translate(0, 0.5 + j/3, i/2 -len(options)/8)


class TextContainer:
	def __init__(self, gui_node):
		self.gui = gui_node
		self.objects = []
		self.index = 0

	def toggle_gui(self) :
		print("toggle gui")
		if self.gui.visible:
			self.gui.hide()
			game.scene.clearSelection()
		else:
			self.gui.show()

	def move_text_up(self, name):
		print("TextContainer.move_text_up")

	def move_text_down(self, name):
		print("TextContainer.move_text_down")

	# Input the name of the label object
	def toggle_visible(self, name):
		print("TextContainer.toggle_visible")
		print("hiding object : ", name)
		# TODO this should retrieve object from the objects not from the Scene
		node = self.find_label(name)
		obj = node.objects[0]
		ho = self.find_object(name)
		assert(ho)
		if ho.visible:
			ho.hide()
			obj.colour = hidden_colour
		else:
			ho.show()
			obj.colour = visible_colour

	def unhide_all(self):
		for node in self.objects :
			obj = node.objects[0]
			assert(obj)
			obj.colour = visible_colour

			# we need to unhide the object not the text
			name = obj.caption
			ho = game.scene.getSceneNode(name)
			ho.show()

	# TODO implement
	def find_label(self, label_name) :
		for n in self.objects :
			if n.name == label_name :
				return n
		return None

	def find_object(self, label_name) :
		for n in self.objects :
			if n.name == label_name :
				obj = n.objects[0]
				return game.scene.getSceneNode(obj.caption)
		return None

	def add_label(self, name):
		obj_name = name + "_text"
		text = game.scene.createMovableText(obj_name, name)
		text.font_name = "BlueHighway-22"
		#text.font_name = "StarWars"
		text.colour = visible_colour
		text.char_height = 0.03
		text_n = game.scene.createSceneNode(obj_name)
		text_n.attachObject(text)
		text_n.translate(0.5, 0.2 + len(self.objects)/15, 0)
		text_n.rotate(Degree(90), Vector3(0, 1, 0))
		# TODO should be tied to the camera and hidable as a single object
		gui.addChild(text_n)
		self.objects.append(text_n)

	# Should not need these anymore since we should be using get by name
	# and a name list where the user can select
	# or we could even tie the name list with extra information using a
	# SceneNode directly might be bit complex with the python interface though
	def next_selection(self) :
		self.index = self.index + 1
		if self.index >= len(self.objects): self.index = 0
		# does not work since we switched to text boxes
		game.scene.clearSelection()
		game.scene.addToSelection(self.objects[self.index])

	def prev_selection(self) :
		self.index = self.index - 1
		if self.index < 0 : self.index = len(self.objects) - 1
		# does not work since we switched to text boxes
		#self.models[self.index].add_to_selection()
		game.scene.clearSelection()
		game.scene.addToSelection(self.objects[self.index])


gui = game.scene.createSceneNode("tree_gui")
camera.addChild(gui)
texts = TextContainer(gui)
for i, model in enumerate(models):
	texts.add_label(model.name)

# Not using keyboard selection for the moment (only wand selection)
# For testing enabled
#
# Might need these for the start of the simulation for showing all the models
# the other option would be to have two lists of them
# which I rather not implement now
trigger = game.event_manager.createKeyTrigger(KC.TAB)
trigger.add_listener(texts.prev_selection)

trigger = game.event_manager.createKeyTrigger(KC.TAB, KEY_MOD.SHIFT)
trigger.add_listener(texts.next_selection)

# Toggle for hiding and showing the selected
def hide_selected():
	print("hide selected")
	for i in range(len(game.scene.selection)):
		# we need to hide the objects with name of the text
		#print("hiding : ", obj.name)
		node = game.scene.selection[i]

		texts.toggle_visible(node.name)

trigger = game.event_manager.createKeyTrigger(KC.C)
trigger.add_listener(texts.toggle_gui)

trigger = game.event_manager.createKeyTrigger(KC.H)
trigger.add_listener(hide_selected)

trigger = game.event_manager.createKeyTrigger(KC.H, KEY_MOD.META)
trigger.add_listener(texts.unhide_all)

#trigger = game.event_manager.createKeyTrigger(KC.U)
#trigger.add_listener(move_selection_up)

#trigger = game.event_manager.createKeyTrigger(KC.J)
#trigger.add_listener(move_selection_down)

# TODO add selection controller using input objects
# gotten from the current selection name
# um this has problems like we should use a own class for it
# well not necessarily if we assume that all selected objects have a child
# of MovableText type we can use the name from there
# or we can use python string utils to cypher the name of the object using a
# common tag

# Add text stuff here using the models array (model.name variable)
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

# TODO fix the joystick stuff
#joy_selection_controller = JoystickSelectionMover(camera)

#trigger = game.event_manager.createJoystickTrigger()
# Disabled it's only for testing
#trigger.add_listener(joystick_print)
# Disabled selecting with the joystick because we are using wand for it
# enable if you need it for testing
#trigger.add_listener(joystick_select)
# Change model of selected part with the directional pad
#trigger.add_kistener(joystick_change_model)
#trigger.add_listener(joy_selection_controller.joystick_updated)

#game.event_manager.frame_trigger.add_listener(joy_selection_controller.progress)

