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

