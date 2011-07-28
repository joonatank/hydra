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

createCameraMovements(camera, speed=10)

game.player.camera = "camera"

addMoveSelection(speed=3, angular_speed=Degree(60), reference=camera)

ogre_ent = game.scene.createEntity("ogre", "ogre.mesh")
ogre = game.scene.createSceneNode("ogre")
ogre.attachObject(ogre_ent)
ogre.position = Vector3(0, 2.5, 0)

game.scene.sky = SkyDomeInfo("CloudySky")

game.scene.addToSelection(ogre)

# Create ground plane
ground_length = 40;
game.mesh_manager.createPlane("ground", ground_length, ground_length)
ground_ent = game.scene.createEntity('ground', "ground", True)
ground = game.scene.createSceneNode('ground')
ground.attachObject(ground_ent)
# Shader material, with shadows
ground_ent.material_name = 'ground/bump_mapped/shadows'
ground_ent.cast_shadows = False

sphere_ent = game.scene.createEntity('sphere', PF.SPHERE)
sphere_ent.material_name = 'finger_sphere/red'
sphere = game.scene.createSceneNode('sphere')
sphere.attachObject(sphere_ent)
sphere.position = Vector3(4, 2.5, 0)
sphere.scale = sphere.scale*0.003
sphere_ent.cast_shadows = True

athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "athene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale = Vector3(1,1,1)*0.05;

game.scene.shadows.enable()

l = game.scene.createLight("spot")
l.type = "spot"
l_node = game.scene.createSceneNode("spot")
l_node.position = Vector3(0, 30, 0)
l_node.orientation = Quaternion(0, 0, 0.7071, 0.7071)
l_node.attachObject(l)

eye_trigger_n = "eyeTrigger"
if(game.event_manager.hasTrackerTrigger(eye_trigger_n)):
	print("Adding eye tracker action")
	ray = game.scene.createRayObject("ray", "finger_sphere/red")
	ray.direction = Vector3(0, 0, 1)
	ray.sphere_radius = 0.2
	#ray.position = Vector3(0, 0, 10)
	#ray.length = 20
	ray.draw_collision_sphere = True
	ray.collision_detection = True
	ray_n = game.scene.createSceneNode("ray")
	ray_n.attachObject(ray)
	# Using the ogre object for easier visualization
	ogre.addChild(ray_n)
	#camera.addChild(ray_n)
	# Hard coded transform to show the ray object
	ray_n.transformation = Transform(Vector3(0, 0.5, 0), Quaternion(0, 0, 0, 1))
	eye_trigger = game.event_manager.getTrackerTrigger(eye_trigger_n)

	# Create the current transformation showing
	action = SetTransformation.create()
	action.scene_node = ray_n

	eye_trigger.action.add_action(action)

	# Create the recorder
	action = RecordRayAction.create()
	action.scene = game.scene
	eye_trigger.action.add_action(action)

