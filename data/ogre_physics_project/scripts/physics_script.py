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


# config.getSceneNode gets a reference to already created SceneNode
# For now it's not possible to create SceneNodes from python
# So use this function to get a SceneNode created from .scene file.
# All SceneNodes in .scene file are created and can be retrieved here.
ogre_name = 'ogre'
ogre_mass = 10

if game.scene.hasSceneNode( ogre_name ):
	ogre = game.scene.getSceneNode(ogre_name)
	addHideEvent(ogre, KC.H)
	# Can not be run as not defines
	#addOgreRotations(ogre)

print('Getting Camera SceneNode')
camera_name = "CameraNode"
if game.scene.hasSceneNode( camera_name ) :
	camera = game.scene.getSceneNode( camera_name )
	createCameraMovements(camera)

# ActiveCamera toggle, supports two cameras. Parameters passed are camera names
# first one is the camera not active at the moment, second one is active at the moment
addToggleActiveCamera("OutsideCamera", "Omakamera" )

# Create physics
game.enablePhysics( True )
world = game.physics_world

print('Physics : Adding ground plane')
plane = world.createPlaneShape( Vector3.unit_y, 1 )
sphere = world.createSphereShape( 0.3 )
trans = Transform(-Vector3.unit_y, Quaternion.identity)
motion_state = world.createMotionState( trans )
world.createRigidBody( 'ground', 0, motion_state, plane )

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
