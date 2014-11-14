# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

camera = game.scene.createSceneNode("camera")
camera.position = Vector3(0, 3, 15)
cam = game.scene.createCamera("camera")
camera.attachObject(cam)

create_camera_controller()

game.player.camera = "camera"

game.scene.sky_dome = SkyDomeInfo("CloudySky")

# Create ground plane
ground_length = 20;
game.mesh_manager.createPlane("ground", ground_length, ground_length)
ground_ent = game.scene.createEntity('ground', "ground", True)
ground = game.scene.createSceneNode('ground')
ground.attachObject(ground_ent)
ground_ent.material_name = 'ground/bump_mapped/shadows'
ground_ent.cast_shadows = False

# Create walls
rot = Quaternion(0.7071, 0.7071, 0, 0)*Quaternion(0.7071, 0, 0.7071, 0)

wall_ent = game.scene.createEntity('wall_left', "ground", True)
wall_ent.material_name = 'ground/bump_mapped/shadows'
wall_ent.cast_shadows = False
wall_left = game.scene.createSceneNode('wall_left')
wall_left.attachObject(wall_ent)
wall_left.rotate(Quaternion(0.7071, 0, 0.7071, 0)*rot)
wall_left.translate(Vector3(0, -10, 0))

wall_ent = game.scene.createEntity('wall_front', "ground", True)
wall_ent.material_name = 'ground/bump_mapped/shadows'
wall_ent.cast_shadows = False
wall_front = game.scene.createSceneNode('wall_front')
wall_front.attachObject(wall_ent)
wall_front.translate(Vector3(0, 0, -10))
wall_front.rotate(rot)

wall_ent = game.scene.createEntity('wall_right', "ground", True)
wall_ent.material_name = 'ground/bump_mapped/shadows'
wall_ent.cast_shadows = False
wall_right = game.scene.createSceneNode('wall_right')
wall_right.attachObject(wall_ent)
#wall_right.rotate(Quaternion(0.7071, 0.7071, 0, 0)*Quaternion(0.7071, 0, 0.7071, 0))
wall_right.rotate(Quaternion(-0.7071, 0, 0.7071, 0)*rot)
wall_right.translate(Vector3(0, -10, 0))

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
athene.scale(0.05);

game.scene.shadows.enable()

l = game.scene.createLight("spot")
l.type = "spot"
l_node = game.scene.createSceneNode("spot")
l_node.position = Vector3(0, 30, 0)
l_node.orientation = Quaternion(0, 0, 0.7071, 0.7071)
l_node.attachObject(l)

# Add code that enables and configures EyeTracker
# Obviously needs EyeTracker interface to be exposed first
# Not auto starting at the moment because we want to calibrate it
# FIXME
# IF we don't auto start it the ray goes missing, still need to test it with the
# debugging variables on though.
# If we auto start we get a weird 45 degree downward angle in the eye ray
game.eye_tracker.start()
# Seems to be working properly with fake gaze tracker and static head tracking
# TODO
# With varying head tracking
# Doesn't seem to work, for some reason the eye flips over when head tracking
# is enabled. It doesn't however seem to have a noticable pattern for flipping.
# 
# TODO
# Test with real eye tracking (without head tracking)
# Test with real head tracking (without eye tracking)
# Test with both eye and head tracking
#game.eye_tracker.debug = True
#game.eye_tracker.head_disabled = True

# Debug functions
def toggle_eye_debug() :
	game.eye_tracker.debug = not game.eye_tracker.debug
def toggle_eye_head() :
	game.eye_tracker.head_disabled = not game.eye_tracker.head_disabled

# Add key event to enable/disable real time eye tracking
def toggle_eye_tracker() :
	if game.eye_tracker.started :
		game.eye_tracker.stop()
	else :
		game.eye_tracker.start()

trigger = game.event_manager.createKeyTrigger(KC.SPACE)
trigger.add_listener(toggle_eye_tracker)

trigger = game.event_manager.createKeyTrigger(KC.U)
trigger.add_listener(toggle_eye_debug)

trigger = game.event_manager.createKeyTrigger(KC.I)
trigger.add_listener(toggle_eye_head)

# Add key event to show/hide previous recording

""" These are not used anymore
eye_trigger_n = "eyeTrigger"
if(game.event_manager.hasTrackerTrigger(eye_trigger_n)):
	print("Adding eye tracker action")
	ray = game.scene.createRayObject("ray", "collision_sphere/red")
	#ray = game.scene.createRayObject("ray", "collision_sphere/transparent/red")
	ray.direction = Vector3(0, 0, -1)
	ray.sphere_radius = 0.2
	#ray.position = Vector3(0, 0, 10)
	#ray.length = 20
	# Doesn't affect performance that much if the spheres are on or off
	ray.draw_collision_sphere = True
	ray.collision_detection = True
	#ray.draw_ray = False
	ray_n = game.scene.createSceneNode("ray")
	ray_n.attachObject(ray)

	# Using the ogre object for easier visualization
	#ogre.addChild(ray_n)
	#camera.addChild(ray_n)
	# Be careful when rotating the ray node, this will freak havok for the
	# visualisation because all the rotations change the coordinate frame
	# the eye is expressed in the Recorded data.
	# The final version uses a static camera with a rotation around
	# y-axis at maximum.
	# This is acceptable because same applies to the head tracking
	# user experiences.
	#ray_n.rotate(Quaternion(0.7071, 0, 0.7071, 0))
	#ray_n.position = Vector3(-0.5, 0, 0.5)

	# Real time updater
	# Hard coded transform to show the ray object
	#ray_n.transformation = Transform(Vector3(0, 1.5, 0))
	eye_trigger = game.event_manager.getTrackerTrigger(eye_trigger_n)

	# Create the current transformation showing
	action = SetTransformation.create()
	action.scene_node = ray_n

	# This will reset the ray_n transformation to default if the tracker
	# is not present
	#eye_trigger.action.add_action(action)

	# Create recording visialisation
	# Recording file needs to be in the project/global resources
	# for it to be loaded.
	#rec = game.loadRecording("tracker_recording_eye.log")
	rec = game.loadRecording("tracker_recording_eye_2.log")
	ray.recording = rec
	# TODO this does not update the ray correctly
	# If enabled here user needs to call ray.update() from command line
	ray.show_recording = True
"""

