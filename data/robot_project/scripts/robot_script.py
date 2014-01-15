# -*- coding: utf-8 -*-

camera_name = "Camera"
create_camera_controller()
game.player.camera = camera_name

# Create physics
game.enablePhysics( True )
world = game.physics_world

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.max_distance = 50
game.scene.shadows.dir_light_extrusion_distance = 100
game.scene.shadows.texture_size = 4096

create_sun()
physics_create_ground()

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
physics_fixed_constraint(link0_body, link1_body, transform)

transform = link1_body.world_transformation
physics_fixed_constraint(link1_body, cylinder_actuator_body, transform)

transform = cylinder_piston_body.world_transformation
physics_fixed_constraint(cylinder_actuator_body, cylinder_piston_body, transform)

transform = cylinder_piston_body.world_transformation
physics_fixed_constraint(cylinder_piston_body, link2_body, transform)

transform = link2_body.world_transformation
transform.position += Vector3(0, 0.5, 0)
physics_fixed_constraint(link2_body, link3_body, transform)

transform = link3_body.world_transformation
transform.position += Vector3(-0.25, 0, 0)
physics_fixed_constraint(link3_body, link4_body, transform)

sphere = addSphere("sphere1", "finger_sphere/blue", Vector3(5.0, 20, 0))
sphere.user_controlled = True
# Rigid body controller doesn't work because RigidBody doesn't have rotate method
#addRigidBodyController(sphere)

