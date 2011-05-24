# -*- coding: utf-8 -*-

# Project specific definitions, like object creations

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

