# -*- coding: utf-8 -*-
# Create physics

PREFIX_RIGIDBODY = "rb_"
PREFIX_COLLISION = "cb_"
def vec3(x,y,z):
        return Vector3(x,y,z)
class ControlledCylinderMotor:
        def __init__(self,slider=None,maxmotorforce=300.0,maxspeed=0.1,key1=None,key2=None):
                assert(slider)
                assert(key1)
                assert(key2)
                self.motor = slider.constraint.translation_motor
                assert(self.motor)
                self.maxspeed=maxspeed
                self.axis = slider.axis
                self.axisvec = slider.axisopts[self.axis]
                self.axisnumber = slider.axisnumbers[self.axis]
                self.motor.max_motor_force = maxmotorforce*self.axisvec
                self.motor.target_velocity = Vector3(0.0,0.0,0.0)
                self.motor.restitution = 0.0
                self.motor.normal_CFM = Vector3(0.1,0.1,0.1)
                self.motor.stop_ERP = Vector3(0.8,0.8,0.8)
                self.motor.stop_CFM = Vector3(0.1,0.1,0.1)
                self.motor.limit_softness = 0.0
                self.motor.enableMotor(self.axisnumber)
                self.control = game.event_manager.createKeyTrigger(key1)
                assert(self.control)
                self.control.addListener(self.subtract)
                self.control = game.event_manager.createKeyTrigger(key2)
                assert(self.control)
                self.control.addListener(self.extend)
        def extend(self):
                self.motor.target_velocity = self.maxspeed*self.axisvec
        def subtract(self):
                self.motor.target_velocity = -self.maxspeed*self.axisvec
class ControlledTorqueMotor:
        def __init__(self,hinge=None,maxmotorforce=300.0,maxlimittorque=600.0,maxspeed=0.1,key1=None,key2=None):
                assert(hinge)
                assert(key1)
                assert(key2)
                self.motor = hinge.constraint.rotation_motor
                assert(self.motor)
                self.maxspeed=maxspeed
                self.axis = hinge.axis
                self.axisvec = hinge.axisopts[self.axis]
                self.axisnumber = hinge.axisnumbers[self.axis]
                self.motor.max_motor_force = maxmotorforce*self.axisvec
                self.motor.max_limit_torque = maxlimittorque*self.axisvec
                self.motor.target_velocity = Vector3(0.0,0.0,0.0)
                self.motor.restitution = 0.0
                self.motor.normal_CFM = Vector3(0.1,0.1,0.1)
                self.motor.stop_ERP = Vector3(0.8,0.8,0.8)
                self.motor.stop_CFM = Vector3(0.1,0.1,0.1)
                self.motor.limit_softness = 0.0                
                self.motor.enableMotor(self.axisnumber)
                self.control = game.event_manager.createKeyTrigger(key1)
                assert(self.control)
                self.control.addListener(self.rotatepositive)
                self.control = game.event_manager.createKeyTrigger(key2)
                assert(self.control)
                self.control.addListener(self.rotatenegative)
        def rotatepositive(self):
                self.motor.target_velocity = self.maxspeed*self.axisvec
        def rotatenegative(self):
                self.motor.target_velocity = -self.maxspeed*self.axisvec
class RigidBody:
        def __init__(self,name=None,mass=1.0,inertia=Vector3(0.0,0.0,0.0),controlled=True):
                # Get scene node, check if it exists:
                self.snode = game.scene.getSceneNode(name)
                assert self.snode, "Object named: {0} does not exist!".format(name)
                # Get object collision mesh cb_{object_name}, if it exists:
                self.msh = game.mesh_manager.loadMesh("cb_" + name)
                assert self.msh, "Mesh name does not correspond to object name {0} + {1}!".format(PREFIX_COLLISION,name)
                self.cshape = ConvexHullShape.create(self.msh)
                self.motion_state = game.physics_world.createMotionState(self.snode.world_transformation,self.snode)
                assert(self.motion_state)
                self.body = game.physics_world.createRigidBody(name, mass, self.motion_state, self.cshape, inertia)
                assert(self.body)
                self.body.user_controlled = controlled
                self.body.linear_damping = 0.0
                self.body.angular_damping = 0.0
                self.body.kinematic = False
                
class Hinge:
        def __init__(self,bA,bB,jt,lowerlimit=1.0,upperlimit=-1.0,axis='X',disableCollision=True):
                assert(isinstance(bA,RigidBody))
                assert(isinstance(bB,RigidBody))
                localA_trans = bA.body.transform_to_local(jt.world_transformation)
                localB_trans = bB.body.transform_to_local(jt.world_transformation)
                self.constraint = PSixDofConstraint.create(bA.body, bB.body, localA_trans, localB_trans, False)
                assert(self.constraint)
                self.constraint.setLinearLowerLimit(Vector3(0.0,0.0,0.0))
                self.constraint.setLinearUpperLimit(Vector3(0.0,0.0,0.0))
                self.axis = axis
                self.axisopts = dict(X=Vector3(1.0,0.0,0.0),
                                     Y=Vector3(0.0,1.0,0.0),
                                     Z=Vector3(0.0,0.0,1.0))
                self.axisnumbers = dict(X=0,Y=1,Z=2)
                self.constraint.setAngularLowerLimit(lowerlimit*self.axisopts[self.axis])
                self.constraint.setAngularUpperLimit(upperlimit*self.axisopts[self.axis])
                game.physics_world.addConstraint(self.constraint, disableCollision)
class Slider:
        def __init__(self,bA,bB,jt,lowerlimit=1.0,upperlimit=-1.0,axis='Z',disableCollision=True):
                assert(isinstance(bA,RigidBody))
                assert(isinstance(bB,RigidBody))
                localA_trans = bA.body.transform_to_local(jt.world_transformation)
                localB_trans = bB.body.transform_to_local(jt.world_transformation)
                self.constraint = PSixDofConstraint.create(bA.body, bB.body, localA_trans, localB_trans, False)
                assert(self.constraint)
                self.constraint.setAngularLowerLimit(Vector3(0.0,0.0,0.0))
                self.constraint.setAngularUpperLimit(Vector3(0.0,0.0,0.0))
                self.axis=axis
                self.axisopts = dict(X=Vector3(1.0,0.0,0.0),
                                     Y=Vector3(0.0,1.0,0.0),
                                     Z=Vector3(0.0,0.0,1.0))
                self.axisnumbers = dict(X=0,Y=1,Z=2)
                self.constraint.setLinearLowerLimit(lowerlimit*self.axisopts[self.axis])
                self.constraint.setLinearUpperLimit(upperlimit*self.axisopts[self.axis])
                game.physics_world.addConstraint(self.constraint, disableCollision)
class Fixed:
        def __init__(self,bA,bB,jt,disableCollision=True):
                assert(isinstance(bA,RigidBody))
                assert(isinstance(bB,RigidBody))
                localA_trans = bA.body.transform_to_local(jt.world_transformation)
                localB_trans = bB.body.transform_to_local(jt.world_transformation)
                self.constraint = PSixDofConstraint.create(bA.body, bB.body, localA_trans, localB_trans, False)
                assert(self.constraint)
                self.constraint.setAngularLowerLimit(Vector3(0.0,0.0,0.0))
                self.constraint.setAngularUpperLimit(Vector3(0.0,0.0,0.0))
                self.constraint.setLinearLowerLimit(Vector3(0.0,0.0,0.0))
                self.constraint.setLinearUpperLimit(Vector3(0.0,0.0,0.0))
                game.physics_world.addConstraint(self.constraint, disableCollision)
def initphysics():        
        createCameraMovements(10)               
        game.enablePhysics( True )
        #CUSTOM SOLVER PARAMETERS:
        solverparams = PhysicsSolverParameters() 
        #Joint normal error reduction parameter (slow(0.0) - fast(1.0)):
        solverparams.erp = 0.8
        #Contact error reduction parameter (slow(0.0) - fast(1.0)) : 
        solverparams.erp2 = 0.8
        #Constraint force mixing parameter, when set to 0.0 constraints will be hard:
        solverparams.global_cfm = 0.0
        #On collisions how much the momentum is conserved: (totally_inelastic(0.0) - totally_elastic(1.0))
        solverparams.restitution = 0.0
        #I have no idea of this, so setting it to default:
        solverparams.max_error_reduction = game.physics_world.solver_parameters.max_error_reduction
        #Internal time step (1/fps)
        solverparams.internal_time_step = 1/480
        #No idea what substep, has it something to do with interpolation, well higher value should be more precise but wastes much more computing power?
        solverparams.max_sub_steps = 20
        #Now we set the solver parameters to current physics world:
        game.physics_world.solver_parameters = solverparams
        #We hide all the collision objects:
        game.scene.hideSceneNodes(PREFIX_COLLISION+"*")
        return game.physics_world
world = initphysics()
game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
create_sun()
ground = physics_create_ground()
#ground.disable_collisions = True
# RIGID BODY PARAMETER TABLE:
rigidbody_dict = dict(track_L=(11.1,Vector3(34.02,33.37,3.26)),
                      track_R=(11.1,Vector3(34.02,33.37,3.26)),
                      hull=(23.04,Vector3(43.63,67.97,41.43)),
                      base=(7.96,Vector3(6.65,10.62,6.35)),
                      boom1=(2.786,Vector3(30.84,29.16,2.52)),
                      boom2=(1.045,Vector3(6.0,0.61,5.58)),
                      bucket=(2.13,Vector3(1.74,1.78,1.22)),
                      fourbarlink1=(0.1,Vector3(0.05,0.03,0.05)),
                      fourbarlink2=(0.1,Vector3(0.05,0.03,0.05)),
                      cyl_hull_R_rod=(0.22,Vector3(0.07,0.002,0.07)),
                      cyl_hull_R_tube=(0.33,Vector3(0.11,0.01,0.11)),
                      cyl_hull_L_rod=(0.22,Vector3(0.07,0.002,0.07)),
                      cyl_hull_L_tube=(0.33,Vector3(0.11,0.01,0.11)),
                      cyl_boom1_rod=(0.305,Vector3(0.12,0.003,0.12)),
                      cyl_boom1_tube=(0.46,Vector3(0.18,0.014,0.18)),
                      cyl_boom2_rod=(0.19,Vector3(0.04,0.002,0.04)),
                      cyl_boom2_tube=(0.285,Vector3(0.06,0.01,0.06))
                      )
OIKEErigidbody_dict = dict(track_L=(11.1,Vector3(34.02,33.37,3.26)),
                      track_R=(11.1,Vector3(34.02,33.37,3.26)),
                      hull=(23.04,Vector3(43.63,67.97,41.43)),
                      base=(7.96,Vector3(6.65,10.62,6.35)),
                      boom1=(5.786,Vector3(30.84,29.16,2.52)),
                      boom2=(3.045,Vector3(6.0,0.61,5.58)),
                      bucket=(3.13,Vector3(1.74,1.78,1.22)),
                      fourbarlink1=(0.1,Vector3(0.05,0.03,0.05)),
                      fourbarlink2=(0.1,Vector3(0.05,0.03,0.05)),
                      cyl_hull_R_rod=(0.22,Vector3(0.07,0.002,0.07)),
                      cyl_hull_R_tube=(0.33,Vector3(0.11,0.01,0.11)),
                      cyl_hull_L_rod=(0.22,Vector3(0.07,0.002,0.07)),
                      cyl_hull_L_tube=(0.33,Vector3(0.11,0.01,0.11)),
                      cyl_boom1_rod=(0.305,Vector3(0.12,0.003,0.12)),
                      cyl_boom1_tube=(0.46,Vector3(0.18,0.014,0.18)),
                      cyl_boom2_rod=(0.19,Vector3(0.04,0.002,0.04)),
                      cyl_boom2_tube=(0.285,Vector3(0.06,0.01,0.06))
                      )
# We'll loop trough the rigidbody_params and create rigid bodies.
rigidbodies = dict()
for name in rigidbody_dict:
        # Get mass:
        mass = rigidbody_dict[name][0]
        assert mass, "Mass wasn't defined for object: {0}".format(name)
        # Get inertia:
        inert = rigidbody_dict[name][1]
        assert inert, "Inertia wasn't defined for object: {0}".format(name)
        rigidbodies[name] = RigidBody(name,mass,inert,True)
#Disable collision from few rigidbodies:
rigidbodies['fourbarlink1'].body.disable_collisions = True
rigidbodies['fourbarlink2'].body.disable_collisions = True
rigidbodies['cyl_boom2_rod'].body.disable_collisions = True
rigidbodies['cyl_hull_R_rod'].body.disable_collisions = True
rigidbodies['cyl_hull_R_tube'].body.disable_collisions = True
rigidbodies['cyl_hull_L_rod'].body.disable_collisions = True
rigidbodies['cyl_hull_L_tube'].body.disable_collisions = True
rigidbodies['cyl_boom1_rod'].body.disable_collisions = True
rigidbodies['cyl_boom1_tube'].body.disable_collisions = True
rigidbodies['cyl_boom2_rod'].body.disable_collisions = True
rigidbodies['cyl_boom2_tube'].body.disable_collisions = True
#rigidbodies['track_L'].body.disable_collisions = True
#rigidbodies['track_R'].body.disable_collisions = True
rigidbodies['hull'].body.disable_collisions = True
rigidbodies['base'].body.disable_collisions = True
#rigidbodies['boom1'].body.disable_collisions = True
#rigidbodies['boom2'].body.disable_collisions = True
#rigidbodies['bucket'].body.disable_collisions = True
# When upperlimit < lowerlimit: constraint dof will be totally free
# When upperlimit & lowerlimit == 0 constraint is locked
# FORMAT:
#1.34
joint_dict = dict(jt_track_L_to_base_fix=("track_L","base","Fixed",'X',0,0,True),
                  jt_track_R_to_base_fix=("track_R","base","Fixed",'X',0,0,True),
                  jt_base_to_hull_rot=("base","hull","Hinge",'X',1.0,-1.0,True),
                  jt_hull_to_boom1_rot=("hull","boom1","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_hull_R_tube_rot=("hull","cyl_hull_R_tube","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_hull_R_trans=("cyl_hull_R_tube","cyl_hull_R_rod","Slider",'Z',-0.20,0.60,True),
                  jt_cyl_hull_R_rod_rot=("cyl_hull_R_rod","boom1","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_hull_L_tube_rot=("hull","cyl_hull_L_tube","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_hull_L_trans=("cyl_hull_L_tube","cyl_hull_L_rod","Slider",'Z',-0.20,0.60,True),
                  jt_cyl_hull_L_rod_rot=("cyl_hull_L_rod","boom1","Hinge",'X',1.0,-1.0,True),
                  jt_boom1_to_boom2_rot=("boom1","boom2","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_boom1_tube_rot=("boom1","cyl_boom1_tube","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_boom1_trans=("cyl_boom1_tube","cyl_boom1_rod","Slider",'Z',-1.2,0.35,True),
                  jt_cyl_boom1_rod_rot=("cyl_boom1_rod","boom2","Hinge",'X',1.0,-1.0,True),
                  jt_boom2_to_bucket_rot=("boom2","bucket","Hinge",'X',1.0,-1.0,True),
                  jt_boom2_to_fourbarlink1_rot=("boom2","fourbarlink1","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_boom2_tube_rot=("boom2","cyl_boom2_tube","Hinge",'X',1.0,-1.0,True),
                  jt_cyl_boom2_trans=("cyl_boom2_tube","cyl_boom2_rod","Slider",'Z',-1.40,0.05,True),
                  jt_cyl_boom2_rod_rot=("cyl_boom2_rod","fourbarlink1","Hinge",'X',1.0,-1.0,True),
                  jt_fourbarlink2_to_bucket_rot=("fourbarlink2","bucket","Hinge",'X',1.0,-1.0,True),
                  jt_fourbarlink1_to_fourbarlink2_rot=("fourbarlink1","fourbarlink2","Hinge",'X',1.0,-1.0,True)
                  )   
constraints = dict()
for jt in joint_dict:
        # Collect data from joint table:
        bodyA = rigidbodies[joint_dict[jt][0]]
        #game.physics_world.getRigidBody(PREFIX_RIGIDBODY+jt[0])
        assert bodyA, "RigidBody named: {0} does not exist!".format(joint_dict[jt][0])
        bodyB = rigidbodies[joint_dict[jt][1]]
        #game.physics_world.getRigidBody(PREFIX_RIGIDBODY+jt[1])
        assert bodyB, "RigidBody named: {0} does not exist!".format(joint_dict[jt][1])
        joint_pos_node = game.scene.getSceneNode(jt)
        assert joint_pos_node, "Object named: {0} does not exist!".format(jt)
        joint_type = joint_dict[jt][2]
        axis = joint_dict[jt][3]
        low = joint_dict[jt][4]
        hi = joint_dict[jt][5]
        disableCollision = joint_dict[jt][6]
        if joint_type is "Hinge":
                constraints[jt] = Hinge(bodyA,bodyB,joint_pos_node,low,hi,axis,disableCollision)
                assert(constraints[jt])
        elif joint_type is "Slider":
                constraints[jt] = Slider(bodyA,bodyB,joint_pos_node,low,hi,axis,disableCollision)
                assert(constraints[jt])
        elif joint_type is "Fixed":
                constraints[jt] = Fixed(bodyA,bodyB,joint_pos_node,disableCollision)
                assert(constraints[jt])
        else:
                print("Arriba!")
hull_base_motor = ControlledTorqueMotor(constraints["jt_base_to_hull_rot"],194.0,500.0,0.5,KC.LEFT,KC.RIGHT)
basecylinderr = ControlledCylinderMotor(constraints["jt_cyl_hull_R_trans"],490.0,0.5,KC.UP,KC.DOWN)
basecylinderl = ControlledCylinderMotor(constraints["jt_cyl_hull_L_trans"],490.0,0.5,KC.UP,KC.DOWN)
boom1cylinder = ControlledCylinderMotor(constraints["jt_cyl_boom1_trans"],521.0,0.5,KC.U,KC.J)
boom2cylinder = ControlledCylinderMotor(constraints["jt_cyl_boom2_trans"],388.0,0.5,KC.I,KC.K)
