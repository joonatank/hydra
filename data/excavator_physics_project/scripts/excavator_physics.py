# -*- coding: utf-8 -*-
# Create physics

PREFIX_RIGIDBODY = "rb_"
PREFIX_COLLISION = "cb_"

createCameraMovements(10)

def regRigidBody(scenenode,collision_mesh,mass,inertia):
        # Create collision shape of the objects collision mesh:
        cshape = ConvexHullShape.create(collision_mesh)
        # Now we must create a motionstate (initial conditions) for the rigidbody:
        mot_state = game.physics_world.createMotionState(scenenode.world_transformation,scenenode)
        # Now create the actual rigid body and register it into the system:
        body = game.physics_world.createRigidBody(PREFIX_RIGIDBODY+scenenode.name,mass,mot_state,cshape,inertia)
        body.user_controlled = True
        return body

def vec3(x,y,z):
        return Vector3(x,y,z)


def create6DofConstraint(bodyA,bodyB,jt_node,
                         linear_lower=vec3(0,0,0),linear_upper=vec3(0,0,0),
                         angular_lower=vec3(0,0,0),angular_upper=vec3(0,0,0),
                         disableCollision=True):
        jt_world_transformation = jt_node.world_transformation        
        localA_trans = bodyA.transform_to_local(jt_world_transformation)
        localB_trans = bodyB.transform_to_local(jt_world_transformation)

        constraint = PSixDofConstraint.create(bodyA, bodyB, localA_trans, localB_trans, False)

        constraint.setLinearLowerLimit(linear_lower)
        constraint.setLinearUpperLimit(linear_upper)
        constraint.setAngularLowerLimit(angular_lower)
        constraint.setAngularUpperLimit(angular_upper)

        game.physics_world.addConstraint(constraint, disableCollision)

        return constraint


game.enablePhysics( True )
world = game.physics_world

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()

create_sun()
ground = physics_create_ground()

game.scene.hideSceneNodes("cb_*")

# RIGID BODY PARAMETER TABLE:
# Basic structure:
# <dict>(<string>{object name}=<tuple>(<float>{mass},<vec3>{inertia}) )
# Example:
# params = {
#         'body1':(1000.0,vec3(0.1,0.5,0.40)),
#         'body2':(1200.0,vec3(0.2,0.4,0.40)) }
#
rigidbody_params = {
        'track_L':(1,vec3(1.0,1.0,1.0)),
        'track_R':(1,vec3(1.0,1.0,1.0)),
        'hull':(1,vec3(1.0,1.0,1.0)),
        'base':(1,vec3(1.0,1.0,1.0)),
        'boom1':(5.786,vec3(30.84,29.16,2.52)),
        'boom2':(1,vec3(1.0,1.0,1.0)),
        'bucket':(1,vec3(1.0,1.0,1.0)),
        '4barlink1':(1,vec3(1.0,1.0,1.0)),
        '4barlink2':(1,vec3(1.0,1.0,1.0)),
        'cyl_hull_R_rod':(1,vec3(1.0,1.0,1.0)),
        'cyl_hull_R_tube':(1,vec3(1.0,1.0,1.0)),
        'cyl_hull_L_rod':(1,vec3(1.0,1.0,1.0)),
        'cyl_hull_L_tube':(1,vec3(1.0,1.0,1.0)),
        'cyl_boom1_rod':(1,vec3(1.0,1.0,1.0)),
        'cyl_boom1_tube':(1,vec3(1.0,1.0,1.0)),
        'cyl_boom2_rod':(1,vec3(1.0,1.0,1.0)),
        'cyl_boom2_tube':(1,vec3(1.0,1.0,1.0))
        }


# We'll loop trough the rigidbody_params and create rigid bodies.
for objkey in rigidbody_params.keys():

        # Get scene node, check if it exists:
        snode = game.scene.getSceneNode(objkey)
        assert snode, "Object named: {0} does not exist!".format(objkey)

        # Get object collision mesh cb_{object_name}, if it exists:
        msh = game.mesh_manager.loadMesh(PREFIX_COLLISION + objkey)
        assert msh, "Mesh name does not correspond to object name cb_ + {0}!".format(objkey)

        # Get mass:
        mass = (rigidbody_params[objkey])[0]
        assert mass, "Mass wasn't defined for object: {0}".format(objkey)

        # Get inertia:
        inert = (rigidbody_params[objkey])[1]
        assert inert, "Inertia wasn't defined for object: {0}".format(objkey)

        # We'll register the object as rigidbody and create dynamically variables named as rb_{objectname}:
        #vars()[PREFIX_RIGIDBODY + objkey] = regRigidBody(snode,msh,mass,inert)
        regRigidBody(snode,msh,mass,inert)



# When upperlimit < lowerlimit: constraint dof will be totally free
# When upperlimit & lowerlimit == 0 constraint is locked
# FORMAT:
# format is nested tuple
# table = <tuple>( <tuple>(<string>{bodyA1},<string>{bodyB2},<string>{joint1},<Vector3>{LinearLowerLimit1},<Vector3>{LinearUpperLimit1},<Vector3>{AngularLowerLimit1},<Vector3>{AngularUpperLimit1},<bool>{disableCollision1}),
#                  <tuple>(<string>{bodyA2},<string>{bodyB2},<string>{joint2},<Vector3>{LinearLowerLimit2},<Vector3>{LinearUpperLimit2},<Vector3>{AngularLowerLimit2},<Vector3>{AngularUpperLimit2},<bool>{disableCollision2}) )

joint_table = (("track_L","base","jt_track_L-base_fix",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),True),
               ("track_R","base","jt_track_R-base_fix",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),True),
               ("base","hull","jt_base-hull_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("hull","boom1","jt_hull-boom1_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("hull","cyl_hull_R_tube","jt_cyl_hull_R_tube_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("cyl_hull_R_tube","cyl_hull_R_rod","jt_cyl_hull_R_tube-cyl_hull_R_rod_trans",vec3(0.0,0.0,-0.3),vec3(0.0,0.0,0.3),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),True),
               ("cyl_hull_R_rod","boom1","jt_cyl_hull_R_rod_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("hull","cyl_hull_L_tube","jt_cyl_hull_L_tube_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("cyl_hull_L_tube","cyl_hull_L_rod","jt_cyl_hull_L_tube-cyl_hull_L_rod_trans",vec3(0.0,0.0,-0.3),vec3(0.0,0.0,0.3),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),True),
               ("cyl_hull_L_rod","boom1","jt_cyl_hull_L_rod_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("boom1","boom2","jt_boom1-boom2_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("boom1","cyl_boom1_tube","jt_cyl_boom1_tube_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("cyl_boom1_tube","cyl_boom1_rod","jt_cyl_boom1_tube-cyl_boom1_rod_trans",vec3(0.0,0.0,-0.3),vec3(0.0,0.0,0.3),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),True),
               ("cyl_boom1_rod","boom2","jt_cyl_boom1_rod_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),               
               ("boom2","bucket","jt_boom2-bucket_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("boom2","4barlink1","jt_boom2-4barlink1_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("boom2","cyl_boom2_tube","jt_cyl_boom2_tube_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("cyl_boom2_tube","cyl_boom2_rod","jt_cyl_boom2_tube-cyl_boom2_rod_trans",vec3(0.0,0.0,-0.3),vec3(0.0,0.0,0.3),vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),True),
               ("cyl_boom2_rod","4barlink1","jt_cyl_boom2_rod_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True),
               ("4barlink2","bucket","jt_4barlink2-bucket_rot",vec3(0.0,0.0,0.0),vec3(0.0,0.0,0.0),vec3(1.0,0.0,0.0),vec3(-1.0,0.0,0.0),True)
               )

for jt in joint_table:
        # Collect data from joint table:
        bodyA = game.physics_world.getRigidBody(PREFIX_RIGIDBODY+jt[0])
        assert bodyA, "RigidBody named: {0} does not exist!".format(jt[0])
        bodyB = game.physics_world.getRigidBody(PREFIX_RIGIDBODY+jt[1])
        assert bodyA, "RigidBody named: {0} does not exist!".format(jt[1])
        joint_node = game.scene.getSceneNode(jt[2])
        assert joint_node, "Object named: {0} does not exist!".format(jt[2])
        lin_low = jt[3]
        lin_up = jt[4]
        ang_low = jt[5]
        ang_up = jt[6]
        disableCollision = jt[7]
        # Create constraint/joint:
        #vars()[(joint_node.name).replace('-','_')] = create6DofConstraint(bodyA,bodyB,joint_node,lin_low,lin_up,ang_low,ang_up,disableCollision)
        create6DofConstraint(bodyA,bodyB,joint_node,lin_low,lin_up,ang_low,ang_up,disableCollision)
