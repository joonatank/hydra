# -*- coding: utf-8 -*-

# Create a Camera and set it to be used
camera = game.scene.createSceneNode("camera")
camera.position = Vector3(0, 3, 15)
cam = game.scene.createCamera("camera")
camera.attachObject(cam)
# TODO fix this to use Camera object instead of name
game.player.camera = "camera"

# TODO these should be configured using options and created in global script
create_camera_controller()


# Create ground plane
# Create a large plane for shader testing
# This shows the usage of the new mesh manager
create_ground()

wall_ent = game.scene.createEntity('wall', "ground", True)
wall = game.scene.createSceneNode("wall")
wall.attachObject(wall_ent)
wall_ent.material_name = 'ground/bump_mapped/shadows'
wall_ent.cast_shadows = False
wall.orientation = Quaternion(0.7071, 0.7071, 0, 0)
wall.position = Vector3(0, 20, -20)

# Can't use the new mesh manager, there are some attributes in the mesh file
# that make it impossible
ogre_ent = game.scene.createEntity("ogre", "ogre.mesh", False)
ogre = game.scene.createSceneNode("ogre")
ogre.attachObject(ogre_ent)
ogre.position = Vector3(3, 2.5, -5)

athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "athene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scaling = Vector3(1,1,1)*0.05;

athene2 = athene.clone();
athene2.position = Vector3(3, 4, 5)

game.scene.shadows.enable()
# Should of course be 4096 but testing the rounding to next power of two
game.scene.shadows.texture_size = 4024
game.scene.shadows.max_distance = 50
game.scene.shadows.dir_light_extrusion_distance = 100

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

game.scene.sky_dome = SkyDomeInfo("CloudySky")


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

class Lights :
	def __init__(self):

		self._create_sun()
		self._create_spots()
		self._create_hemis()

		# Start with only the directional light
		self.toggleSpots()
		self.toggleHemis()

	def _create_sun(self):
		self.sun = game.scene.createLight("sun")
		self.sun.type = "directional"
		self.sun.cast_shadows = True
		self.sun.diffuse = ColourValue(0.5, 0.5, 0.5)
		self.sun_n = game.scene.createSceneNode("sun")
		self.sun_n.attachObject(self.sun)
		# Position is not needed for directional lights
		# Though it would be more intuitive if there was a sphere
		# along which the sun would move.
		self.sun_n.orientation = Quaternion(0.753, -0.58, -0.13, -0.29)
		game.scene.addToSelection(self.sun_n)
		create_selection_controller()

	def _create_spots(self):
		self.spot1 = game.scene.createLight("spot")
		self.spot1.type = "spot"
		self.spot1.attenuation = LightAttenuation(100, 0.9, 0.1, 0)
		# For PSSM shadow camera (and LiSPSM) Ogre has a bug that
		# does not allow low range values this might of course 
		# be because of some parameters how the split planes are created
		# but who the hell assumes some unit system when no unit system is
		# forced in the engine...
		#self.spot1.attenuation = LightAttenuation(1000, 0.9, 0.1, 0)
		self.spot1.diffuse = ColourValue(0.5, 0.5, 0.5)
		spot_n = game.scene.createSceneNode("spot")
		spot_n.attachObject(self.spot1)
		spot_n.position = Vector3(0, 25, 35)
		spot_n.orientation = Quaternion(0.974, -0.225, 0.025, 0.03)

		self.spot2 = game.scene.createLight("spot_2")
		self.spot2.type = "spot"
		self.spot2.attenuation = LightAttenuation(100, 0.9, 0.1, 0)
		self.spot2.diffuse = ColourValue(0.5, 0.5, 0.5)
		spot_n = game.scene.createSceneNode("spot_2")
		spot_n.attachObject(self.spot2)
		spot_n.position = Vector3(0, 50, 0)
		spot_n.orientation = Quaternion(0.753, -0.58, -0.13, -0.29)

	def _create_hemis(self):
		self.hemi1 = game.scene.createLight("hemi")
		self.hemi1.attenuation = LightAttenuation(50, 0.9, 0.1, 0)
		hemi_n = game.scene.createSceneNode("hemi")
		hemi_n.attachObject(self.hemi1)
		hemi_n.position = Vector3(-20, 35, 0)

		self.hemi2 = game.scene.createLight("hemi2")
		self.hemi2.attenuation = LightAttenuation(50, 0.9, 0.1, 0)
		hemi2_n = game.scene.createSceneNode("hemi2")
		hemi2_n.attachObject(self.hemi2)
		hemi2_n.position = Vector3(20, 35, -10)


	def toggleSun(self):
		self.sun.visible = not self.sun.visible

	def toggleHemis(self):
		vis = self.hemi1.visible
		self.hemi1.visible = not vis
		self.hemi2.visible = not vis

	def toggleSpots(self):
		# Keep the visibility same for both spots
		vis = self.spot1.visible
		self.spot1.visible = not vis
		self.spot2.visible = not vis

lights = Lights()

trigger = game.event_manager.createKeyTrigger(KC.U)
trigger.addListener(lights.toggleSpots)

trigger = game.event_manager.createKeyTrigger(KC.I)
trigger.addListener(lights.toggleHemis)

trigger = game.event_manager.createKeyTrigger(KC.P)
trigger.addListener(lights.toggleSun)

def toggleShadows():
	game.scene.shadows.enabled = not game.scene.shadows.enabled

trigger = game.event_manager.createKeyTrigger(KC.Z)
trigger.addListener(toggleShadows)

