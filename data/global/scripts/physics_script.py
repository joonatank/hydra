# -*- coding: utf-8 -*-

# Common functions for defining physics actions and constraints

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

def addDynamicAction(body, reference = None):
	print('Creating Dynamic event on ' + body.name)

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
	trans_action = DynamicAction.create()
	trans_action.body = body
	trans_action.reference = reference
	trans_action.local = False
	trans_action.max_speed = 3
	# Relative to the mass, so we get nice acceleration and add some force
	# to win gravity
	trans_action.force = Vector3(10, 10, 10)*body.mass + Vector3(0, 10, 0)
	# No torque because no rotations
	#trans_action.torque = Vector3(10, 100, 100)
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
	motion_state = game.physics_world.createMotionState(trans, box_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	box = game.physics_world.createRigidBody(name, mass, motion_state, box_shape, inertia)
	return box


def addSphere(name, mat_name, position, mass = 1, size = 1) :
	print('Adding a sphere ' + name)
	sphere_node = game.scene.createSceneNode(name)
	# TODO this should be copied from the shape, using bounding boxes
	sphere_node.scale = Vector3(0.02, 0.02, 0.02)*size
	sphere = game.scene.createEntity(name, PF.SPHERE)
	sphere_node.attachObject(sphere)
	sphere.material_name = mat_name

	sphere_shape = SphereShape.create(size)
	trans = Transform( position, Quaternion.identity)
	motion_state = game.physics_world.createMotionState(trans, sphere_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	body = game.physics_world.createRigidBody(name, mass, motion_state, sphere_shape, inertia)

	# Set some damping so it doesn't go on endlessly
	body.setDamping(0.3, 0.3)
	return body 

def createFixedConstraint(body0, body1, transform, disableCollision = True):
	local0_trans = body0.transform_to_local(transform)
	local1_trans = body1.transform_to_local(transform)
	constraint = PSixDofConstraint.create(body0, body1, local0_trans, local1_trans, False)
	constraint.setLinearLowerLimit(Vector3(0, 0, 0))
	constraint.setLinearUpperLimit(Vector3(0, 0, 0))
	constraint.setAngularLowerLimit(Vector3(0, 0, 0))
	constraint.setAngularUpperLimit(Vector3(0, 0, 0))
	game.physics_world.addConstraint(constraint, disableCollision)
	return constraint

