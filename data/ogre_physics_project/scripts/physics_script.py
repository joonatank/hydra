# -*- coding: utf-8 -*-

# Most of the functions are in the global config now, script global_script
# Easy to define commonly used functions in there
# Here we can use those functions and pass the scene related objects to them
#
# Global scripts are always processed first
# Other than that order of script processing is not guaranteed.
print('Python ogre physics script')

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
	ogre_phys = world.createRigidBody( 'ogre', ogre_mass, motion_state, sphere, Vector3(1,1,1), True )

	# Add force action
	print('Adding Force action to KC_F')
	action = ApplyForce.create()
	action.body = ogre_phys
	action.force = Vector3(0, 2500, 0)
	trigger = game.event_manager.createKeyPressedTrigger( KC.F )
	trigger.addAction( action )

	# Add torque action
	print('Adding Torque action to KC_G')
	action = ApplyTorque.create()
	action.body = ogre_phys
	action.torque = Vector3(0, 2000, 0)
	trigger = game.event_manager.createKeyPressedTrigger( KC.G )
	trigger.addAction( action )

