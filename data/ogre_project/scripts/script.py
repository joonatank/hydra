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

# TODO these should be configured using options and created in global script
create_camera_controller()

def hide_ogre():
	if ogre.visible:
		ogre.hide()
	else : 
		ogre.show()

ogre = game.scene.getSceneNode("ogre")
trigger = game.event_manager.createKeyTrigger(KC.H)
trigger.add_listener(hide_ogre)

ogre.position = Vector3(0, 2.5, 0)

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
game.scene.shadows.max_distance = 50
game.scene.shadows.dir_light_extrusion_distance = 100

ground = create_ground(size=100)

print("Create Sphere")
game.mesh_manager.createSphere('sphere')
sphere_ent = game.scene.createEntity('sphere', 'sphere', True)
sphere_ent.material_name = 'finger_sphere/red'
sphere = game.scene.createSceneNode('sphere')
sphere.attachObject(sphere_ent)
sphere.position = Vector3(4, 2.5, 0)
sphere_ent.cast_shadows = True

athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "athene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale(0.05)

if game.scene.hasSceneNode("spot"):
	print("Hiding spot light")
	spot = game.scene.getLight("spot")
	spot.hide()

sun = create_sun()
sun.orientation = Quaternion(-0.900988, 0.43373, 0, 0)

# Create spotlight and hehkulamppu objects
spotti_n = game.scene.createSceneNode("spotti")
spotti = game.scene.createEntity("spotti", "spotlight.mesh", True)
spotti.material_name = "editor/spotlight_material"
spotti_n.position = Vector3(5, 4, 0)
spotti_n.attachObject(spotti)

lightpulp_n = game.scene.createSceneNode("hehkulamppu")
lightpulp = game.scene.createEntity("hehkulamppu", "light_pulp.mesh", True)
lightpulp.material_name = "editor/hehkulamppu_material"
lightpulp_n.position = Vector3(5, 4, -4)
lightpulp_n.attachObject(lightpulp)

plane = Plane(Vector3(0, 0, 1), 1)
m = buildReflectionMatrix(plane)

ogre.addChild(spotti_n)
ogre.addChild(lightpulp_n)

ogre2 = ogre.clone()
ogre2.translate(Vector3(0, 10, 0))

def printSpotti():
	Ts = spotti_n.world_transformation
	# Don't try to copy the object, it uses references by default
	iTs = spotti_n.world_transformation
	iTs.invert()
	# For some reason  inv(Ts)*Ts has better accuracy, but both of them
	# are pretty close to zero using this with arbitary transformations
	print('Ts = ', Ts, ' inv(Ts) = ', iTs, "Ts*inv(Ts) = ",
			Ts*iTs, "inv(Ts)*Ts = ", iTs*Ts)

text = game.scene.createMovableText('text', 'text')
text.font_name = "BlueHighway-22"
#text.font_name = "StarWars"
text.colour = ColourValue(0, 1.0, 0)
text.char_height = 0.4
text_n = game.scene.createSceneNode('text')
text_n.attachObject(text)
text_n.translate(Vector3(3, 2, 2))

def rotateSpotti(t):
	speed = Degree(10)
	angle = Radian(speed*Degree(float(t)))
	spotti_n.rotate(Quaternion(angle, Vector3(0, 1, 0)))

game.event_manager.frame_trigger.add_listener(rotateSpotti)

def printAnalog(val):
	text.caption = "analog value = " + str(int(val))

# Test vrpn analog client
analog = game.event_manager.create_analog_client("meh@localhost")
analog.n_sensors = 1
analog.get_sensor(0).add_listener(printAnalog)

# Start with paused and toggle state with space
# TODO this is pretty useless because all events except for physics
# are still processed and simulation time is advanced
# Specifically
# Camera movements work
# Spot is rotated at each frame
# Sky is changing
game.auto_start = False
trigger = game.event_manager.createKeyTrigger(KC.SPACE)
trigger.add_listener(toggle_pause)

def show_debug():
	#ogre.show_debug_display = True
	game.scene.show_axes(True)

"""
def timer_callback():
	print("Called from continuous timer")

def single_timer_callback():
	print("Called from single timer.")

# TODO
# Can not be created from callbacks because it messes the iterators
# TODO Counting the timers start before the scene has completely loaded
trigger = game.event_manager.createTimeTrigger()
trigger.interval = time(2, 0)
trigger.add_listener(timer_callback)


trigger = game.event_manager.createTimeTrigger()
trigger.interval = time(10, 0)
trigger.continuous = False
trigger.add_listener(single_timer_callback)
"""

ogre.translate(0, 1, 0)
trigger = game.event_manager.createKeyTrigger(KC.Y)
trigger.add_listener(show_debug)

# Testing joystick trigger
# evt is the event
# i is index of the component which caused the triggering
def joystick_print(evt, i):
	#print("joystick callback called : ", evt)
	#print("state : ", evt.state)
	#print("type : ", evt.type)

	if evt.state.is_button_down(10) :
		print("Button 10 is down") 

	# Try to mess with buttons
	# print(evt.state.buttons)
	# can't access the buttons because exposing them is bit iffy

trigger = game.event_manager.createJoystickTrigger()
trigger.add_listener(joystick_print)

# For testing the Razer Hydra
def razer_hydra_cb(evt) :
	print(evt)

#game.razer_hydra.add_listener(razer_hydra_cb)

