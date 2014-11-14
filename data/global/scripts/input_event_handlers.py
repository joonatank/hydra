class InputEventHandler:
	""" InputEventHandler is a sad attempt to create a "generic" event handler for input devices.
	At the moment it's bad design isn't really so generic and due this
	it has to be redesigned completely.
	Point is that you can create multiple handlers per device. You can
	input required device id, vendor id and owner id's to match the correct device
	you want or use empty string to mark it as any device. """
	
	def __init__(self, name, device_id="", vendor_id="", owner_id=""):
		self.name = name
		self.did = device_id
		self.vid = vendor_id
		self.oid = owner_id
		self.disabled = False
		#self.event_states = dict()
			
	def __call__(self, evt, cid):
		""" This function is used for all handlers! It's basically a template
		for executing the process of event handling... """
		if not self.device_matching(evt.info.device_id, evt.info.vendor_id, evt.info.owner_id) or self.disabled:
			return
		self.handler(evt, cid)
	
	def connect_action(cid, slot):
		""" Connect functions are functions which are
		are used to connect callbacks to corresponding device events.
		Action type events are all events which happens for example 
		when button is pressed. cid is abbreviation of component id
		in the case of joystick for example it may mean button id or axis id."""
		raise NotImplementedError
	
	def connect_state(cid, slot_on, slot_off):
		""" State type event, has on and off states eg. button pressed and released. """
		raise NotImplementedError
		
	def connect_range(cid, slot, *ops):
		""" Range can be compared to for example joystick axis! """
		raise NotImplementedError
	
	def connect_vector3(cid, slot, *ops):
		""" Some input devices might output vector3 """
		raise NotImplementedError
	
	def connect_quaternion(cid, slot, *ops):
		raise NotImplementedError
	
	def connect_transform(cid, slot, *ops):
		raise NotImplementedError
		
	def handler(self, evt, cid):
		""" handler is a user modifiable function which is used for handling
		the input device event. """
		raise NotImplementedError("Use concrete event handler classes and implement handle method")
		
	def disable(self):
		self.disabled = True

	def enable(self):
		self.disabled = False

	def device_matching(self, did, vid, oid):
		""" Function which is used for recognizing different devices.
		At the moment it's very simple."""
		if self.did == did or self.did == "":
			if self.vid == vid or self.vid == "":
				if self.oid == oid or self.oid == "":
					return True
		return False
	

class JoystickEventHandler(InputEventHandler):
	""" Event handler for joysticks. """
	def __init__(self, name, device_id="", vendor_id="", owner_id=""):
		InputEventHandler.__init__(self, name, device_id, vendor_id, owner_id)
		self.signals = dict()
	
	class Flip:
		""" Operator for flipping axis value to negative/positive """
		def __init__(self):
			pass
		def __call__(self, value):
			return -1.0*value
	
	class ZeroCut:
		""" Operator for cutting the joystick axis to zero if there's error signal present"""
		def __init__(self, size=0.03):
			self.size = size
		def __call__(self, value):
			if abs(value) < abs(self.size):
				return 0
			return value
			
	def handler(self, evt, cid):
		""" During writing this function handles only button presses, axes and vectors.
		This is supposed to add as listener into joystick trigger"""
		#print("EVENT: ", evt, "COMPONENT ID: ", cid)
		try:
			all = self.signals[evt.type, cid]
		except KeyError:
			return
		sig = all[0]
		ops = all[1]
		st = evt.state
		if evt.type == JOYSTICK_EVENT_TYPE.BUTTON_PRESSED:
			sig()
		elif evt.type == JOYSTICK_EVENT_TYPE.BUTTON_RELEASED:
			sig()
		elif evt.type == JOYSTICK_EVENT_TYPE.AXIS:
			try:
				val = st.axes[cid]
			except IndexError("No index: ", cid, " in joystick event vectors!"):
				return
			for op in ops:
				val = op(val)
			sig(val)
		elif evt.type == JOYSTICK_EVENT_TYPE.VECTOR:
			try:
				val = st.vectors[cid]
			except IndexError("No index: ", cid, " in joystick event vectors!"):
				return
			for op in ops:
				val = op(val)
			sig(val)
		else:
			print("Unknown joystick event type!")
	
	def connect_action(self, cid, slot):
		""" Button presses are mapped to actions, cid is button number
		and slot is a callback without arguments eg. action """
		self.__connect(JOYSTICK_EVENT_TYPE.BUTTON_PRESSED, cid, slot)
	
	def connect_state(self, cid, slot_on, slot_off):
		""" Button press and release is mapped to state with state on and
		state off callbacks. cid is button number """
		self.__connect(JOYSTICK_EVENT_TYPE.BUTTON_PRESSED, cid, slot_on)
		self.__connect(JOYSTICK_EVENT_TYPE.BUTTON_RELEASED, cid, slot_off)
	
	def connect_range(self, cid, slot, *ops):
		""" Axes from joystick event are mapped to ranges. 
		cid is axis number and slot is a callback with single
		value as argument (eg. axis value) """
		self.__connect(JOYSTICK_EVENT_TYPE.AXIS, cid, slot, *ops)
	
	def connect_vector3(self, cid, slot, *ops):
		""" This function isn't tested and I don't even know should
		we use it. Same idea as before but this time passed value
		is vector3 type. Operations have to react accordingly."""
		self.__connect(JOYSTICK_EVENT_TYPE.VECTOR, cid, slot, *ops)
	
	def __connect(self, event_type, cid, slot, *ops):
		#This check might be a duplicate:
		if not callable(slot):
			raise ValueError("Slot connected to joystick handler isn't callable!")
		for op in ops:
			if not hasattr(op, "__call__"):
				raise ValueError("Operator used for connecting joystick handler isn't callable!")
		if not (event_type, cid) in self.signals:
			self.signals[event_type, cid] = (Signal(), ops)
		self.signals[event_type, cid][0].connect(slot)
	


def create_splitfish_handler(name):
	handler = JoystickEventHandler(name, "", "SplitFish Game Controller", "")
	trigger = game.event_manager.createJoystickTrigger()
	trigger.add_listener(handler)
	return handler
