# -*- coding: utf-8 -*-
# Create physics

PREFIX_RIGIDBODY = "rb_"
PREFIX_COLLISION = "cb_"
import math

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


"""


class ControlledCylinderMotor:
    def __init__(self,slider=None,maxmotorforce=300.0,maxspeed=0.1,key1=None,key2=None, modifier=KEY_MOD.NONE):
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
        self.control = game.event_manager.createKeyTrigger(key1, modifier)
        assert(self.control)
        self.control.addKeyDownListener(self.subtract)
        self.control.addKeyUpListener(self.zero)
        
        self.control = game.event_manager.createKeyTrigger(key2, modifier)
        assert(self.control)
        self.control.addKeyDownListener(self.extend)
        self.control.addKeyUpListener(self.zero)
        
    def extend(self):
        self.motor.target_velocity = self.maxspeed*self.axisvec
    def subtract(self):
        self.motor.target_velocity = -self.maxspeed*self.axisvec
    def zero(self):
        self.motor.target_velocity = Vector3.zero
class ControlledTorqueMotor:
    def __init__(self,hinge=None,maxmotorforce=300.0,maxlimittorque=600.0,maxspeed=0.1,key1=None,key2=None, modifier=KEY_MOD.NONE):
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
        self.control = game.event_manager.createKeyTrigger(key1, modifier)
        assert(self.control)
        self.control.addKeyDownListener(self.rotatepositive)
        self.control.addKeyUpListener(self.zero)
        
        self.control = game.event_manager.createKeyTrigger(key2, modifier)
        assert(self.control)
        self.control.addKeyDownListener(self.rotatenegative)
        self.control.addKeyUpListener(self.zero)
    def rotatepositive(self):
        self.motor.target_velocity = self.maxspeed*self.axisvec
    def rotatenegative(self):
        self.motor.target_velocity = -self.maxspeed*self.axisvec
    def zero(self):
        self.motor.target_velocity = Vector3.zero
"""
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
        self.body.set_sleeping_thresholds(0.0, 0.0)

def createFixedConstraint2(bodyA, bodyB, joint_transform, disableCollision):
    assert(isinstance(bodyA,RigidBody))
    assert(isinstance(bodyB,RigidBody))
    localA_trans = bodyA.body.transform_to_local(joint_transform)
    localB_trans = bodyB.body.transform_to_local(joint_transform)
    constraint = PSixDofConstraint.create(bodyA.body, bodyB.body, localA_trans, localB_trans, False)
    assert(constraint)
    
    constraint.angular_lower_limit = Vector3.zero
    constraint.angular_upper_limit = Vector3.zero
    
    constraint.linear_lower_limit = Vector3.zero
    constraint.linear_upper_limit = Vector3.zero
    
    game.physics_world.addConstraint(constraint, disableCollision)
    return(constraint, "No_Motor")
    
def createSliderConstraint2(bodyA, bodyB, joint_transform, lowerlimit, upperlimit, disableCollision, max_motor_force):
    assert(isinstance(bodyA,RigidBody))
    assert(isinstance(bodyB,RigidBody))
    #(axis_number, axis_vector) = convert_axis_char(axis)
    axis_vector = Vector3.unit_z
    axis_number = 2
    localA_trans = bodyA.body.transform_to_local(joint_transform)
    localB_trans = bodyB.body.transform_to_local(joint_transform)
    constraint = PSixDofConstraint.create(bodyA.body, bodyB.body, localA_trans, localB_trans, False)
    assert(constraint)
    
    constraint.angular_lower_limit = 10.0*axis_vector
    constraint.angular_upper_limit = -10.0*axis_vector
    
    constraint.linear_lower_limit = lowerlimit*axis_vector
    constraint.linear_upper_limit = upperlimit*axis_vector
    
    game.physics_world.addConstraint(constraint, disableCollision)
    motor=constraint.translation_motor
    assert(motor)
    if max_motor_force:
        motor.max_motor_force = max_motor_force*axis_vector
        motor.target_velocity = Vector3.zero
        motor.restitution = 0.0
        motor.normal_CFM = Vector3(0.1,0.1,0.1)
        motor.stop_ERP = Vector3(0.8,0.8,0.8)
        motor.stop_CFM = Vector3(0.1,0.1,0.1)
        motor.limit_softness = 0.0
        motor.enable_motor(axis_number)
    return(constraint, motor)
        
def createHingeConstraint2(bodyA, bodyB, joint_transform, lowerlimit, upperlimit, disableCollision, max_motor_force, max_limit_torque):
    assert(isinstance(bodyA,RigidBody))
    assert(isinstance(bodyB,RigidBody))
    #(axis_number, axis_vector) = convert_axis_char(axis)
    axis_vector = Vector3.unit_x
    axis_number = 0
    localA_trans = bodyA.body.transform_to_local(joint_transform)
    localB_trans = bodyB.body.transform_to_local(joint_transform)
    constraint = PSixDofConstraint.create(bodyA.body, bodyB.body, localA_trans, localB_trans, False)
    assert(constraint)
    
    constraint.angular_lower_limit = lowerlimit*axis_vector
    constraint.angular_upper_limit = upperlimit*axis_vector
    
    constraint.linear_lower_limit = Vector3.zero
    constraint.linear_upper_limit = Vector3.zero
    
    game.physics_world.addConstraint(constraint, disableCollision)
    motor=constraint.rotation_motor
    assert(motor)
    if max_motor_force and max_limit_torque:
        motor.max_limit_torque = max_limit_torque*axis_vector
        motor.max_motor_force = max_motor_force*axis_vector
        motor.target_velocity = Vector3.zero
        motor.restitution = 0.0
        motor.normal_CFM = Vector3(0.1,0.1,0.1)
        motor.stop_ERP = Vector3(0.8,0.8,0.8)
        motor.stop_CFM = Vector3(0.1,0.1,0.1)
        motor.limit_softness = 0.0
        motor.enable_motor(axis_number)
    return(constraint, motor)

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

if __name__ == "__main__":
    world = initphysics()
    game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
    game.scene.shadows.enable()
    game.scene.shadows.texture_size = 4096
    game.scene.shadows.max_distance = 50
    create_sun()
    ground = physics_create_ground()
    game.scene.sky_dome = SkyDomeInfo("CloudySky")

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
    joint_dict = dict(jt_track_L_to_base_fix=("track_L","base","Fixed", True, None, None, None, None), 
                      jt_track_R_to_base_fix=("track_R","base","Fixed", True, None, None, None, None),
                      jt_base_to_hull_rot=("base", "hull", "Hinge", 1.0, -1.0, True, 194.0, 300.0),
                      jt_hull_to_boom1_rot=("hull","boom1","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_R_tube_rot=("hull","cyl_hull_R_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_R_trans=("cyl_hull_R_tube","cyl_hull_R_rod","Slider",-0.20,0.60,True, 490.0, None),
                      jt_cyl_hull_R_rod_rot=("cyl_hull_R_rod","boom1","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_L_tube_rot=("hull","cyl_hull_L_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_hull_L_trans=("cyl_hull_L_tube","cyl_hull_L_rod","Slider",-0.20,0.60,True, 490.0, None),
                      jt_cyl_hull_L_rod_rot=("cyl_hull_L_rod","boom1","Hinge",1.0,-1.0,True, None, None),
                      jt_boom1_to_boom2_rot=("boom1","boom2","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom1_tube_rot=("boom1","cyl_boom1_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom1_trans=("cyl_boom1_tube","cyl_boom1_rod","Slider",-1.2,0.35,True, 521.0, None),
                      jt_cyl_boom1_rod_rot=("cyl_boom1_rod","boom2","Hinge",1.0,-1.0,True, None, None),
                      jt_boom2_to_bucket_rot=("boom2","bucket","Hinge",1.0,-1.0,True, None, None),
                      jt_boom2_to_fourbarlink1_rot=("boom2","fourbarlink1","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom2_tube_rot=("boom2","cyl_boom2_tube","Hinge",1.0,-1.0,True, None, None),
                      jt_cyl_boom2_trans=("cyl_boom2_tube","cyl_boom2_rod","Slider",-1.40,0.05,True, 388.0, None),
                      jt_cyl_boom2_rod_rot=("cyl_boom2_rod","fourbarlink1","Hinge",1.0,-1.0,True, None, None),
                      jt_fourbarlink2_to_bucket_rot=("fourbarlink2","bucket","Hinge",1.0,-1.0,True, None, None),
                      jt_fourbarlink1_to_fourbarlink2_rot=("fourbarlink1","fourbarlink2","Hinge",1.0,-1.0,True, None, None)
                      )   
    constraints = dict()
    for jt in joint_dict:
        # Collect data from joint table:
        (nameA, nameB, jtype, low, hi, disableCollision, max_force, max_torque) = joint_dict[jt]
        bodyA = rigidbodies[nameA]
        #game.physics_world.getRigidBody(PREFIX_RIGIDBODY+jt[0])
        assert bodyA, "RigidBody named: {0} does not exist!".format(nameA)
        bodyB = rigidbodies[nameB]
        #game.physics_world.getRigidBody(PREFIX_RIGIDBODY+jt[1])
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
    
    hull_base_motor = KBandJoystickController(constraints["jt_base_to_hull_rot"][1], Vector3(-0.5,0.0,0.0), KC.LEFT, KC.RIGHT, KEY_MOD.NONE, 'X')
    hull_base_motor.name = "HullBase"
    basecylinderr = KBandJoystickController(constraints["jt_cyl_hull_R_trans"][1], Vector3(0.0, 0.0, 0.5), KC.UP, KC.DOWN, KEY_MOD.NONE,'Y')
    basecylinderr.name = "Cylinder0R"
    basecylinderl = KBandJoystickController(constraints["jt_cyl_hull_L_trans"][1], Vector3(0.0, 0.0, 0.5), KC.UP, KC.DOWN, KEY_MOD.NONE, 'Y')
    basecylinderl.name = "Cylinder0L"
    boom1cylinder = KBandJoystickController(constraints["jt_cyl_boom1_trans"][1], Vector3(0.0, 0.0, 0.5), KC.UP, KC.DOWN, KEY_MOD.CTRL, 'Y', 0)
    boom1cylinder.name = "boom1cylinder"
    boom2cylinder = KBandJoystickController(constraints["jt_cyl_boom2_trans"][1], Vector3(0.0, 0.0, 0.5), KC.UP, KC.DOWN, KEY_MOD.SHIFT, 'Y', 1)
    boom2cylinder.name = "Boom2cylinder"
    """
    hull_base_motor = ControlledTorqueMotor(constraints["jt_base_to_hull_rot"],194.0,500.0,0.5,KC.LEFT,KC.RIGHT)
    basecylinderr = ControlledCylinderMotor(constraints["jt_cyl_hull_R_trans"],490.0,0.5,KC.UP,KC.DOWN)
    basecylinderl = ControlledCylinderMotor(constraints["jt_cyl_hull_L_trans"],490.0,0.5,KC.UP,KC.DOWN)
    boom1cylinder = ControlledCylinderMotor(constraints["jt_cyl_boom1_trans"],521.0,0.5,KC.UP,KC.DOWN, MOD_KEY.CTRL)
    boom2cylinder = ControlledCylinderMotor(constraints["jt_cyl_boom2_trans"],388.0,0.5,KC.UP,KC.DOWN, MOD_KEY.SHIFT)
    """
