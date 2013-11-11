#Should we create one file purely for imports? Or is python
#going to care about multiple math imports?
import math


""" Brief explanation of this file: includes all classes and functions related to wand dragger.
at the moment includes classes like VisualRay, RayDragger, WorldTransformGenerator of which
only WorldTransformGenerator depends on Signal class. """

class VisualRay:
	""" VisualRay was supposed to be the union of visual representation
	and ray test algorithm. However due lack of time
	it was never finalized or even designed properly. Currently it's used only
	as stupid adapter/container for original c++ RayObject.
	It attaches RayObject to identically named scene node """
	def __init__(self, name, direction=Vector3(0,0,-1), indicator_size = 0.05):
		self.scene_node = game.scene.createSceneNode(name)
		self.object = game.scene.createRayObject(name, "collision/light")
		self.object.direction = direction
		self.object.draw_collision_sphere = True
		self.object.collision_detection = True
		self.object.sphere_radius = indicator_size
		self.scene_node.attachObject(self.object)

	def hide(self):
		self.object.hide()
	
	def show(self):
		self.object.show()
		
	def set_distance(self, value):
		self.object.length = value
	
	#Unite VisualRay's and scene_nodes transformation:
	@property
	def world_transformation(self):
		return self.scene_node.world_transformation
	
	@world_transformation.setter
	def world_transformation(self, tr):
		self.scene_node.world_transformation = tr
	
	def collision_indicator_on(self):
		self.object.draw_collision_sphere = True
		self.object.collision_detection = True
	
	def collision_indicator_off(self):
		self.object.draw_collision_sphere = False
		self.object.collision_detection = False
		

class IRayDragger:
	""" Useless interface class for RayDragger class.
	In the future RayDragger should be sort of like an
	operator which will get it's data for example from
	ray object.
	"""
	def reset(self):
		raise NotImplementedError
	
	def start_drag(self):
		raise NotImplementedError
	
	def stop_drag(self):
		raise NotImplementedError
	
	def toggle_drag(self):
		raise NotImplementedError
	
	def change_drag_distance(self, axis):
		raise NotImplementedError
	
	def increase_drag_distance(self):
		raise NotImplementedError
	
	def decrease_drag_distance(self):
		raise NotImplementedError
	
	def frame_callback(self, dtime):
		raise NotImplementedError
	
	def tracker_callback(self, transformation):
		raise NotImplementedError

		
class RayDragger(IRayDragger):
	""" This class is meant for interfacing with ray operations related to dragging.
	Currently the class will use global scene selection and only one object is possible
	to add into it. For visualization of this ray a class called VisualRay
	is used. RayDragger instantiates this class which in turn will create c++
	RayObject and scene node. 
	Init arguments:
	name = name used for creating the VisualRay
	ray_pickable_objects = list of objects which can be dragged. If empty all
	objects which have bullet collision object can be dragged.
	speed = is the speed used for translating dragged object
	towards and outwards of the ray origin.
	ray_max_distance = the distance how far the raytests are performed,
	also it's linked to VisualRay's	length. """
	
	def __init__(self, visual_ray, ray_pickable_objects=[], speed=5.0, ray_max_distance=15.0):
		self.visual = visual_ray
		self.selected = []
		self.pickables = ray_pickable_objects
		self.speed = speed
		self.ray_max_distance = ray_max_distance
		self.drag_distance = ray_max_distance
		self.world_transformation = Transform()
		
		#State variables:
		self.disabled = False
		self.dragging = False
		self.drag_distance_signal = 0.0
	
	#Draggin related controls:	
	def reset(self):
		""" With this function we're able to reset
		dragger to init state. """
		self.dragging = False
		self.disabled = False
		self.drag_distance = self.ray_max_distance
		game.scene.clearSelection()
		del self.selected[:]
		self.drag_distance_signal = 0.0
	
	#User operated callback for setting dragging on.
	def start_drag(self):
		""" callback which will try to ray test. If there are any hits
		they will be added to global selection as scene nodes and
		into local selection as hit data """
		if self.disabled or self.dragging:
			return
		startpos = self.world_transformation.position
		endpos_in_ray = -Vector3.unit_z * self.ray_max_distance
		endpos = self.world_transformation * endpos_in_ray
		result = game.physics_world.cast_first_hit_ray(startpos, endpos)
		
		#We have to be sure there were some results:
		try:
			hit_object = result[0].hit_object
		except IndexError:
			print("No hits to drag.")
			return
		hit_point = result[0].hit_point
		pivot_in_ray = self.world_transformation.inverted() * hit_point 
		object_in_ray = self.world_transformation.inverted() * hit_object.motion_state.world_transformation
		pivot_object_offset_in_ray = object_in_ray.position - pivot_in_ray
		
		if hit_object in self.pickables or len(self.pickables) is 0:
			self.selected.append((hit_object, object_in_ray, pivot_in_ray, pivot_object_offset_in_ray))
			game.scene.addToSelection(hit_object.motion_state.node)
			self.drag_distance = pivot_in_ray.length()
			self.dragging = True
			
	def stop_drag(self):
		""" Callback for stopping the dragging procedure.
		Clears the global and local selections and sets
		ray distance to max. """
		if self.dragging:
			print("Unpicking")
			self.dragging = False
			self.drag_distance = self.ray_max_distance
			#Clear the local selection:
			game.scene.clearSelection()
			del self.selected[:]
	
	#User operated callback for toggling drag function!
	def toggle_drag(self):
		""" Convenience callback for toggling drag action. """
		if self.disabled:
			return
		
		if self.dragging:
			self.stop_drag()
		else:
			self.start_drag()
	
	
	def frame_callback(self, dtime):
		""" This is a callback meant to connect to global frame trigger. 
		handles updating the dragging distance and dragging itself.
		Also takes care of the update_transform function which is used
		for keyboard callbacks"""
		if self.disabled:
			return
		if self.dragging:
			self._update_drag_distance(dtime)
			self._during_drag(dtime)
			
		
	
	#This is a callback for a trigger/signal which provides a world transformation for
	#ray origin. eg. callback for tracker trigger.
	def tracker_callback(self, transformation):
		""" Quite dangerous tracker callback. @todo: remove
		the need for this callback or at least remove the chaining! """
		#Transforming the wand's coordinate system into active camera's:
		tr = game.player.camera_node.world_transformation * transformation
		#Setting the world transformation of RayDragger:
		self.world_transformation = tr
		#Setting the same transformation for VisualRay
		#this is stupid chaining which should be removed asap.
		self.visual.world_transformation = tr
	
	#Some user operated dragging distance callbacks. Operable
	#for example with buttons/keys and (normalized [-1.0, 1.0])analog axes.
	def change_drag_distance(self, value):
		""" Range type callback for updating dragging distance. """
		self.drag_distance_signal = value
	
	def increase_drag_distance(self):
		""" State type callback for increasing dragging distance. """
		self.drag_distance_signal += 1.0
	
	def decrease_drag_distance(self):
		""" State type callback for decreasing dragging distance. """
		self.drag_distance_signal -= 1.0
	
	#This has to be run once per frame. It is executed in frame callback function!
	def _during_drag(self, dtime):
		""" This function is ran once per frame. It's purpose is to update
		dragged object as long as dragging state is on eg. ray test
		results generated in start_drag are utilized here. Currently
		this can be used only for dragging bullet collision objects, although
		this will hopefully change soon. """
		#@todo: support for multiple selections!
		for obj, object_in_ray, pivot_in_ray, pivot_object_offset_in_ray in self.selected:
			#In ray space we set the z coordinate according to dragging distance which user can define.
			#But also we have to take account the offset of the object to the ray pivot point:
			object_in_ray.position = Vector3(0, 0, -self.drag_distance) + pivot_object_offset_in_ray
			#We transform this back to world space and set it as transformation for selected object:
			obj.motion_state.world_transformation = self.world_transformation * object_in_ray
	
	def _update_drag_distance(self, dtime):
		""" Updates dragged objects distance from viewer.
		Also updates visual ray's length. This is a weird
		way of doing so simple tasks. Should change this to
		use object controller or similar. """
		self.drag_distance += self.speed * math.copysign( min(abs(self.drag_distance_signal), 1.0), self.drag_distance_signal) * float(dtime)
		self.visual.set_distance(self.drag_distance)

		
class WorldTransformGenerator:
	""" Class used to generate tracker transformation
	for tracker callbacks. Purely for debug use. First implementation
	was meant for sine wave generation (didn't have time to test it properly),
	but I stripped all that for simplicity and left only keyboard controls. 
	Current implementation depends on Signal().
	Problem with the Signal is it's slowness. At the moment reference
	isn't applicable! """
	def __init__(self, init_transform=Transform(), reference=Transform(), speed=10.0, angular_speed=1.0):
		self.speed = speed
		self.angular_speed = angular_speed
		self.transformation = init_transform
		self.disabled = False
		#Control signals:
		self.trans_sig = Vector3.zero
		self.rot_sig = Vector3.zero
		
		#Signals (Signal slot implementation):
		self.world_transformation_changed = Signal()
	
	def disable(self):
		self.disabled = True
	
	def enable(self):
		self.disabled = False
	
	def connect(self, slot):
		self.world_transformation_changed.connect(slot)
	
	def disconnect(self, slot):
		self.world_transformation_changed.disconnect(slot)
	
	def clear_connections(self):
		self.world_transformation_changed.clear()
		
	def up(self):
		self.trans_sig.y += 1.0
	
	def down(self):
		self.trans_sig.y -= 1.0
	
	def left(self):
		self.trans_sig.x -= 1.0
	
	def right(self):
		self.trans_sig.x += 1.0
	
	def forward(self):
		self.trans_sig.z -= 1.0
	
	def backward(self):
		self.trans_sig.z += 1.0
	
	def rotate_left(self):
		self.rot_sig.y += 1.0 
	
	def rotate_right(self):
		self.rot_sig.y -= 1.0
	
	def rotate_up(self):
		self.rot_sig.x += 1.0
	
	def rotate_down(self):
		self.rot_sig.x -= 1.0
		
	def roll_cw(self):
		self.rot_sig.z -= 1.0
		
	def roll_ccw(self):
		self.rot_sig.z += 1.0	
	
	def frame_callback(self, dtime):
		if self.disabled:
			return
		#Translations:
		sig = self.trans_sig
		sig_len = sig.length()
		translation = Vector3.zero
		if sig_len != 0:
			sig /= sig_len
			translation = self.speed * float(dtime) * sig * min(sig_len, 1.0)
		#Rotations:
		sig = self.rot_sig
		sig_len = sig.normalise()
		angle = Radian(self.angular_speed * float(dtime) * min(sig_len, 1.0))
		rotation = Quaternion(angle, sig)
		
		self.transformation.position += translation
		self.transformation.quaternion = rotation * self.transformation.quaternion 
		#Finally we dispatch the signal:
		self.world_transformation_changed(self.transformation)


def createWandRayController(name="wand", tracker_name="wand", debug=False):
	""" Creates a dragger instance and adds controls to it. If debug flag is used
	then it will not use tracker or joystick to determine ray transformation but 
	keyboard controls are used. Decision to use it like this is that the ray should be never
	controlled via keyboard. To change the mapped keys or adding more dofs to ray movement
	just add triggers inside this... In the near future I will create an interface
	for the ray object and ray test so it can be used more easily. Also one idea is to add attach method inside
	this unified ray which can be used to attach ray to any object/scene_node.
	This way we don't have to use tracker to control the dragger itself,
	but we can use global Controller class to control the attached object.
	RayDragger part will be merely a decorator for Ray or something similar. """
	visual_ray = VisualRay(name)
	dragger = RayDragger(visual_ray)
	game.event_manager.frame_trigger.addListener(dragger.frame_callback)
	if not debug:
		if game.event_manager.hasTrackerTrigger(tracker_name):
			ttrigger = game.event_manager.getTrackerTrigger(tracker_name)
			ttrigger.addListener(dragger.tracker_callback)
		else:
			raise ValueError("Tracker trigger with a name: ", tracker_name, " does not exist!")
		""" Joystick related:
		create_splitfish_handler creates JoystickTrigger
		and JoystickEventListener (for splitfish) and connects these two: """
		jhandler = create_splitfish_handler(name)
		""" For time being the only ops are Flip and ZeroCut. Those are operator objects of which Flip is
		multiplying the joystick axis value with -1.0 and ZeroCut is cutting axis to zero when it's value
		threshold is smaller than threshold size. Threshold size is set default as 0.03. ZeroCut(threshold_size)
		Operators are kinda stupid hack at the moment... """
		jhandler.connect_range(2, dragger.change_drag_distance, JoystickEventHandler.Flip(), JoystickEventHandler.ZeroCut())
		# TODO Replace by button 8
		# TODO Test the new function
		jhandler.connect_action(0, dragger.toggle_drag)
		#jhandler.connect_state(0, start_drag, stop_drag)
		return dragger, jhandler
	else:
		""" This is for debugging part. If you want more dofs or change mapping feel free to edit. """
		debug_movements = WorldTransformGenerator(Transform(), game.player.camera_node.world_transformation,
				speed=1)
		game.event_manager.frame_trigger.addListener(debug_movements.frame_callback)
		#Connecting the generator to dragger's callback:
		debug_movements.connect(dragger.tracker_callback)
		
		trigger = game.event_manager.createKeyTrigger(KC.I)
		trigger.addKeyDownListener(debug_movements.up)
		trigger.addKeyUpListener(debug_movements.down)
		
		trigger = game.event_manager.createKeyTrigger(KC.K)
		trigger.addKeyDownListener(debug_movements.down)
		trigger.addKeyUpListener(debug_movements.up)
		
		trigger = game.event_manager.createKeyTrigger(KC.L)
		trigger.addKeyDownListener(debug_movements.right)
		trigger.addKeyUpListener(debug_movements.left)
		
		trigger = game.event_manager.createKeyTrigger(KC.J)
		trigger.addKeyDownListener(debug_movements.left)
		trigger.addKeyUpListener(debug_movements.right)
		
		trigger = game.event_manager.createKeyTrigger(KC.Y)
		trigger.addKeyDownListener(debug_movements.forward)
		trigger.addKeyUpListener(debug_movements.backward)
		
		trigger = game.event_manager.createKeyTrigger(KC.H)
		trigger.addKeyDownListener(debug_movements.backward)
		trigger.addKeyUpListener(debug_movements.forward)
		
		trigger = game.event_manager.createKeyTrigger(KC.O)
		trigger.addKeyDownListener(debug_movements.rotate_right)
		trigger.addKeyUpListener(debug_movements.rotate_left)
		
		trigger = game.event_manager.createKeyTrigger(KC.U)
		trigger.addKeyDownListener(debug_movements.rotate_left)
		trigger.addKeyUpListener(debug_movements.rotate_right)
		#Dragging related:
		trigger = game.event_manager.createKeyTrigger(KC.N8)
		trigger.addKeyUpListener(dragger.toggle_drag)
		
		trigger = game.event_manager.createKeyTrigger(KC.N9)
		trigger.addKeyDownListener(dragger.increase_drag_distance)
		trigger.addKeyUpListener(dragger.decrease_drag_distance)
		
		trigger = game.event_manager.createKeyTrigger(KC.N7)
		trigger.addKeyDownListener(dragger.decrease_drag_distance)
		trigger.addKeyUpListener(dragger.increase_drag_distance)
		
		return dragger, debug_movements

