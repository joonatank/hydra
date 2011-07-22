# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

print( 'Getting Ogre SceneNode' )
# config.getSceneNode gets a reference to already created SceneNode
# For now it's not possible to create SceneNodes from python
# So use this function to get a SceneNode created from .scene file.
# All SceneNodes in .scene file are created and can be retrieved here.
# Tangent space lighting does not work on the ogre_ent

print( 'Getting Camera SceneNode' )
camera = game.scene.getSceneNode("CameraNode")
camera.position = Vector3(0, 3, 15)
createCameraMovements(camera, speed=10)

ogre = game.scene.getSceneNode("ogre")
addHideEvent(ogre, KC.H)
addMoveSelection(speed=3, angular_speed=Degree(60), reference=camera)
ogre.position = Vector3(0, 2.5, 0)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )
game.player.camera = "Omakamera"

game.createBackgroundSound("The_Dummy_Song.ogg")
addToggleMusicEvent(KC.M)

# Create ground plane
# TODO should create the mesh using MeshManager so the size can be assigned
# Create a large plane for shader testing
# This shows the usage of the new mesh manager
ground_length = 40;
ground_mesh = game.mesh_manager.createPlane("ground", ground_length, ground_length)
print(ground_mesh)
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

if game.scene.hasSceneNode("spot"):
	spot = game.scene.getLight("spot")
	spot.setSpotRange(Radian(1.5), Radian(1.7), 0.7)
	spot.attenuation = LightAttenuation(35, 0.9, 0.09, 0.01)
	spot_n = game.scene.getSceneNode("spot")
	spot_n.position = Vector3(0, 20, 0)
	# Test code for lights at a distance
	#spot_n.position = Vector3(0, 100, 0)
	#spot.attenuation = LightAttenuation(200, 0.9, 0.09, 0.01)
	game.scene.addToSelection(spot_n)

#game.scene.removeFromSelection(ogre)

#if game.scene.hasLight("spot"):
#	game.scene.getLight("spot").visible = False

if game.scene.hasSceneNode("Ambient_light"):
	ambient_light = game.scene.getSceneNode("Ambient_light")

""" We use the default spot for now
l = game.scene.createLight("test_light")
l.type = "spot"
l_node = game.scene.createSceneNode("test_light")
l_node.position = Vector3(5, 10, -10)
l_node.orientation = Quaternion(0, 0, 0.7071, 0.7071)
l_node.attachObject(l)
game.scene.addToSelection(l_node)
"""

# Create spotlight and hehkulamppu objects
spotti_n = game.scene.createSceneNode("spotti")
spotti = game.scene.createEntity("spotti", "spotlight.mesh")
spotti.material_name = "editor/spotlight_material"
spotti_n.position = Vector3(5, 4, 0)
spotti_n.attachObject(spotti)

lightpulp_n = game.scene.createSceneNode("hehkulamppu")
lightpulp = game.scene.createEntity("hehkulamppu", "light_pulp.mesh")
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
text.track_camera = False
text.colour = ColourValue(0, 1.0, 0)
text_n = game.scene.createSceneNode('text')
text_n.attachObject(text)
text_n.translate(Vector3(3, 2, 2))

eye_trigger_n = "glassesTrigger"
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
	action = SetTransformation.create()
	action.scene_node = ray_n

	eye_trigger = game.event_manager.getTrackerTrigger(eye_trigger_n)
	# TODO this will remove the head tracking action
	# TODO this will override the ray_n transformation
	#eye_trigger.action = action

