# -*- coding: utf-8 -*-

# Project specific definitions, like object creations

print('Creating Camera SceneNode')
camera_n = game.scene.createSceneNode("Camera")
camera = game.scene.createCamera("Camera")
camera.near_clip = 0.001
camera_n.attachObject(camera)
create_camera_controller()
camera_n.position = Vector3(0, 5, 20)

game.player.camera = "Camera"

game.scene.sky_dome = SkyDomeInfo("CloudySky")
game.scene.shadows.enable()
game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)

create_sun()

ground = create_ground()

# Initialise pcan
controller = game.event_manager.get_pcan()

# read data and print it to console
def can_message_received(msg):
	# How to read the Sandvik joystick information
	# length = 8 bytes
	# id = 0x380 + node_id
	# type = 0xFF
	#
	can_msg_id = 0x2b3
	# can also be 0x3b3 or 0x2b3
	# should be 0x2b2 because it changes if we mess around with the joystick
	# 0x2b2 is the left hand joystick only
	# 0x3b3 is the rightmost joystick and buttons from both of the right
	# joysticks, yeah weird right
	# 0x3b3 is not needed for these applications
	# 0x2b3 is the left one of the right joysticks with buttons from both of
	# the right joysticks, bit pattern seems to be same for both messages
	# should not be 0x4b2 because it doesn't have zero start data
	if msg.id == 0x2b2:
		print("Left boom control joystick message")
		assert(msg.length == 8)
		pos_x = msg.data[1]
		pos_y = msg.data[3]

		# Only the top button is used in these applications
		low_button = msg.data[6] & 1 == 1
		top_button = msg.data[6] & 1 << 1 == 1 << 1
		mid_button = msg.data[6] & 1 << 2 == 1 << 2

		print("(x, y) ({},{}) top button {} mid button {} low button {}".format(pos_x, pos_y, top_button, mid_button, low_button))

	if msg.id == can_msg_id:
		print("Right boom control joystick message")
		assert(msg.length == 8)

		# How to convert uint8_t to int so we preserve the bits (sign)?
		# Or should the data be stored in a int8_t instead?
		# stored in int8_t for now
		# For some reason for the joystick data the first byte is always crap
		# so the data actually takes 16-bits but only the last 8-bits is useful
		pos_x = msg.data[1]
		pos_y = msg.data[3]

		# Only the top button is used in these applications
		low_button = msg.data[6] & 1 == 1
		top_button = msg.data[6] & 1 << 1 == 1 << 1
		mid_button = msg.data[6] & 1 << 2 == 1 << 2

		print("(x, y) ({},{}) top button {} mid button {} low button {}".format(pos_x, pos_y, top_button, mid_button, low_button))


controller.add_listener(can_message_received)

