# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

camera = game.scene.getSceneNode("editor/camera")
#camera.position = Vector3(0, 3, 15)
createCameraMovements(camera, speed=10)

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
ground.translate(0, -1, 0)
# Shader material, with shadows
#ground_ent.material_name = 'ground/bump_mapped/shadows'
ground_ent.material_name = 'ground/flat/shadows'
ground_ent.cast_shadows = False

light = game.scene.createLight("light")
light_n = game.scene.createSceneNode("light")
light_n.attachObject(light)
light_n.position = Vector3(0, 30, 5)

# TODO fix the enity name in the DAE importer
cube_ent = game.scene.getEntity("Cube-mesh")
material = game.material_manager.get_material("Material")
#material.emissive = ColourValue(0.5, 0, 0)

