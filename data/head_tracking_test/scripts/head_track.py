
start_node = game.scene.getSceneNode("position_left")

camera = game.scene.createSceneNode("camera")
cam = game.scene.createCamera("camera")
camera.attachObject(cam)
camera.position = start_node.position
camera.orientation = Quaternion(0, 0, 1, 0)
#camera.rotate(Quaternion(0, 0, 1, 0))

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()

# Create light
spot = game.scene.createSceneNode("spot")
spot_l = game.scene.createLight("spot")
#spot_l.type = "spot"
spot.attachObject(spot_l)
spot.position = Vector3(0, 20, -10)
game.scene.addToSelection(spot)
addMoveSelection()

createCameraMovements(speed=1)
game.player.camera = "camera"

# TODO These are not affected by each other..
game.player.head_transformation = Transform(Vector3(0, 1.61, 0))
class HeadController():
	def __init__(self):
		self.left = True
		self.down = False

	def sideStep(self):
		self.left = not self.left
		self._refresh()

	def crouch(self):
		self.down = not self.down
		self._refresh()

	def _refresh(self):
		t = Transform()
		if self.down:
			t.position += Vector3(0, 0.16, 0)
		else:
			t.position += Vector3(0, 1.61, 0)

		if self.left:
			t.position += Vector3(0, 0, 0)
		else:
			t.position += Vector3(0.74, 0, 0)

		game.player.head_transformation = t

# Add key toggle to change head tracking using player.head_transformation
head = HeadController()
trigger = game.event_manager.createKeyTrigger(KC.X)
trigger.addListener(head.sideStep)

trigger = game.event_manager.createKeyTrigger(KC.Z)
trigger.addListener(head.crouch)

