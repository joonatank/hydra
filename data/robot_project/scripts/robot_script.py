# -*- coding: utf-8 -*-

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

body_name = "cb_link0"
link0_body = game.physics_world.getRigidBody(body_name)
body_name = "cb_link1"
link1_body = game.physics_world.getRigidBody(body_name)
body_name = "cb_link2"
link2_body = game.physics_world.getRigidBody(body_name)
body_name = "cb_link3"
link3_body = game.physics_world.getRigidBody(body_name)
body_name = "cb_link4"
link4_body = game.physics_world.getRigidBody(body_name)
body_name = "cb_cylinder_actuator"
cylinder_actuator_body = game.physics_world.getRigidBody(body_name)
body_name = "cb_cylinder_pistonrod"
cylinder_piston_body = game.physics_world.getRigidBody(body_name)

transform = link0_body.world_transformation
transform.position += Vector3(0, -0.5, 0)
createFixedConstraint(link0_body, link1_body, transform)

transform = link1_body.world_transformation
createFixedConstraint(link1_body, cylinder_actuator_body, transform)

transform = cylinder_piston_body.world_transformation
createFixedConstraint(cylinder_actuator_body, cylinder_piston_body, transform)

transform = cylinder_piston_body.world_transformation
createFixedConstraint(cylinder_piston_body, link2_body, transform)

transform = link2_body.world_transformation
transform.position += Vector3(0, 0.5, 0)
createFixedConstraint(link2_body, link3_body, transform)

transform = link3_body.world_transformation
transform.position += Vector3(-0.25, 0, 0)
createFixedConstraint(link3_body, link4_body, transform)

cylinder_actuator_body.user_controlled = True
addKinematicAction(cylinder_actuator_body)

sphere = addSphere("sphere1", "finger_sphere/blue", Vector3(5.0, 20, 0))
#sphere.user_controlled = True
#addKinematicAction(sphere)
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

