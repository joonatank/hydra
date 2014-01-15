# -*- coding: utf-8 -*-
# Create physics

PREFIX_RIGIDBODY = "rb_"
PREFIX_COLLISION = "cb_"
import math


#Shorter way to express Vector3, dunno if this is allowed or what?
Vec3 = Vector3


def initphysics():
    ##Warning this function has to be run before using any physics.
    ##Also it's return value has to be assigned into a variable named "phy"
    create_camera_controller()               
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
    solverparams.internal_time_step = 1.0/480.0
    #No idea what substep, has it something to do with interpolation, well higher value should be more precise but wastes much more computing power?
    solverparams.max_sub_steps = 9
    #Now we set the solver parameters to current physics world:
    game.physics_world.solver_parameters = solverparams
    #We hide all the collision objects:
    game.scene.hideSceneNodes(PREFIX_COLLISION+"*")
    return game.physics_world
"""
class RumblePad:
    pass
class MotorContoller:
    def __init__(motors):
        self.motors = motors
    def positive_x():
        pass
    def negative_x():
        pass
    def 
        
    pass
"""
class KBandJoystickController:
    def __init__(self, motor, max_velocity, key1, key2, keymodifier=KEY_MOD.NONE, joystick_axis = 'X', *buttonmods):
        self.name = "default"
        self._motor = motor
        self._jsignal = 0.0
        self._kbsignal = 0.0
        #self._joystickmap =
        self.disabled = False
        self.joystick_axis = joystick_axis
        self.max_velocity = max_velocity
        self.buttonmods = buttonmods
        self.keymodifier = keymodifier
        self.trigger_positive = game.event_manager.createKeyTrigger(key2, self.keymodifier)
        self.trigger_negative = game.event_manager.createKeyTrigger(key1, self.keymodifier)
        assert(self.trigger_positive and self.trigger_negative)
        
        self.trigger_positive.addKeyDownListener(self.positive_signal)
        self.trigger_positive.addKeyUpListener(self.zero_signal)
        
        self.trigger_negative.addKeyDownListener(self.negative_signal)
        self.trigger_negative.addKeyUpListener(self.zero_signal)
        
        self.joystick = game.event_manager.getJoystick()
        self.joystick.addListener(self.joystick_listener)
        
    def positive_signal(self):
        self._kbsignal = 1.0
        self.execute()
    def negative_signal(self):
        self._kbsignal = -1.0
        self.execute()
    def zero_signal(self):
        self._kbsignal = 0.0
        self.execute()
    def execute(self):
        clamped = self._clamp_signal(self._jsignal+self._kbsignal, 1.0)
        self._motor.target_velocity = clamped*self.max_velocity
    def joystick_listener(self, evt):
        if self.disabled:
            return
        if self.check_button_modifiers(evt)[0]:
            #@TODO: repair this to selectable in python style, no naive axis parameters:
            if self.joystick_axis is 'X':
                self._jsignal = evt.axis_x
            if self.joystick_axis is 'Y':
                self._jsignal = evt.axis_y
            if self.joystick_axis is 'Z':
                self._jsignal = evt.axis_z
        else:
            self._jsignal = 0.0
        self.execute()
        
    def check_button_modifiers(self, evt):
        buttons=[False, False, False, False, False, False, False, False]
        bcount = 0
        modcount = 0
        total_mod_count = 0
        for i in range(8):
            if evt.is_button_down(i):
                buttons.insert(i, True)
                bcount += 1
            else:
                buttons.insert(i, False)
        if not self.buttonmods:
            modcount = 0
            total_mod_count = 0
        else:
            for k in self.buttonmods:
                total_mod_count += 1
                if evt.is_button_down(k):
                    modcount += 1
        #print(self.name, "MODCOUNT: ", modcount, "BCOUNT: ", bcount, "Buttons: ", buttons)
        if bcount is modcount and modcount is total_mod_count:
            return(True, buttons)
        else:
            return(False, buttons)
    
    def _clamp_signal(self, signal, max):
        if math.fabs(signal) >= max:
            return math.copysign(max, signal)
        else:
            return signal

def createRigidBody(name, mstate, shape, mass=0.0, inertia=Vec3.zero, user_controlled=False):
    rb = phy.createRigidBody(phy, name, mstate, shape)
    rb.mass = mass
    rb.inertia = inertia
    rb.user_controlled = user_controlled
    return rb

def createCompoundShape(parentname, childnamelist):
##    Creates a compound collision shape. Parent is used only for base coordinate system of the shape.
##    in practice parent is the visual node of the rigid body. (eg. it's shape isn't added to compound) All child shape
##    coordinates are transformed to the parents coordinates.
##    Namelist is a list of node names corresponding all the nodes of the compound shape NOTE: node and mesh names
##    doesn't have to correspond each other, script collects meshes from nodes.
    gnode = game.scene.getSceneNode(parentname)
    cshape = CompoundShape.create(True)
    for childname in childnamelist:
        childnode = game.scene.getSceneNode(childname)
        childmsh = childnode.objects[0].mesh
        #Create child shapes:
        childshape = ConvexHullShape.create(childmsh)
        #We need to input child shapes local position and orientation in graphics node coordinate system:
        local_transformation = gnode.world_transformation.inverted()*childnode.world_transformation
        cshape.add_child_shape(local_transformation, childshape)
    return cshape


##class RigidBody:
##    def __init__(self, world, name=None, shape=None, mass=0.0, inertia=Vec3(0.0, 0.0, 0.0), controlled=True):
##        # Get scene node, check if it exists:
##        self.snode = game.scene.getSceneNode(name)
##        assert self.snode, "Object named: {0} does not exist!".format(name)
##        # Get object collision mesh cb_{object_name}, if it exists:
##        #self.msh = game.mesh_manager.loadMesh("cb_" + name)
##        #assert self.msh, "Mesh name does not correspond to object name {0} + {1}!".format(PREFIX_COLLISION,name)
##        #self.cshape = ConvexHullShape.create(self.msh)
##        self.shape = shape
##        self.motion_state = phy.createMotionState(self.snode.world_transformation,self.snode)
##        assert(self.motion_state)
##        self.body = world.createRigidBody(name, mass, self.motion_state, self.shape, inertia)
##        assert(self.body)
##        self.body.user_controlled = controlled
##        self.body.linear_damping = 0.0
##        self.body.angular_damping = 0.0
##        self.body.kinematic = False
##        self.body.set_sleeping_thresholds(0.0, 0.0)
##
##    def getShape(self):
##        return self.shape
##    def getNative(self):
##        return body
##    def getMotionState(self):
##        return self.motion_state
##    def setLinearDamping(self, value):
##        self.body.linear_damping = value
##    def getLinearDamping(self):
##        return self.body.linear_damping
##    def setAngularDamping(self,value):
##        self.body.angular_damping = value
##    def getAngularDamping(self):
##        return self.body.angular_damping
##    def setKinematic(self,boolean):
##        self.body.kinematic = boolean
##    def isKinematic(self):
##        return self.body.kinematic
##    def setSleepingThresholds(self, value1,value2):
##        self.body.set_sleeping_thresholds(value1,value2)


def createFixedConstraint2(bodyA, bodyB, joint_transform, disableCollision):
    localA_trans = bodyA.transform_to_local(joint_transform)
    localB_trans = bodyB.transform_to_local(joint_transform)
    constraint = PSixDofConstraint.create(bodyA, bodyB, localA_trans, localB_trans, False)
    assert(constraint)
    
    constraint.angular_lower_limit = Vec3.zero
    constraint.angular_upper_limit = Vec3.zero
    
    constraint.linear_lower_limit = Vec3.zero
    constraint.linear_upper_limit = Vec3.zero
    
    phy.addConstraint(constraint, disableCollision)
    return(constraint, "No_Motor")
    
def createSliderConstraint2(bodyA, bodyB, joint_transform, lowerlimit, upperlimit, disableCollision, max_motor_force):
    assert(isinstance(bodyA,RigidBody))
    assert(isinstance(bodyB,RigidBody))
    #(axis_number, axis_vector) = convert_axis_char(axis)
    axis_vector = Vec3.unit_z
    axis_number = 2
    localA_trans = bodyA.transform_to_local(joint_transform)
    localB_trans = bodyB.transform_to_local(joint_transform)
    constraint = PSixDofConstraint.create(bodyA, bodyB, localA_trans, localB_trans, False)
    assert(constraint)
    
    constraint.angular_lower_limit = 1.0*axis_vector
    constraint.angular_upper_limit = -1.0*axis_vector
    
    constraint.linear_lower_limit = lowerlimit*axis_vector
    constraint.linear_upper_limit = upperlimit*axis_vector
    
    phy.addConstraint(constraint, disableCollision)
    motor=constraint.translation_motor
    assert(motor)
    if max_motor_force:
        motor.max_motor_force = max_motor_force*axis_vector
        motor.target_velocity = Vec3.zero
        motor.restitution = 1.0
        motor.normal_CFM = Vec3(0.1,0.1,0.1)
        motor.stop_ERP = Vec3(0.8,0.8,0.8)
        motor.stop_CFM = Vec3(0.1,0.1,0.1)
        motor.limit_softness = 1.0
        motor.enable_motor(axis_number)
    return(constraint, motor)
        
def createHingeConstraint2(bodyA, bodyB, joint_transform, lowerlimit, upperlimit, disableCollision, max_motor_force, max_limit_torque):
    assert(isinstance(bodyA,RigidBody))
    assert(isinstance(bodyB,RigidBody))
    #(axis_number, axis_vector) = convert_axis_char(axis)
    axis_vector = Vec3.unit_x
    axis_number = 0
    localA_trans = bodyA.transform_to_local(joint_transform)
    localB_trans = bodyB.transform_to_local(joint_transform)
    constraint = PSixDofConstraint.create(bodyA, bodyB, localA_trans, localB_trans, False)
    assert(constraint)
    
    constraint.angular_lower_limit = lowerlimit*axis_vector
    constraint.angular_upper_limit = upperlimit*axis_vector
    
    constraint.linear_lower_limit = Vec3.zero
    constraint.linear_upper_limit = Vec3.zero
    
    phy.addConstraint(constraint, disableCollision)
    motor=constraint.rotation_motor
    assert(motor)
    if max_motor_force and max_limit_torque:
        motor.max_limit_torque = max_limit_torque*axis_vector
        motor.max_motor_force = max_motor_force*axis_vector
        motor.target_velocity = Vec3.zero
        motor.restitution = 1.0
        motor.normal_CFM = Vec3(0.1,0.1,0.1)
        motor.stop_ERP = Vec3(0.8,0.8,0.8)
        motor.stop_CFM = Vec3(0.1,0.1,0.1)
        motor.limit_softness = 1.0
        motor.enable_motor(axis_number)
    return(constraint, motor)





if True :
    phy = initphysics()
    game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
    game.scene.shadows.enable()
    game.scene.shadows.texture_size = 4096
    game.scene.shadows.max_distance = 50
    create_sun()

    #ground = physics_create_ground()

    game.scene.sky_dome = SkyDomeInfo("CloudySky")

    #ground.disable_collisions = True
    # RIGID BODY PARAMETER TABLE:
    dynamic_rigidbody_data = dict(track_L=(11.1,Vec3(34.02,33.37,3.26)),
                          track_R=(11.1,Vec3(34.02,33.37,3.26)),
                          hull=(7.96,Vec3(15.07, 23.48, 14.31)),
                          base=(23.04,Vec3(19.23,30.743,18.37)),
                          boom1=(5.785,Vec3(30.95,29.27,2.54)),
                          boom2=(3.045,Vec3(5.02,4.81,0.54)),
                          fourbarlink1=(0.1,Vec3(0.05,0.05,0.03)),
                          fourbarlink2=(0.1,Vec3(0.05,0.05,0.03)),
                          cyl_hull_R_rod=(0.22,Vec3(0.07,0.07,0.002)),
                          cyl_hull_R_tube=(0.33,Vec3(0.11,0.11,0.01)),
                          cyl_hull_L_rod=(0.22,Vec3(0.07,0.07,0.002)),
                          cyl_hull_L_tube=(0.33,Vec3(0.11,0.11,0.01)),
                          cyl_boom1_rod=(0.305,Vec3(0.12,0.12,0.003)),
                          cyl_boom1_tube=(0.46,Vec3(0.18,0.18,0.014)),
                          cyl_boom2_rod=(0.19,Vec3(0.04,0.04,0.002)),
                          cyl_boom2_tube=(0.285,Vec3(0.06,0.06,0.01))                          
                          )
    static_rigidbody_data = ["t1_1_terrain_grass",
                          "t1_2_terrain_grass",
                          "t1_3_terrain_grass",
                          "t2_1_terrain_grass",
                          "t2_2_terrain_dirt_tracks",
                          "t2_3_terrain_pit",
                          "t3_1_terrain_grass",
                          "t3_2_terrain_grass",
                          "t3_3_terrain_grass"]
    compound_rigidbody_data = dict(bucket = ((3.126, Vec3(1.74,1.78,1.22)), ("cb_bucket0", "cb_bucket1", "cb_bucket2", "cb_bucket3", "cb_bucket4", "cb_bucket5", "cb_bucket6", "cb_bucket7", "cb_bucket8", "cb_bucket9", "cb_bucket10")))

    VAARArigidbody_dict = dict(track_L=(11.1,Vec3(34.02,33.37,3.26)),
                          track_R=(11.1,Vec3(34.02,33.37,3.26)),
                          hull=(23.04,Vec3(43.63,67.97,41.43)),
                          base=(7.96,Vec3(6.65,10.62,6.35)),
                          boom1=(5.786,Vec3(30.84,29.16,2.52)),
                          boom2=(3.045,Vec3(6.0,0.61,5.58)),
                          bucket=(3.13,Vec3(1.74,1.78,1.22)),
                          fourbarlink1=(0.1,Vec3(0.05,0.03,0.05)),
                          fourbarlink2=(0.1,Vec3(0.05,0.03,0.05)),
                          cyl_hull_R_rod=(0.22,Vec3(0.07,0.002,0.07)),
                          cyl_hull_R_tube=(0.33,Vec3(0.11,0.01,0.11)),
                          cyl_hull_L_rod=(0.22,Vec3(0.07,0.002,0.07)),
                          cyl_hull_L_tube=(0.33,Vec3(0.11,0.01,0.11)),
                          cyl_boom1_rod=(0.305,Vec3(0.12,0.003,0.12)),
                          cyl_boom1_tube=(0.46,Vec3(0.18,0.014,0.18)),
                          cyl_boom2_rod=(0.19,Vec3(0.04,0.002,0.04)),
                          cyl_boom2_tube=(0.285,Vec3(0.06,0.01,0.06))
                          )
    # We'll loop trough the dynamic_rigidbody_data and create rigid bodies with Convex Shapes.
    rigidbodies = dict()
    for name,data in dynamic_rigidbody_data.items():
            # Get mass:
            mass = data[0]
            #assert mass, "Mass {0} wasn't defined for object: {1}".format(mass, name)
            # Get inertia:
            inertia = data[1]
            assert inertia, "Inertia wasn't defined for object: {0}".format(name)
            try :
                controlled = data[2]
            except IndexError:
                controlled = True
            #Get node, mesh and shape:
            node = game.scene.getSceneNode(name)
            cbnode = game.scene.getSceneNode(PREFIX_COLLISION+name)
            msh = cbnode.objects[0].mesh
            shape = ConvexHullShape.create(msh)
            mstate = phy.createMotionState(node.world_transformation, node)
            rigidbodies[name] = phy.createRigidBody(name, mass, mstate ,shape, inertia)
            rigidbodies[name].user_controlled = controlled
            rigidbodies[name].linear_damping = 0.0
            rigidbodies[name].angular_damping = 0.0
            rigidbodies[name].kinematic = False
            rigidbodies[name].set_sleeping_thresholds(0.0, 0.0)

    #Handling static triangle mesh shape geometry:
    for name in static_rigidbody_data:
        node = game.scene.getSceneNode(name)
        msh = node.objects[0].mesh
        shape = StaticTriangleMeshShape.create(msh)
        mstate = phy.createMotionState(node.world_transformation,node)
        rigidbodies[name] = phy.createRigidBody(name, 0.0, mstate, shape, Vec3(0,0,0))

    #Handling dynamic rigidbodies with compound shapes :
    for name, (params, childlist) in compound_rigidbody_data.items():    
        #First create the compound shape out of graphics node and childnodes:
        cshape = createCompoundShape(name, childlist)
        #Create a rigidbody out of it:
        mass = params[0]
        inertia = params[1]
        try:
            controlled = params[3]
        except IndexError:
            controlled = True
        gnode = game.scene.getSceneNode(name)
        mstate = phy.createMotionState(gnode.world_transformation, gnode)
        rigidbodies[name] = phy.createRigidBody(name, mass, mstate, cshape, inertia)
        rigidbodies[name].user_controlled = controlled

    #Creating the rocks:
    cbnode = game.scene.getSceneNode("cb_rock")
    msh = cbnode.objects[0].mesh
    shape = ConvexHullShape.create(msh)
    for x in range(-2,2):
        for y in range(5,10):
            for z in range(-10,-5):
                new_name = "rock"+"_"+str(x)+"_"+str(y)+"_"+str(z)
                new_node = game.scene.createSceneNode(new_name)
                new_entity = game.scene.createEntity(new_name, "rock", True)
                new_node.attachObject(new_entity)
                new_node.position = Vec3(x, y, z)
                #Don't change this earlier to set position:
                mstate = phy.createMotionState(new_node.world_transformation, new_node)
                rigidbodies[new_name] = phy.createRigidBody(new_name, 0.2, mstate, shape, Vec3(0.026,0.029,0.037))
                rigidbodies[new_name].user_controlled = False
                rigidbodies[new_name].linear_damping = 0.00
                rigidbodies[new_name].angular_damping = 0.00
                rigidbodies[new_name].kinematic = False
                rigidbodies[new_name].set_sleeping_thresholds(0.001,math.pi/180)
                rigidbodies[new_name].anisotropic_friction = Vec3(2.0, 2.0, 2.0)
                print("ROCK: ", new_name, "CREATED!")
    #Disable collision from few rigidbodies:
    rigidbodies['fourbarlink1'].disable_collisions = True
    rigidbodies['fourbarlink2'].disable_collisions = True
    rigidbodies['cyl_boom2_rod'].disable_collisions = True
    rigidbodies['cyl_hull_R_rod'].disable_collisions = True
    rigidbodies['cyl_hull_R_tube'].disable_collisions = True
    rigidbodies['cyl_hull_L_rod'].disable_collisions = True
    rigidbodies['cyl_hull_L_tube'].disable_collisions = True
    rigidbodies['cyl_boom1_rod'].disable_collisions = True
    rigidbodies['cyl_boom1_tube'].disable_collisions = True
    rigidbodies['cyl_boom2_rod'].disable_collisions = True
    rigidbodies['cyl_boom2_tube'].disable_collisions = True
    #rigidbodies['track_L'].disable_collisions = True
    #rigidbodies['track_R'].disable_collisions = True
    rigidbodies['hull'].disable_collisions = True
    rigidbodies['base'].disable_collisions = True
    rigidbodies['boom1'].disable_collisions = True
    rigidbodies['boom2'].disable_collisions = True
    #rigidbodies['bucket'].disable_collisions = True
    # When upperlimit < lowerlimit: constraint dof will be totally free
    # When upperlimit & lowerlimit == 0 constraint is locked
    # FORMAT:
    #1.34
    #-0.2 ja 0.5 oikeat rajat hull cyl
    joint_dict = dict(jt_track_L_to_base_fix=("track_L","base","Fixed", True, None, None, None, None), 
                      jt_track_R_to_base_fix=("track_R","base","Fixed", True, None, None, None, None),
                      jt_base_to_hull_rot=("base", "hull", "Hinge", 1.0, -1.0, True, 193.8*60, 193.8*60),
                      jt_hull_to_boom1_rot=("hull","boom1","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_R_tube_rot=("hull","cyl_hull_R_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_R_trans=("cyl_hull_R_tube","cyl_hull_R_rod","Slider",-1.30,0.5,True, 974.2*60, None),
                      jt_cyl_hull_R_rod_rot=("cyl_hull_R_rod","boom1","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_L_tube_rot=("hull","cyl_hull_L_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_L_trans=("cyl_hull_L_tube","cyl_hull_L_rod","Slider",-1.30,0.5,True, 974.2*60, None),
                      jt_cyl_hull_L_rod_rot=("cyl_hull_L_rod","boom1","Hinge",1.0,-1.0,True, None, None),
                      jt_boom1_to_boom2_rot=("boom1","boom2","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom1_tube_rot=("boom1","cyl_boom1_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom1_trans=("cyl_boom1_tube","cyl_boom1_rod","Slider",-1.2,0.35,True, 1078.8*60, None),
                      jt_cyl_boom1_rod_rot=("cyl_boom1_rod","boom2","Hinge",1.0,-1.0,True, None, None),
                      jt_boom2_to_bucket_rot=("boom2","bucket","Hinge",1.0,-1.0,True, None, None),
                      jt_boom2_to_fourbarlink1_rot=("boom2","fourbarlink1","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom2_tube_rot=("boom2","cyl_boom2_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom2_trans=("cyl_boom2_tube","cyl_boom2_rod","Slider",-1.40,0.05,True, 865.2*60, None),
                      jt_cyl_boom2_rod_rot=("cyl_boom2_rod","fourbarlink1","Hinge",1.0,-1.0,True, None, None),
                      jt_fourbarlink2_to_bucket_rot=("fourbarlink2","bucket","Hinge",1.0,-1.0,True, None, None),
                      jt_fourbarlink1_to_fourbarlink2_rot=("fourbarlink1","fourbarlink2","Hinge",1.0,-1.0,True, None, None)
                      )
    constraints = dict()
    for jt in joint_dict:
        # Collect data from joint table:
        (nameA, nameB, jtype, low, hi, disableCollision, max_force, max_torque) = joint_dict[jt]
        bodyA = rigidbodies[nameA]
        #phy.getRigidBody(PREFIX_RIGIDBODY+jt[0])
        assert bodyA, "RigidBody named: {0} does not exist!".format(nameA)
        bodyB = rigidbodies[nameB]
        #phy.getRigidBody(PREFIX_RIGIDBODY+jt[1])
        assert bodyB, "RigidBody named: {0} does not exist!".format(nameB)
        joint_pos_node = game.scene.getSceneNode(jt)
        jtw = joint_pos_node.world_transformation
        assert joint_pos_node, "Object named: {0} does not exist!".format(jt)
        print(jtype)
        if jtype is "Hinge":
            constraints[jt] = createHingeConstraint2(bodyA, bodyB, jtw, low, hi, disableCollision, max_force, max_torque)
        elif jtype is "Slider":
            constraints[jt] = createSliderConstraint2(bodyA, bodyB, jtw, low, hi, disableCollision, max_force)
        elif jtype is "Fixed":
            constraints[jt] = createFixedConstraint2(bodyA, bodyB, jtw, disableCollision)
        else:
            print("Arriba!")
            print(jt, bodyA, bodyB, jtype)
    
    hull_base_motor = KBandJoystickController(constraints["jt_base_to_hull_rot"][1], Vec3(-0.5,0.0,0.0), KC.LEFT, KC.RIGHT, KEY_MOD.NONE, 'X')
    hull_base_motor.name = "HullBase"
    basecylinderr = KBandJoystickController(constraints["jt_cyl_hull_R_trans"][1], Vec3(0, 0, 0.5), KC.UP, KC.DOWN, KEY_MOD.NONE,'Y')
    basecylinderr.name = "Cylinder0R"
    basecylinderl = KBandJoystickController(constraints["jt_cyl_hull_L_trans"][1], Vec3(0, 0, 0.5), KC.UP, KC.DOWN, KEY_MOD.NONE, 'Y')
    basecylinderl.name = "Cylinder0L"
    boom1cylinder = KBandJoystickController(constraints["jt_cyl_boom1_trans"][1], Vec3(0.0, 0.0, -0.5), KC.UP, KC.DOWN, KEY_MOD.CTRL, 'Y', 0)
    boom1cylinder.name = "boom1cylinder"
    boom2cylinder = KBandJoystickController(constraints["jt_cyl_boom2_trans"][1], Vec3(0.0, 0.0, -0.5), KC.UP, KC.DOWN, KEY_MOD.SHIFT, 'Y', 1)
    boom2cylinder.name = "Boom2cylinder"
    
    #ground.friction = 1.0
    #ground.anisotropic_friction = Vec3(1.0, 1.0, 1.0)
    #rigidbodies['bucket'].friction = 1.0
    #rigidbodies['track_L'].friction = 1.0
    #rigidbodies['track_R'].friction = 1.0
    
    #rigidbodies['bucket'].anisotropic_friction = Vec3(2.0, 2.0, 2.0)
    #rigidbodies['track_L'].anisotropic_friction = Vec3(6.0, 6.0, 6.0)
    #rigidbodies['track_R'].anisotropic_friction = Vec3(6.0, 6.0, 6.0)

    
    def more_power():
        motR = constraints["jt_cyl_hull_R_trans"][1]
        motL = constraints["jt_cyl_hull_L_trans"][1]
        motR.max_motor_force += Vec3(0,0,100.0) 
        motL.max_motor_force += Vec3(0,0,100.0)
    trigu = game.event_manager.createKeyTrigger(KC.M)
    trigu.addListener(more_power)
    
    def adjust_power(dt):
        motHull = constraints["jt_base_to_hull_rot"][1]
        motCBoomR = constraints["jt_cyl_hull_R_trans"][1]
        motCBoomL = constraints["jt_cyl_hull_L_trans"][1]
        motCArm = constraints["jt_cyl_boom1_trans"][1]
        motCBucket = constraints["jt_cyl_boom2_trans"][1]
        fdt = float(dt)
        torqueHull = Vec3(0,0,193.8/fdt)
        forceBoom = Vec3(0,0,974.2/fdt)
        forceArm = Vec3(0,0,1078.8/fdt)
        forceBucket = Vec3(0,0,865.2/fdt)
        motHull.max_motor_torque = torqueHull
        motCBoomR.max_motor_force = forceBoom
        motCBoomL.max_motor_force = forceBoom
        motCArm.max_motor_force = forceArm
        motCBucket.max_motor_force = forceBucket
    frame_trigu = game.event_manager.getFrameTrigger()
    frame_trigu.addListener(adjust_power)
