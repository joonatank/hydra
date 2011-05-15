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
	trigger.action.add_action( trans_action )

def addBox(name, mat_name, position, size = Vector3(1,1,1), mass = 1) :
	print('Physics : Adding a box ' + name)
	box_node = game.scene.createSceneNode(name)
	box = game.scene.createEntity(name, PF.CUBE)
	box_node.attachObject(box)
	box.material_name = mat_name
	box_node.scale = size

	# TODO fix the size
	#box_shape = BoxShape.create(size)
	box_mesh = game.mesh_manager.getMesh('prefab_cube')
	box_shape = ConvexHullShape.create(box_mesh)
	box_shape.scale = size
	trans = Transform( position, Quaternion.identity)
	motion_state = world.createMotionState(trans, box_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	box = world.createRigidBody(name, mass, motion_state, box_shape, inertia)
	return box


def addSphere(name, mat_name, position, mass = 1) :
	print('Adding a sphere ' + name)
	sphere_node = game.scene.createSceneNode(name)
	# TODO this should be copied from the shape, using bounding boxes
	sphere_node.scale = Vector3(0.02, 0.02, 0.02)
	sphere = game.scene.createEntity(name, PF.SPHERE)
	sphere_node.attachObject(sphere)
	sphere.material_name = mat_name

	sphere_shape = SphereShape.create(1)
	trans = Transform( position, Quaternion.identity)
	motion_state = world.createMotionState(trans, sphere_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	sphere_body = world.createRigidBody(name, mass, motion_state, sphere_shape, inertia)

	# Set some damping so it doesn't go on endlessly
	sphere_body.setDamping(0.3, 0.3)
	return sphere_body

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
ground.cast_shadows = False

print('Physics : Adding ground plane')
ground_mesh = game.mesh_manager.loadMesh("prefab_plane")
ground_shape = StaticTriangleMeshShape.create(ground_mesh)
g_motion_state = world.createMotionState(Transform(Vector3(0, 0, 0)), ground_node)
world.createRigidBody('ground', 0, g_motion_state, ground_shape)

# TODO add some boxes
box1 = addBox("box1", "finger_sphere/blue", Vector3(5.0, 1, -5), mass=10)
# FIXME Mass 30 causes the box to go through the ground plane
# Also size 3 causes lots of accuracy problems in the collision detection
# size 2 causes less accuracy problems but still does, probably a problem
# with scaling a ConvexHull
box2 = addBox("box2", "finger_sphere/blue", Vector3(-5.0, 10, -5), size=Vector3(1, 1, 1), mass=20)

sphere_body = addSphere("sphere1", "finger_sphere/blue", Vector3(5.0, 20, 0), 10)
sphere_body.user_controlled = True


sphere_2_body = addSphere("sphere2", "finger_sphere/green", Vector3(-5.0, 5, 0), 0)

sphere_3_body = addSphere("sphere3", "finger_sphere/red", Vector3(3, 7, 3))
constraint = SliderConstraint.create(sphere_body, sphere_2_body, Transform(), Transform(), False)
constraint.lower_lin_limit = -5
constraint.upper_lin_limit = 5
#constraint.lower_ang_limit = 0
#constraint.upper_ang_limit = 1
#world.addConstraint(constraint)

six_dof = SixDofConstraint.create(sphere_body, sphere_2_body, Transform(), Transform(), False)
six_dof.setLinearLowerLimit(Vector3(-10, -5, -5))
six_dof.setLinearUpperLimit(Vector3(10, 5, 5))
#six_dof.setAngularLowerLimit(Vector3(1, 1, 1))
#six_dof.setAngularUpperLimit(Vector3(1, 1, 1))
#world.addConstraint(six_dof)

# Add force action
print('Adding Force action to KC_F')
action = ScriptAction.create()
action.game = game
action.script = 'sphere_body.applyForce(Vector3(0, 2500, 0), Vector3(0,0,0))'
trigger = game.event_manager.createKeyTrigger( KC.F )
trigger.action_down = action

# Add torque action
print('Adding Torque action to KC_G')
action = ScriptAction.create()
action.game = game
action.script = 'sphere_body.applyTorque(Vector3(0, 500, 0))'
trigger = game.event_manager.createKeyTrigger( KC.G )
trigger.action_down = action

print('Adding set Liner velocity action to KC_T')
action = ScriptAction.create()
action.game = game
action.script = 'sphere_body.setLinearVelocity(Vector3(1, 0, 0))'
trigger = game.event_manager.createKeyTrigger(KC.T)
trigger.action_down = action

print('Adding kinematic action')
addKinematicAction(sphere_body)

