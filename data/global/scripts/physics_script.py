# -*- coding: utf-8 -*-

# Common functions for defining physics actions and constraints

# TODO split collision detection stuff to another script and rigid body
# controllers to another

# TODO rewrite the actions using GameJoysticks and listener classes
# Action system is going to be removed in next release (0.4)

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

# TODO add an example of the use of models both static and
# dynamic
# we can generate the model for example a torus using mesh manager
# and then create both static version of it and separate convex hull version
# these are for testing the collision detection with mesh types

def addBox(name, mat_name, position, size = Vector3(1,1,1), mass = 1) :
	print('Physics : Adding a box ' + name)
	box_node = game.scene.createSceneNode(name)
	mesh_name = name+"_box_mesh"
	game.mesh_manager.createCube(mesh_name, size)
	box = game.scene.createEntity(name, mesh_name, True)
	box_node.attachObject(box)
	box.material_name = mat_name
	box_node.scale = size

	box_shape = BoxShape.create(size)
	trans = Transform( position, Quaternion.identity)
	motion_state = game.physics_world.createMotionState(trans, box_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	box = game.physics_world.createRigidBody(name, mass, motion_state, box_shape, inertia)
	return box


def addSphere(name, mat_name, position, mass = 1, radius = 0.5) :
	print('Adding a sphere ' + name)
	sphere_node = game.scene.createSceneNode(name)
	# Uses 1m for the size of the graphics object we should move to using
	# MeshManager directly so we can specify the radius of the sphere
	mesh_name = name+"_mesh"
	game.mesh_manager.createSphere(mesh_name, radius, 8, 8)
	sphere = game.scene.createEntity(name, mesh_name, True)
	sphere_node.attachObject(sphere)
	sphere.material_name = mat_name

	sphere_shape = SphereShape.create(radius)
	trans = Transform( position, Quaternion.identity)
	motion_state = game.physics_world.createMotionState(trans, sphere_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	body = game.physics_world.createRigidBody(name, mass, motion_state, sphere_shape, inertia)

	# Set some damping so it doesn't go on endlessly
	body.setDamping(0.3, 0.3)
	return body 

def addCylinder(name, mat_name, position, mass=1, radius=0.5, height=1) :
	print('Adding a cylinder ' + name)
	cylinder_node = game.scene.createSceneNode(name)

	mesh_name = name + "_mesh"
	game.mesh_manager.createCylinder(mesh_name, radius, height)
	cylinder = game.scene.createEntity(name, mesh_name, True)
	cylinder_node.attachObject(cylinder)
	cylinder.material_name = mat_name

	cylinder_shape = CylinderShape.create(radius, height)
	trans = Transform( position, Quaternion.identity)
	motion_state = game.physics_world.createMotionState(trans, cylinder_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	body = game.physics_world.createRigidBody(name, mass, motion_state, cylinder_shape, inertia)

	# Set some damping so it doesn't go on endlessly
	body.setDamping(0.3, 0.3)
	return body 

def addCapsule(name, mat_name, position, mass=1, radius=0.5, height=1) :
	print('Adding a capsule ' + name)
	capsule_node = game.scene.createSceneNode(name)

	mesh_name = name + "_mesh"
	game.mesh_manager.createCapsule(mesh_name, radius, height)
	capsule = game.scene.createEntity(name, mesh_name, True)
	capsule_node.attachObject(capsule)
	capsule.material_name = mat_name

	capsule_shape = CapsuleShape.create(radius, height)
	trans = Transform( position, Quaternion.identity)
	motion_state = game.physics_world.createMotionState(trans, capsule_node)
	inertia = Vector3.zero
	if(mass != 0) :
		inertia = Vector3(1,1,1)
	body = game.physics_world.createRigidBody(name, mass, motion_state, capsule_shape, inertia)

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


# Create a translation joint between two bodies along z-axis
# min and max defines the freedom of the joint in meters
# Depending on wether the original coordinates have positive or negative
# z forward, you need to use positive or negative velocity for driving
# positive z forward positive velocity
def createTranslationConstraint(body0, body1, transform, min, max, disableCollision = True) :
	# SliderConstraint works on x-axis, so we rotate the reference
	# z-axis to x-axis
	trans = transform*Transform(Quaternion(-0.7071, 0, 0, 0.7071))
	local0_trans = body0.transform_to_local(trans)
	local1_trans = body1.transform_to_local(trans)
	constraint = PSliderConstraint.create(body0, body1, local0_trans, local1_trans, False)
	constraint.lower_lin_limit = min
	constraint.upper_lin_limit = max
	constraint.lower_ang_limit = 0
	constraint.upper_ang_limit = 0
	game.physics_world.addConstraint(constraint, disableCollision)
	return constraint

# Define a hinge constraint using objects +y axis
# +y because the objects were modeled in Blender with +z as the rotation axis
# but the exporter flips y and z
def createHingeConstraint(body0, body1, transform, disableCollision = True, min = Degree(0), max = Degree(0)) :
	trans = transform*Transform(Quaternion(0.7071, 0.7071, 0, 0))
	local0_trans = body0.transform_to_local(trans)
	local1_trans = body1.transform_to_local(trans)
	constraint = PHingeConstraint.create(body0, body1, local0_trans, local1_trans, False)
	constraint.set_limit(Radian(min), Radian(max))
	game.physics_world.addConstraint(constraint, disableCollision)
	return constraint

