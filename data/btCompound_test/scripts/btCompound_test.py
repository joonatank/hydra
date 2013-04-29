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
    solverparams.internal_time_step = 1.0/120.0
    #No idea what substep, has it something to do with interpolation, well higher value should be more precise but wastes much more computing power?
    solverparams.max_sub_steps = 20
    #Now we set the solver parameters to current physics world:
    game.physics_world.solver_parameters = solverparams
    return game.physics_world


def createRB(name = None, shape = None, mass = 1.0, inertia = Vector3(0.3, 0.3, 0.3)):
    assert(shape)
    snode = game.scene.getSceneNode(name)
    assert(snode)    
    motion_state = game.physics_world.createMotionState(snode.world_transformation, snode)
    assert(motion_state)
    rbody = game.physics_world.createRigidBody("rb_"+name, mass, motion_state, shape, inertia)
    assert(rbody)
    rbody.user_controlled = True
    rbody.linear_damping = 0.05
    rbody.angular_damping = 0.05
    rbody.kinematic = False
    rbody.set_sleeping_thresholds(0.0, 0.0)
    return rbody

#MAIN STARTS HERE:
world = initphysics()

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
game.scene.shadows.max_distance = 50

ground = physics_create_ground()
game.scene.sky_dome = SkyDomeInfo("CloudySky")

#We need scene nodes for calculating correct local transformations:
cpool = game.scene.getSceneNode("collision_pool")
wall_B = game.scene.getSceneNode("wall_B")
wall_L = game.scene.getSceneNode("wall_L")
wall_R = game.scene.getSceneNode("wall_R")
wall_F = game.scene.getSceneNode("wall_F")

#Now it's time to do some transformations:
zerotrans = Transform()
zerotrans.setIdentity()
CP_local_trans = zerotrans
WB_local_trans = cpool.world_transformation.inverted()*wall_B.world_transformation
WL_local_trans = cpool.world_transformation.inverted()*wall_L.world_transformation
WR_local_trans = cpool.world_transformation.inverted()*wall_R.world_transformation
WF_local_trans = cpool.world_transformation.inverted()*wall_F.world_transformation

cp_msh = game.mesh_manager.loadMesh("collision_pool")
cp_shape = ConvexHullShape.create(cp_msh)
wall_msh = game.mesh_manager.loadMesh("wall")
wall_shape = ConvexHullShape.create(wall_msh)
ball_msh = game.mesh_manager.loadMesh("ball")
ball_shape = ConvexHullShape.create(ball_msh)



rblist = []
#Finally we can create compound collision shape:
compound = CompoundShape.create(True)
compound.add_child_shape(CP_local_trans, cp_shape)
compound.add_child_shape(WB_local_trans, wall_shape)
compound.add_child_shape(WL_local_trans, wall_shape)
compound.add_child_shape(WR_local_trans, wall_shape)
compound.add_child_shape(WF_local_trans, wall_shape)

#Then we create the shape as a rigidbody:
compound_body = createRB("collision_pool", compound, 5.0, Vector3(0.0, 0.0, 0.0)) 
rblist.append(compound_body)

#We'll add the balls to the scene as rigidbodies:
nodes = game.scene.scene_nodes
for i in range(0,len(nodes)-1):
    nimi = nodes[i].name
    if nimi.startswith('ball'):
        rblist.append(createRB(nimi, ball_shape, 0.5, Vector3(0.3,0.3,0.3)))


#applying force with key F:
class Sormi:
    def __init__(self, rb, key, magnitude):
        self.rb = rb
        self.force_vector = Vector3(0.0, 0.0, 0.0)
        self.force_magnitude = magnitude
        #game.event_manager.frame_trigger.addListener(progress)
        self.ftrigger = game.event_manager.frame_trigger
        self.ftrigger.addListener(self.progress)

        self.ktrigger = game.event_manager.createKeyTrigger( key )
        self.ktrigger.addKeyUpListener(self.forceup)
        self.ktrigger.addKeyDownListener(self.forcedown)
    def progress(self,t):
        self.rb.applyForce(self.force_vector,Vector3(0.0, 0.0, 0.0))
        print(self.force_vector)
    def forcedown(self):
        self.force_vector.y = self.force_magnitude
    def forceup(self):
        self.force_vector.y = -self.force_magnitude
        
contrl = Sormi(compound_body, KC.F, 1000.0)


"""   
    rigidbodies['bucket'].body.anisotropic_friction = Vector3(1.3, 1.3, 1.3)
    rigidbodies['track_L'].body.anisotropic_friction = Vector3(6.0, 6.0, 6.0)
    rigidbodies['track_R'].body.anisotropic_friction = Vector3(6.0, 6.0, 6.0)
"""
