# -*- coding: utf-8 -*-

camera = game.scene.createSceneNode("camera")
camera.position = Vector3(0, 3, 15)
cam = game.scene.createCamera("camera")
camera.attachObject(cam)
createCameraMovements(camera, speed=10)

game.player.camera = "camera"

addMoveSelection(speed=3, angular_speed=Degree(60))

#ogre = game.scene.getSceneNode("ogre")
#ogre.position = Vector3(0, 2.5, 0)

# Create ground plane
# Create a large plane for shader testing
# This shows the usage of the new mesh manager
ground_length = 40;
ground_mesh = game.mesh_manager.createPlane("ground", ground_length, ground_length)
print(ground_mesh)
ground_ent = game.scene.createEntity('ground', "ground", True)
ground = game.scene.createSceneNode('ground')
ground.attachObject(ground_ent)
# Shader material with shadows
#ground_ent.material_name = 'ground/bump_mapped/shadows'
ground_ent.material_name = 'ground/flat/shadows'
ground_ent.cast_shadows = False

wall_ent = game.scene.createEntity('wall', "ground", True)
wall = game.scene.createSceneNode("wall")
wall.attachObject(wall_ent)
wall_ent.material_name = 'ground/flat/shadows'
wall_ent.cast_shadows = False
wall.orientation = Quaternion(0.7071, 0.7071, 0, 0)
wall.position = Vector3(0, 20, -20)

# Can't use the new mesh manager, there are some attributes in the mesh file
# that make it impossible
ogre_ent = game.scene.createEntity("ogre", "ogre.mesh", False)
ogre = game.scene.createSceneNode("ogre")
ogre.attachObject(ogre_ent)
ogre.position = Vector3(3, 2.5, -5)
#athene.scale = Vector3(1,1,1)*0.05;


athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "athene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale = Vector3(1,1,1)*0.05;

athene2 = athene.clone();
athene2.position = Vector3(3, 4, 5)

game.scene.shadows.enable()

# Test transparency
# a semi-transparent glass surface for testing transparency colour
game.mesh_manager.createPlane("glass", 4, 5, Vector3.unit_z, 2, 5)
glass_ent = game.scene.createEntity('glass', "glass", True)
glass = game.scene.createSceneNode('glass')
# Shader material that has both ambient and diffuse alpha values but no texture
glass_ent.material_name = 'glass/blue'
glass_ent.cast_shadows = False
glass.attachObject(glass_ent)
# Straight in front of the camera
glass.position = Vector3(0, 2.5, 10)

# a textured semi-transparent surface
game.mesh_manager.createPlane("tree_plane", 4, 5, Vector3.unit_z, 2, 5)
tree_ent = game.scene.createEntity('ash', "tree_plane", True)
tree = game.scene.createSceneNode('ash')
tree_ent.material_name = 'ash_tree'
# TODO transparent objects don't cast shadows, needs material script updates
#tree_ent.cast_shadows = False
tree.attachObject(tree_ent)
tree.position = Vector3(0, 2.5, -10)

# bump mapping comparison
# with bump mapping without texturing
# no bump mapping, with texturing
# bump mapping and texturing
kontti_ent = game.scene.createEntity('kontti', "Kontti_12_Red.mesh", False)
kontti_ent.material_name = 'kontti_bump'
kontti = game.scene.createSceneNode('kontti')
kontti.attachObject(kontti_ent)
kontti.position = Vector3(-15, 1.2, 0)
kontti.orientation = Quaternion(0.7071, 0, 0.7071, 0)

kontti_ent = game.scene.createEntity('kontti2', "Kontti_12_Red.mesh", False)
kontti_ent.material_name = 'kontti_tex'
kontti = game.scene.createSceneNode('kontti2')
kontti.attachObject(kontti_ent)
kontti.position = Vector3(-15, 3.6, 0)
kontti.orientation = Quaternion(0.7071, 0, 0.7071, 0)

kontti_ent = game.scene.createEntity('kontti3', "Kontti_12_Red.mesh", False)
kontti_ent.material_name = 'kontti_both'
kontti = game.scene.createSceneNode('kontti3')
kontti.attachObject(kontti_ent)
kontti.position = Vector3(-15, 6.0, 0)
kontti.orientation = Quaternion(0.7071, 0, 0.7071, 0)

hytti_ent = game.scene.createEntity('hytti', "hytti.mesh", False)
hytti = game.scene.createSceneNode('hytti')
hytti.attachObject(hytti_ent)
hytti.position = Vector3(10, 1.2, -10)

# TODO add hemi light for shader testing

# TODO add directional sun light for shader testing

# Test spotlight
spot = game.scene.createLight("spot")
spot.type = "spot"
spot.attenuation = LightAttenuation(100, 0.9, 0.1, 0)
spot_n = game.scene.createSceneNode("spot")
spot_n.attachObject(spot)
# Really weird shadow effects when light is tied to the camera
spot_n.position = Vector3(0, 25, 35)
spot_n.orientation = Quaternion(0.974, -0.225, 0.025, 0.03)

game.scene.addToSelection(spot_n)

game.scene.sky = SkyDomeInfo("CloudySky")

hemi = game.scene.createLight("hemi")
hemi.attenuation = LightAttenuation(50, 0.9, 0.1, 0)
hemi_n = game.scene.createSceneNode("hemi")
hemi_n.attachObject(hemi)
hemi_n.position = Vector3(-20, 35, 0)

hemi2 = game.scene.createLight("hemi2")
hemi2.attenuation = LightAttenuation(50, 0.9, 0.1, 0)
hemi2_n = game.scene.createSceneNode("hemi2")
hemi2_n.attachObject(hemi2)
hemi2_n.position = Vector3(20, 35, -10)

spot2 = game.scene.createLight("spot_2")
spot2.type = "spot"
spot2.attenuation = LightAttenuation(100, 0.9, 0.1, 0)
spot2_n = game.scene.createSceneNode("spot_2")
spot2_n.attachObject(spot2)
# Really weird shadow effects when light is tied to the camera
spot2_n.position = Vector3(0, 50, 0)
spot2_n.orientation = Quaternion(0.753, -0.58, -0.13, -0.29)

# TODO create a configuration for lights toggled by keyboard/python commands
# Easy to do with python commands
# one point light
# one spot light
# two point lights
# two spot lights
# two point lights and two spot lights
# This is then profiled using the gDEBugger's profiler mode
# and saved
# DO the same tests for single pass shader when it's ready

