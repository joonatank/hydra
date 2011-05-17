# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.

# TODO these are same in the physics_script move them to global physics_script
# which is included from project (not global)
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


print('Getting Camera SceneNode')
camera_name = "Camera"
if game.scene.hasSceneNode( camera_name ) :
	camera = game.scene.getSceneNode( camera_name )
	createCameraMovements(camera)
game.player.camera = camera_name

# Create physics
game.enablePhysics( True )
world = game.physics_world

# TODO this is not supported yet in 0.2.1
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

# TODO hides cb_:s automatically, needs to be a python command
# the support is there in the engine, don't hide them before we have real models
# game.scene.hideSceneNodes("cb_") if you need it

if( game.scene.hasSceneNode("cb_cylinder_actuator") ):
	cylinder = game.scene.getSceneNode("cb_cylinder_actuator")
	print(cylinder)

body_name = "cb_cylinder_actuator"
if( game.physics_world.hasRigidBody(body_name) ):
	print('Adding kinematic action to ', body_name)
	body = game.physics_world.getRigidBody(body_name)
#	addKinematicAction(body)
else :
	print('Physics world does not have body :', body_name)

sphere = addSphere("sphere1", "finger_sphere/blue", Vector3(5.0, 20, 0), 10)
sphere.user_controlled = True
addKinematicAction(sphere)
"""
print('Physics : Adding ogre')
if game.scene.hasSceneNode( ogre_name ):
	ogre = game.scene.getSceneNode(ogre_name)
	trans = Transform( Vector3(0, 20, 0), Quaternion.identity)
	motion_state = world.createMotionState( trans, ogre )
	ogre_phys = world.createRigidBody('ogre', ogre_mass, motion_state, sphere, Vector3(1,1,1))
	ogre_phys.setUserControlled()

	# Set some damping so it doesn't go on endlessly
	ogre_phys.setDamping(0.3, 0.3)

	# Add force action
	print('Adding Force action to KC_F')
#	action = ApplyForce.create()
	action = ScriptAction.create()
	action.game = game
	action.script = 'ogre_phys.applyForce(Vector3(0, 2500, 0), Vector3(0,0,0))'
	trigger = game.event_manager.createKeyPressedTrigger( KC.F )
	trigger.addAction( action )

	# Add torque action
	print('Adding Torque action to KC_G')
	#action = ApplyTorque.create()
	action = ScriptAction.create()
	action.game = game
	action.script = 'ogre_phys.applyTorque(Vector3(0, 500, 0))'
	trigger = game.event_manager.createKeyPressedTrigger( KC.G )
	trigger.addAction( action )

	print('Adding set Liner velocity action to KC_T')
	action = ScriptAction.create()
	action.game = game
	action.script = 'ogre_phys.setLinearVelocity(Vector3(1, 0, 0))'
	trigger = game.event_manager.createKeyPressedTrigger(KC.T)
	trigger.addAction( action )

	print('Adding kinematic action')
	addKinematicAction(ogre_phys)
"""

