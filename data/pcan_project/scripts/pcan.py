# -*- coding: utf-8 -*-

import struct

# Project specific definitions, like object creations

print('Creating Camera SceneNode')
camera_n = game.scene.createSceneNode("Camera")
camera = game.scene.createCamera("Camera")
camera.near_clip = 0.001
camera_n.attachObject(camera)
createCameraMovements()
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
def print_message(msg):
	# TODO utf-8 error if we print the message
	print("New message from CAN : ", msg)
	#msg.print()
	# Add deciphering the can message
	# How to read the Sandvik joystick information
	# length = 8 bytes
	# id = 
	# type = 0xFF
	#
	node_id = 0
	can_msg_id = 0x380 + node_id
	print("can msg id 0x{0:x}".format(can_msg_id))
	# For some reason 0x380 + 0 != 0x380, wut?
	if msg.id == can_msg_id:
		print("Joystick message")
		assert(msg.length == 8)

		# How to convert uint8_t to int so we preserve the bits (sign)?
		# Or should the data be stored in a int8_t instead?
		# stored in int8_t for now
		boom_pos_x = msg.data[0]
		boom_pos_y = msg.data[1]
		boom_dir_x = msg.data[2]
		boom_dir_y = msg.data[3]
		drill_dir_x = msg.data[4]
		drill_dir_y = msg.data[5]
		#print("values : {} {} {} {} {} {}".format(boom_pos_x, boom_pos_y, boom_dir_x, boom_dir_y, drill_dir_x, drill_dir_y))

		# Vaihda puomin liiketta, pyytopainike 0
		boom_dir_top_button = msg.data[6] & 1 == 1
		# Kaynnista automaattinen poraus, pyyntopainike
		boom_dir_upper_side_button = msg.data[6] & 1 << 1 == 1 << 1
		# Kayta TRH-kangenvaihdinta (option), pyyntopainike
		boom_dir_lower_side_button = msg.data[6] & 1 << 2 == 1 << 2
		# Kierteytys, pyyntopainike 0
		drilling_top_button = msg.data[6] & 1 << 3 == 1 << 3
		# Isku paalle, pyyntopainike 0
		drilling_upper_side_button = msg.data[6] & 1 << 4 == 1 << 4
		# Finish hole request active 0
		drilling_lower_side_button = msg.data[6] & 1 << 5 == 1 << 5

		# Vaihda puomin liiketta, pyyntopainike 0
		boom_pos_top_button = (msg.data[7] & 1) == 1
		# Lukitse lahin reika paikoitusikkunaan, pyyntopainike
		boom_pos_lower_side_button = (msg.data[7] & 1 << 1) == 1 << 1
		# Kayta TRS-kangenvaihdinta (option), pyyntopainike
		boom_pos_upper_side_button = (msg.data[7] & 1 << 2) == 1 << 2
		# Kayttopaikan lukituskytkimen tila ( 1 = lukittu, 0 = kaytossa )
		hmi_locked = (msg.data[7] & 1 << 3) == 1 << 3
		# 1 = Valid, 0 = Not valid Normaali
		pdo_valid = (msg.data[7] & 1 << 4) == 1 << 4


controller.add_listener(print_message)

