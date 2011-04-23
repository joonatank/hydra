# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

def addKinematicAction(body):
	print( 'Creating Kinematic event on ' + body.name )

	# Create the translation action using a proxy
	trans_action_proxy = MoveActionProxy.create()
	trans_action_proxy.enableTranslation()
	addKeyActionsForAxis( trans_action_proxy, Vector3(1, 0, 0), KC.NUMPAD6, KC.NUMPAD4 )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 0, 1), KC.NUMPAD5, KC.NUMPAD8 )
	addKeyActionsForAxis( trans_action_proxy, Vector3(0, 1, 0), KC.NUMPAD9, KC.NUMPAD7 )

	# Create the rotation action using a proxy
	#rot_action_proxy = MoveActionProxy.create()
	#rot_action_proxy.enableRotation()
	# This is not useful, maybe using Q and E
	# TODO add rotation proxy, using a CTRL modifier

	# Create the real action
	trans_action = KinematicAction.create()
	trans_action.body = body
	trans_action.speed = 5
	trans_action.angular_speed = Radian(Degree(90))
	# Add the real action to the proxies
	trans_action_proxy.action = trans_action
	#rot_action_proxy.action = trans_action
	# TODO add rotation speed
	# Create a FrameTrigger and add the action to that
	trigger = game.event_manager.getFrameTrigger()
	trigger.addAction( trans_action )

print('Creating Camera SceneNode')
camera_n = game.scene.createSceneNode("Camera")
camera = game.scene.createCamera("Camera")
camera_n.attachObject(camera)
createCameraMovements(camera_n)
camera_n.position = Vector3(0, 5, 20)

game.player.camera = "Camera"

game.scene.sky = SkyDomeInfo("CloudySky")

# Create physics
# TODO these should be in the project file
game.enablePhysics( True )
world = game.physics_world

# Create light
spot = game.scene.createSceneNode("spot")
spot_l = game.scene.createLight("spot")
#spot_l.type = "spot"
spot.attachObject(spot_l)
spot.position = Vector3(0, 20, 0)
spot.orientation = Quaternion(0, 0, 0.7071, 0.7071)

ground_node = game.scene.createSceneNode("ground")
ground = game.scene.createEntity("ground", PF.PLANE)
ground_node.attachObject(ground)
ground.material_name = "ground/Basic"
ground_node.scale = ground_node.scale*0.2
ground.cast_shadows = False
ground_node.orientation = Quaternion(-0.7071, 0.7071, 0, 0)

print('Physics : Adding ground plane')
ground_shape = world.createBoxShape(Vector3(50, 1, 50))
g_motion_state = world.createMotionState()
world.createRigidBody('ground', 0, g_motion_state, ground_shape)

# TODO add some boxes

print('Physics : Adding sphere')
sphere_node = game.scene.createSceneNode("sphere")
# TODO this should be copied from the shape, using bounding boxes
sphere_node.scale = Vector3(0.03, 0.03, 0.03)
sphere = game.scene.createEntity("sphere", PF.SPHERE)
sphere_node.attachObject(sphere)
# TODO change a decent material
sphere.material_name = "debug_red"

sphere_shape = world.createSphereShape(0.1)
trans = Transform( Vector3(5.0, 20, 0), Quaternion.identity)
motion_state = world.createMotionState(trans, sphere_node)
inertia = Vector3(1,1,1)
sphere_body = world.createRigidBody('sphere', 10, motion_state, sphere_shape, inertia)
sphere_body.setUserControlled()

# Set some damping so it doesn't go on endlessly
sphere_body.setDamping(0.3, 0.3)

# Add force action
print('Adding Force action to KC_F')
action = ScriptAction.create()
action.game = game
action.script = 'sphere_body.applyForce(Vector3(0, 2500, 0), Vector3(0,0,0))'
trigger = game.event_manager.createKeyPressedTrigger( KC.F )
trigger.addAction( action )

# Add torque action
print('Adding Torque action to KC_G')
action = ScriptAction.create()
action.game = game
action.script = 'sphere_body.applyTorque(Vector3(0, 500, 0))'
trigger = game.event_manager.createKeyPressedTrigger( KC.G )
trigger.addAction( action )

print('Adding set Liner velocity action to KC_T')
action = ScriptAction.create()
action.game = game
action.script = 'sphere_body.setLinearVelocity(Vector3(1, 0, 0))'
trigger = game.event_manager.createKeyPressedTrigger(KC.T)
trigger.addAction( action )

print('Adding kinematic action')
addKinematicAction(sphere_body)

