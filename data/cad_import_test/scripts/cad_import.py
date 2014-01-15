# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

camera = game.scene.getSceneNode("editor/camera")
#camera.position = Vector3(0, 3, 15)
create_camera_controller()

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
ground = create_ground()
#ground.translate(0, -6, 0)
create_sun()

# We need to create a callback here
cb = CadImporterCallback.create(game)
#import_file = "assy1.SLDASM"
import_file = "puominosturi.SLDASM"
game.cad_importer.load(import_file, cb)

"""
camera = game.player.camera_node
assert(camera)
camera.position = Vector3(-3., -2.1, -10)
camera.orientation = Quaternion(-0.1629, 0, 0.9866, 0)
"""

