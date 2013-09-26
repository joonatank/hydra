# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

# config.getSceneNode gets a reference to already created SceneNode
# For now it's not possible to create SceneNodes from python
# So use this function to get a SceneNode created from .scene file.
# All SceneNodes in .scene file are created and can be retrieved here.
# Tangent space lighting does not work on the ogre_ent

#TRACKING:


class TrackerTrigger:
        def __init__(self, tracker_name, trigger_name, default_transform = Transform(Vector3(0.0,0.0,0.0))):
                self.tracker_name = tracker_name
                self.trigger_name = trigger_name
                self.default_transform = default_transform
                if not game.event_manager.hasTrackerTrigger(self.trigger_name):
                        self.tracker = Tracker.create(tracker_name)
                        self.tracker.n_sensors = 1
                        self.tracker.getSensor(0).default_transform = self.default_transform
                        self.trigger = game.event_manager.createTrackerTrigger(self.trigger_name)
                        self.tracker.getSensor(0).trigger = self.trigger
                        game.tracker_clients.addTracker(self.tracker)
                self.trigger = game.event_manager.getTrackerTrigger(self.trigger_name)

        def add_listener(self,listener):
                self.trigger.addListener(listener)

                
class ControllerContext
        def __init__(self, nodes, speed = 5, high_speed = 10, angular_speed = Degree(90),
                     forward_axis = Vector3(0.0,0.0,-1.0), reference=TS.LOCAL):
                self.speed = speed
                self.high_speed = high_speed
                self.angular_speed = angular_speed
                self.forward_axis = forward_axis
                self.reference = reference
                self.disabled = False
        def toggle_disable(self):
                self.disabled = not self.disabled
        def rotate(self, quat):
                pass
        def translate(self, vector):
                pass

class Joystick:
        def __init__(self, name):
                self.trigger = game.event_manager.getJoystick(name)
class JoystickMappingWrapper:
        def __init__(self, name):
                self.trigger = game.event_manager.getJoystick(name)
                self.trigger.addListener(self.update)
        def update(self, evt)

class FragfxsharkMappingWrapper(Joystick):
        def __init__(self):
                super().__init__("Fragfxshark")
                self.trigger.addListener(self.update)
        def update(self, evt):
                self.dpad_vertical = evt.axis_y
                self.dpad_horizontal = evt.axis_x
                self.pov_vertical = evt.axis_z
                self.pov_vertical = evt.axis_w
                self.btn_select = evt.is_button_down(0)
                self.btn_ps = evt.is_button_down(1)
                self.btn_f = evt.is_button_down(2)
                self.btn_1 = evt.is_button_down(3)
                self.btn_2 = evt.is_button_down(4)

class RumblePadMappingWrapper(Joystick):
        def __init__(self):
                super().__init__("RumblePad")
                self.trigger.addListener(self.update)
        def update(self, evt):
                self.dpad_vertical = Vector2(evt.axis_x, evt.axis_y)
                self.dpad_horizontal = evt.axis_y
                self.pov_left_vertical = None
                self.pov_left_horizontal = None
                self.pov_right_vertical = None
                self.pov_right_horizontal = None
                self.btn_mode = is_button_down(10)
                self.btn_1 = is_button_down(0)
                self.btn_2 = is_button_down(1)
                self.btn_3 = is_button_down(2)
                self.btn_4 = is_button_down(3)
                self.btn_5 = is_button_down(4)
                self.btn_6 = is_button_down(5)
                self.btn_7 = is_button_down(6)
                self.btn_8 = is_button_down(7)
                self.btn_9 = is_button_down(8)
                self.btn_10 = is_button_down(9)


class ActionComponent:
        def __init__(self): pass
        def map_target(self, target):
                self.target = target

class Action:
        def __init__(self, targets=[]):
                self.targets = targets
                self.action_disabled = False
        def execute(self, *arg):
                print("Method execute not implemented!")

        def update_targets(self, targets):
                self.targets = targets

        def add_listener(self, listener): print("Method add_listener not implemented!")
        def disable_action(self):
                self.action_disabled = True
        def enable_action(self):
                self.action_disabled = False
                

#Toggle high speed should be broken into an action component. This way we don't need to copypaste the code all the time.
#Also currently the moving and rotating codes are identical so basically we could unite this with more general method and member variable names.
class MoveTargets(Action):

        def __init__(self, targets=[], speed=5.0, high_speed=10.0, move_direction=Vector3(0.0,0.0,-1.0), reference=TS.LOCAL):
                super().__init__(targets)
                self.speed = speed
                self.high_speed = high_speed
                self.move_direction = move_direction
                self.coordinate_reference = reference
                self.high_speed_enabled = False

        def execute(self, dtime=None):
                if self.action_disabled:
                        return
                translation = speed*self.move_direction
                if dtime:
                        translation = float(time)*translation
                for target in self.targets:
                        target.translate(translation, self.reference)

        def update_direction(self, direction): pass

        def toggle_high_speed(self):
                self.high_speed_enabled = not self.high_speed_enabled 


class RotateTargets(Action):

        def __init__(self, targets=[], speed=2.0, high_speed=5.0, rotation_axis=Vector(0.0,0.0,0.0), reference=TS.LOCAL):
                super().__init__(targets)
                self.speed = speed
                self.high_speed = high_speed
                self.rotation_axis = rotation_axis
                self.coordinate_reference = reference
                self.high_speed_enabled = False

        def execute(self, dtime=None):
                if self.action_enabled:
                        return
                speed = self.speed
                if self.high_speed_enabled:
                        speed = self.high_speed
                rotation = speed*self.rotation_axis
                if dtime:
                        rotation = float(dtime)*rotation
                for target in self.targets:
                        target.rotate(rotation, self.reference)

        def update_axis(self, new_axis):
                self.axis = new_axis

        def toggle_high_speed(self):
                self.high_speed_enabled = not self.high_speed_enabled


class SetTargetOrientation(Action):
        def __init__(self,targets=[], orientation=Quaternion.zero, reference=None):
                self.targets = targets
                self.orientation = orientation
        def execute(self):
                for target in targets:
                        target.quaternion = orientation


class Actions:

        def __init__(self, name):
                self.name = name
                
        def rotate(self, targets=[], speed=0.0, axis=Vector3(0.0, 1.0, 0.0), reference=TS.LOCAL):
                raise NotImplementedError, "You haven't implemented Rotate action!"

        def translate(self, targets=[], speed=0.0, axis=Vector3(0.0, 0.0, -1.0), reference=TS.LOCAL):
                raise NotImplementedError, "You haven't implemented Translate action!"

        def set_orientation(self, targets=[], orientation=Quaternion(1.0, 0.0, 0.0, 0.0), reference=TS.GLOBAL):
                raise NotImplementedError, "You haven't implemented SetOrientation action!"

        def show(self, targets=[]):
                raise NotImplementedError, "You haven't implemented SetOrientation action!"

        def hide(self, targets=[]):
                raise NotImplementedError, "You haven't implemented SetOrientation action!"


class Action:
        def __init__(self, targets=[]):
                self.targets = targets
                self.disabled = False
        def execute(self, **kwargs): raise NotImplemented, "For some weird reason execute method of current action wasn't implemented!"

        def adapter(self, **kwargs): raise NotImplemented, "For some weird reason adapter method of current action wasn't implemented!"

        def enable(self):
                self.disabled = False

        def disable(self):
                self.disabled = True                
                

class ControlComponent:
        def __init__(self):pass
        def update(self): pass


class RangeComponent(ControlComponent):
        def __init__(self)
        def __call__(self        self.value = 0.0
                self.disabled = False
        def update(self, value):
                if self.disabled:
                        self.value = 0.0
                        return
                self.value = value
        def disable(self):
                self.disabled = True
        def enable(self):
                self.disabled = False
class RumblePad:
        DPAD_VERTICAL, DPAD_HORIZONTAL = RangeControl(), RangeControl()
        BUTTONS = list(StateControl(), StateControl(), StateControl(), StateControl(), StateControl(), StateControl())
        POV_L, POV_R = VectorControl(), VectorControl()
        
        def __init__(self):
                self.trigger = game.event_manager.getJoystick("Rumble")
                self.trigger.addListener(self.update)
                
        def update(self, evt):
                RumbleMap.DPAD_H.update(evt.axis_x)
                RumbleMap.DPAD_V.update(evt.axis_y)
                RumbleMap.POV_L.update(evt.pov[0])
                RumbleMap.POV_R.update(evt.pov[1])
                for index,bt in enumerate(RumbleMap.BUTTONS):
                        bt.update(evt.is_button_down(index))
                RumbleMap.BUTTONS


class CamContext:
        def __init__(self, devices):
                

class VectorControl(ControlComponent): pass
class QuaternionControl(ControlComponent): pass
class StateControl(ControlComponent): pass
class KEYWORDARGUMENTS:
        TIME = 'dtime'
        EVENT = 'event'
        

class RotateAction(Action):

        def __init__(self, targets=[], normal_speed=5.0, high_speed=10.0, axis=Vector3(0.0,0.0,0.0), reference=TS.LOCAL):
                super().__init__(targets)
                self.disabled = False
                self.selected_speed = normal_speed
                self.normal_speed = normal_speed
                self.high_speed = high_speed
                self.high_speed_enabled = False
                self.axis = axis.normalise()

        def toggle_high_speed(self):
                #This is done like this because otherwise the execute method is bloated with branches
                if self.high_speed_enabled:
                        self.selected_speed = self.high_speed
                else:
                        self.selected_speed = self.normal_speed

        def execute(self, **kwargs):
                if self.disabled:
                        return
                if kwargs['dtime']:
                        amount = kwargs['signal']*kwargs['dtime']*self.selected_speed
                else:
                        amount = kwargs['signal']*self.selected_speed
                adapter('amount'=amount)

        def adapter(self, **kwargs):
                #purpose of this function is pure convenience when
                #inheriting everything to TranslationAction or any other action class
                #which is nearly identical
                for target in self.targets:
                        target.rotate(Quaternion(kwargs['amount'], self.axis))


class ControllerContext:

        def __init__(actions=dict()):
                self.actions = actions
                self.disabled = False

        def add_action(self, name, action):
                self.actions.update(name, action)

        def remove_action(self, name):
                self.actions.remove(name)

        def update(self, **kwargs):
                if self.disabled:
                        return
                for name, action in self.actions:
                        if not action.disabled:
                                action.execute(kwargs)

        def disable(self):
                self.disabled = True

        def enable(self):
                self.disabled = False
                

class CameraControllerContext(ControllerContext):
        def __init__(self):
                super().__init__([TranslateAction(),
                                  RotateActions()])
                self.frame_trigger = game.event_manager.frame_trigger
                self.frame_trigger.addListener(__get_active_camera)
        def __update_active_camera(self, dtime):
                [game.player.camera_node]
                                  

class TranslateAction(RotateAction):
        def __init__(self, targets=[], normal_speed=5.0, high_speed=10.0, axis=Vector3(0.0,0.0,0.0), reference=TS.LOCAL):
                super().__init__(targets, normal_speed, high_speed, axis, reference)
        def adapter(self, target, amount):
                target.translate(amount*self.axis, self.reference)


                
class NodeActions(Actions):
        def __init__(self, targets=[],
        def rotate(self, targets=[], speed=0.0, axis=Vector3(0.0, 1.0, 0.0), reference=TS.LOCAL):
                for target in targets:
                        target.rotate(speed*axis,reference)

        def translate(self, targets=[], speed=0.0, axis=Vector3(0.0,0.0,-1.0), reference=TS.LOCAL):
                for target in targets:
                        targer.translate(speed*axis, reference)

        def set_orientation(self, targets=[], orientation=Quaternion(1.0,0.0,0.0,0.0)):
                for target in targets:
                        target.orientation = orientation


class DeviceMapComponent:
        def __init__(self, signals=[], actions=[]):
                self.signals = signals
                self.actions = actions
                
                                       
class Controller:
        def __init__(self, actions=[], device_maps=[]):
                self.actions = actions
                self.device_maps = device_maps
                self.mapper = dict()
                self.disabled = False
        def create_map(self):
                for dev in self.devices:
                        for actions
        def update(self, dtime):pass
        def progress(self, dtime):pass

class InputDevice:
        def __init__(self, vendor="", dev_id=0):
                self.vendor = vendor
                self.id = dev_id
                self.relative_ranges =list()
                self.abs_ranges = list()
                self.vectors = list()
                self.states = list()
                self.quaternions = list()
        def populate_ranges(self):pass
        def populate_vectors(self):pass
        def populate_quaternions(self): pass
        def populate_states(self):pass

                     
class RumblePad2(Device):
        def __init__(self):
                super().__init__("RumblePad2", 1)
                self.joystick_trigger = game.event_manager.getJoystick(self.vendor)
                self.joystick_trigger.addListener(self.update)
                self.button_count = 8
                self.axis_count = 2
                self.pov_count = 2
                self.quaternion_count = 0
        def update(self, evt):
                self.ranges[0] = evt.axis_x
                self.ranges[1] = evt.axis_y
                self.ranges[2] = evt.axis_z
                for bt in range(0,8):
                        self.states[bt] = evt.is_button_down(bt)
        def populate_ranges(self):
                self.ranges = [0.0, 0.0, 0.0, 0.0]
        def populate_vectors(self):
                self.vectors = [Vector3.zero, Vector3.zero]
        def populate_states(self):
                for bt in range(0,8):
                        self.states.append(False)
        
        

class ActionMapper:
    self.actions = dict()
    self.controllers = []
class ActionMap:
        def __init__(self, target): pass
        def transform(self, nodes, dtime): pass
        def translate(self, nodes, dtime): pass
        def rotate(self, nodes, dtime): pass
        def progress(self): pass
class ActiveCameraActionMap(ActionMap): pass
class ControlsDecorator(ActionMap):
        def __init__(self, controller):
                self.action_map = action_map
class KeyboardControls(ControlsDecorator): pass
class MouseControls(ControlsDecorator): pass
class JoystickControls(ControlsDecorator): pass
class TrackerControls(ControlsDecorator): pass



class ContextManager:
        def __init__(self):
                self.active_contexts = list()
                self.contexts = list()
        def add_context(self, ctx):
                self.contexts.append(ctx)
        def create_intent(self, intention):
                self.intents.append(intention)
                return intention

class CameraControllerDecorator:
        def __init__(self):
                
                
class IntentContext:
        def __init__(self, name, priority):
                self.name = name
                self.priority = priority
                self.intents = set()
        def add_intent(self, obj):
                self.intents.update(obj)
        def remove_intent(self, obj):                
                self.intents.remove(obj)
        def update(self, evt):
                for obj in self.listeners:
                        obj.update(evt)
        def __gt__(self, other):
                if self.priority > other.priority:
                     return False
                else:
                     return True
                
                
class Intent:
        def __init__(self, name):
                self.name = name
                self.listeners = 
        def add_listener(self, context, listener):
                self.listeners.update(context, listener)
        def remove_listener(self, listener):
class RotateAroundAxis                
        

class MoveInDirection(Intent):
        def __init__(self, objects):
                self.listeners = dict()
        def add_listener(self, context, listener):
                self.listeners.update(context=listener)
        def update_event(self, evt):
                
class IController:
        self.devices = []
        self.action_maps = dict()
        self.action_maps.update(DPAD_VERTICAL=self)
        
class Action:
        def __init__(self): pass
        def execute(self):pass

class TranslateAction(Action):
        def __init__(self, nodes=[], direction=Vector(0.0,0.0,-1.0), velocity=Vector3(1,1,1)):
                self.nodes = nodes
                self.velocity = velocity
                self.direction = direction
        def execute(self, signal_vector, dtime):

class RotateAction(Action):
        def __init__(self, nodes, axis, angular_velocity=Vector3(1,1,1)):
                self.nodes = nodes
                self.angular_velocity = angular_velocity
                self.axis = axis
        def execute(self, signal_vector, dtime):
                
class ActionMap(dict):
        def __init__(self):
                pass
        def update_values()

class Controller:
        self.nodes = []
        self.contexts
        self.action_maps
        self.device_maps
        def progress(self, dtime):
                pass
        def translate(self, dtime):
                pass
        def rotate(self, dtime):
                
class InputManager:
        def __init__(self):
                self.devices = dict()
                self.active_contexts = set()
                self.contexts = set()
        def add_context(self,ctx):
                self.contexts.update(ctx)
        def remove_context(self,ctx):
                self.contexts.difference_update(ctx)
        def activate_context(self, ctx):
                self.active_contexts.update(ctx)
        def deactivate_context(self, ctx):
                self.active_contexts.difference_update(ctx)

        

class Wand:
        def __init__(self, joystick_map = RazerMap, tracker_map):

                self.joystick = game.event_manager.getJoystick("default")
                self.tracker = TrackerTrigger("wand", "WandTrigger")
                self.tracker_map = dict(POSITION = None, DIRECTION = self.direction)
                self.joystick_map = joystick_map
                
        
class WandControllerMapper:

        def __init__(self, controller):
                self.controller = controller
                self.joystick = game.event_manager.getJoystick("default")
                self.tracker = TrackerTrigger("wand", "WandTrigger")
                #self.forward_axis = forward
                self.tracker.addListener(set_move_direction)
                self.joystick.addListener(progress)

        def progress(self, dtime):
		nodes = [game.player.camera_node]
                self.transform(nodes, dtime)

        def transform(self, nodes, dtime):
                pass
        def tracker_listener(self, trans):
                self.mov_dir = trans.quaternion*forward_axis
                self.mov_dir.normalise()
        def joystick_listener(self, evt):
                pass
                

camera = game.scene.getSceneNode("CameraNode")
camera.position = Vector3(0, 3, 15)
createCameraMovements(camera, speed=10)
# for the clouds we need to increase far clipping
cam = game.scene.getCamera("Omakamera")
cam.far_clip = 5e5

ogre = game.scene.getSceneNode("ogre")
addHideEvent(ogre, KC.H)
createSelectionController(speed=5, angular_speed=Degree(60), reference=camera)
key = game.event_manager.createKeyTrigger(KC.SPACE)
addTrackerMoveSelection("fingerTrigger", key)
ogre.position = Vector3(0, 2.5, 0)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("Omakamera", "OutsideCamera")

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
game.scene.shadows.max_distance = 50
game.scene.shadows.dir_light_extrusion_distance = 100
# SkyX does not work with reloading the scene
#game.scene.sky.preset = "sunset" #"clear"

ground = create_ground(size=100)

print("Create Sphere")
game.mesh_manager.createSphere('sphere')
sphere_ent = game.scene.createEntity('sphere', 'sphere', True)
sphere_ent.material_name = 'finger_sphere/red'
sphere = game.scene.createSceneNode('sphere')
sphere.attachObject(sphere_ent)
sphere.position = Vector3(4, 2.5, 0)
sphere_ent.cast_shadows = True

athene = game.scene.createSceneNode("athene")
# Testing the new Mesh Manager for loading meshes
athene_ent = game.scene.createEntity("athene", "athene.mesh", True)
athene_ent.material_name = "athene_material"
athene.attachObject(athene_ent)
athene.position = Vector3(-3, 4, 5)
athene.scale(0.05)

if game.scene.hasSceneNode("spot"):
	print("Hiding spot light")
	spot = game.scene.getLight("spot")
	spot.hide()

sun = create_sun()
sun.orientation = Quaternion(-0.900988, 0.43373, 0, 0)

# Create spotlight and hehkulamppu objects
spotti_n = game.scene.createSceneNode("spotti")
spotti = game.scene.createEntity("spotti", "spotlight.mesh", True)
spotti.material_name = "editor/spotlight_material"
spotti_n.position = Vector3(5, 4, 0)
spotti_n.attachObject(spotti)

lightpulp_n = game.scene.createSceneNode("hehkulamppu")
lightpulp = game.scene.createEntity("hehkulamppu", "light_pulp.mesh", True)
lightpulp.material_name = "editor/hehkulamppu_material"
lightpulp_n.position = Vector3(5, 4, -4)
lightpulp_n.attachObject(lightpulp)

plane = Plane(Vector3(0, 0, 1), 1)
m = buildReflectionMatrix(plane)

ogre.addChild(spotti_n)
ogre.addChild(lightpulp_n)

ogre2 = ogre.clone()
ogre2.translate(Vector3(0, 10, 0))

def printSpotti():
	Ts = spotti_n.world_transformation
	# Don't try to copy the object, it uses references by default
	iTs = spotti_n.world_transformation
	iTs.invert()
	# For some reason  inv(Ts)*Ts has better accuracy, but both of them
	# are pretty close to zero using this with arbitary transformations
	print('Ts = ', Ts, ' inv(Ts) = ', iTs, "Ts*inv(Ts) = ",
			Ts*iTs, "inv(Ts)*Ts = ", iTs*Ts)

text = game.scene.createMovableText('text', 'text')
text.font_name = "BlueHighway-22"
#text.font_name = "StarWars"
text.colour = ColourValue(0, 1.0, 0)
text.char_height = 0.4
text_n = game.scene.createSceneNode('text')
text_n.attachObject(text)
text_n.translate(Vector3(3, 2, 2))

def rotateSpotti(t):
	speed = Degree(10)
	angle = Radian(speed*Degree(float(t)))
	spotti_n.rotate(Quaternion(angle, Vector3(0, 1, 0)))

game.event_manager.frame_trigger.addListener(rotateSpotti)

def printAnalog(val):
	text.caption = "analog value = " + str(int(val))

# Test vrpn analog client
analog = game.create_analog_client("meh@localhost")
analog.n_sensors = 1
analog.get_sensor(0).addListener(printAnalog)

#ogre.rotate(Quaternion(0.7071, 0, -0.7071, 0))
#game.scene.addToSelection(lightpulp_n)

# Start with paused and toggle state with space
# TODO this is pretty useless because all events except for physics
# are still processed and simulation time is advanced
# Specifically
# Camera movements work
# Spot is rotated at each frame
# Sky is changing
game.auto_start = False
trigger = game.event_manager.createKeyTrigger(KC.SPACE)
trigger.addListener(toggle_pause)

def show_debug():
	#ogre.show_debug_display = True
	game.scene.show_axes(True)

"""
def timer_callback():
	print("Called from continuous timer")

def single_timer_callback():
	print("Called from single timer.")

# TODO
# Can not be created from callbacks because it messes the iterators
# TODO Counting the timers start before the scene has completely loaded
trigger = game.event_manager.createTimeTrigger()
trigger.interval = time(2, 0)
trigger.addListener(timer_callback)


trigger = game.event_manager.createTimeTrigger()
trigger.interval = time(10, 0)
trigger.continuous = False
trigger.addListener(single_timer_callback)
"""

ogre.translate(0, 1, 0)
trigger = game.event_manager.createKeyTrigger(KC.Y)
trigger.addListener(show_debug)

