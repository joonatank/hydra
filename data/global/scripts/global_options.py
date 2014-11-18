# Options structure
# anything can be added to it (because of pythons dynamic typing)
# this structure should be created in c++ to game.options during initialisation
# of the python engine
# these default options would be hard coded or based on the c++ program
# these would then be changed by a ini parser writen in python
#
# they could then be changed from the project specific python script (maybe)
#	- problem with this one is that we can't move stuff to global script
#	without ending up using the options before the user modifies is
# then they would be used
#
# TODO we need a parser for ini files
# Ini files should be stored inside the project directory and named {PROJECT_NAME}.ini
# 
class Options :
	class Camera :
		def __init__(self):
			self.forward = KC.W
			self.left = KC.A
			self.backward = KC.S
			self.right = KC.D
			self.up = KC.PGUP
			self.down = KC.PGDOWN
			self.rotate_left = KC.Q
			self.rotate_right = KC.E
			self.joystick_enabled = True
			# For HMD
			# Misc configurations
			self.speed = 5
			self.high_speed = 10
			self.angular_speed = Degree(90)
			self.high_speed_disabled = False
			# Start the controller as disabled
			self.controller_disabled = False

		# Too lazy to actually implement printing
		def __repr__(self):
			return str(dir(self))
		def __str__(self):
			# Just messing with the pringting
			meh = "forward : {}\nbackward : {}\nleft : {}\nright : {}".format(self.forward, self.backward, self.left, self.right)
			return "Options.Camera\n{}\nspeed = {} and high speed = {}".format(meh, self.speed, self.high_speed)

	# Endof Camera


	class Selection:
		def __init__(self):
			self.forward = KC.NUMPAD8
			self.backward = KC.NUMPAD5
			self.left = KC.NUMPAD4
			self.right = KC.NUMPAD6
			self.up = KC.NUMPAD9
			self.down = KC.NUMPAD7
			# No rotations for the moment, need to add SHIFT toggle for them
			# since selection is special case we can probably just add a configure for the
			# specific toggle e.g. SHIFT or CTRL
			# and disable specific axes
			#self.rotate_left = KC.
			#self.rotate_right = KC.
			self.joystick_enabled = True
			# Misc configurations
			self.speed = 0.5
			self.angular_speed = Degree(30)
			# Start the controller as disabled
			self.controller_disabled = False

		# Too lazy to actually implement printing
		def __repr__(self):
			return str(dir(self))
		def __str__(self):
			return "Options.Selection"

	# Endof Selection

	#### Options Methods ####
	def __init__(self):
		print("Creating options : with camera and selection")
		self.camera = Options.Camera()
		self.selection = Options.Selection()
		print("Camera : ", self.camera)
		print("Selection : ", self.selection)

	# Too lazy to actually implement printing
	def __repr__(self):
		return "Options"
	def __str__(self):
		return "Options"


game.options = Options()

