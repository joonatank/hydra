import math
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

def printtaa_resultti(result, nimi):
    obs = result.hit_objects
    for i in obs:
        print("Objekti resultista ",nimi,": ", i.name)
    

def VP_to_ND_cs(mouse_x, mouse_y, width_px, height_px):
    """
    Transforms viewport coordinates (mouse cursor position) to normalized device coordinates/
    homogenous clip coordinates
    """
    x = ( ( ( 2.0*mouse_x ) / width_px ) - 1.0 )
    y = -( ( ( 2.0*mouse_y ) / height_px ) - 1.0 )
    z = 1.0
    ray = Vector3(x, y, z)
    print("NORMALIZED DEVICE COORDS: ", ray)
    return ray

def ND_to_CLIP_cs(ray):
    ray_in_CLIP = Vector4(ray)
    ray_in_CLIP.z = -1.0
    ray_in_CLIP.w = -ray_in_CLIP.z
    print("HOMOGENOUS CLIPPING COORDINATES: ", ray_in_CLIP)
    return ray_in_CLIP

def CLIP_to_EYE_cs(ray, inv_projection_matrix):
    """
    Transforms normalized device coordinates to cyclop/eye coordinates
    """
    ray = inv_projection_matrix*ray
    #point_in_eye_cs = ray.w*Vector3(ray.x, ray.y, -1.0)
    ray_in_eye_cs = ray.w*Vector3(ray.x, ray.y, ray.z)
    #ray_in_eye_cs = ray
    #ray_in_eye_cs.normalise()
    print("EYE COORDINATES: ", ray_in_eye_cs)
    return ray_in_eye_cs

def EYE_to_WORLD_cs(ray, inv_view_matrix):
    ray = inv_view_matrix.to_quaternion.inverse()*ray
    ray_w = Vector3(ray.x,ray.y,ray.z)
    #ray_w.normalise()
    print("IN WORLD COORDS: ", ray)
    return ray_w

def normalise_vec4(vec4):
    lenpower = math.pow(vec4.x, 2) + math.pow(vec4.y, 2) + math.pow(vec4.z, 2) + math.pow(vec4.w, 2)
    length = math.sqrt(lenpower)
    invL = 1.0/length
    return vec4*invL


#GLOBAL
selection_list = []
pickOn = []
"""
class SelectionSet2:
        def __init__(self, selection = set(), selectable_set = set()):
                self.selection = selection
                self.set = selectable_set
        def __str__(self):
                s = "SelectionSet2 :\n"
		s.join(self.selection)
		return s
        def add(self):
            pass
	def round_buffer_next(self):
            
            return
        

class Selection:
    def __init__(self, selection = set()):
        self.selection = selection
        self.center_pivot = Vector3(0.0, 0.0, 0.0)
        self.draw_selection = True    
    def calculate_origin(self):
        pos_sum  = Vector3(0.0, 0.0, 0.0)
        inv_obj_count = 1.0/len(self.selection)
        for obj in self.selection:
            pos_sum = pos_sum + obj.position
        center = pos_sum*inv_obj_count
        self.center_pivot = center
        return center
    def update(self, nodes):
        self.selection.update(nodes)
        self.center_pivot
"""    
def pick_handler(evt,bid):
    if bid is MOUSEBUTTON_ID.MB_L:
        if not pickOn:
            view_matrix = renderer.view_matrix
            view_transform = Transform(view_matrix)
            inv_view_matrix = view_matrix.inverse()
            projection_matrix = renderer.projection_matrix
            inv_projection_matrix = projection_matrix.inverse()
            print("MOUSE COORDS: X: ", evt.axis_x.abs, " Y: ", evt.axis_y.abs)
            #There's a bug which is introduced by windows task panel. It will
            #eat it's height of pixels. So basically if you have exact screen
            #pixels in function argument below it will calculate wrong ray.
            ray_in_ND = VP_to_ND_cs(evt.axis_x.abs, evt.axis_y.abs, 1024, 740)
            #Transform normalized device coordinates to homogenous clipping coordinates:
            ray_in_CLIP = ND_to_CLIP_cs(ray_in_ND)
            #Transform homogenous clipping coordinates to eye/cyclop coordinates:
            ray_in_EYE = CLIP_to_EYE_cs(ray_in_CLIP, inv_projection_matrix)
            #Transform eye coordinates to world coordinates:
            ray_in_WORLD = EYE_to_WORLD_cs(ray_in_EYE, inv_view_matrix)
            print("RAY BEFORE NORMALISATION: ", ray_in_WORLD)
            raylen = ray_in_WORLD.length()
            ray_in_WORLD.normalise()
            #print("RAY AFTER NORMALISATION: ", ray_in_WORLD)
            start_pos = view_transform.position
            end_pos = start_pos + raylen*ray_in_WORLD
            print("START: ", start_pos, " END: ", end_pos)
            result = game.physics_world.cast_ray(start_pos, end_pos)
            printtaa_resultti(result, "MOUSE_EVENT: ")
            if result.hit_objects[0]:
                constraint = create_pick_constraint(result)
                pickOn.append(constraint)
        else:
            pass

def unpick_handler(evt, bid):
    if bid is MOUSEBUTTON_ID.MB_L and pickOn:
        game.physics_world.removeConstraint(pickOn[0])
        
def create_pick_constraint(rayresult):
    bodyB = rayresult.hit_objects[0]
    hp_world = rayresult.hit_points[0]
    hp_local = bodyB.transform_to_local(hp_world)
    constraint = PSixDofConstraint.create(game.physics_world, bodyB, hp_world, hp_local, False)
    return constraint
"""
class Mouse:
    def __init__(self, mouse_trigger):
        mouse_trigger.addMovedListener(self.update)
        mouse_trigger.addButtonDownListener(self.update)
        mouse_trigger.addButtonUpListener(self.update)
    def update_moved(self, evt):
        self.axis_x = evt.axis_x.abs
        self.axis_y = evt.axis_y.abs
        self.printtaa()
    def update(self, evt, bid=None):
        self.axis_x = evt.axis_x.abs
        self.axis_y = evt.axis_y.abs
        self.bid = bid
        self.printtaa()
    def printtaa(self):
        print("X: ", self.axis_x, " Y: ", self.axis_y, " button_ID: ", self.bid) 
 """       


#MAIN STARTS HERE:
world = initphysics()

game.scene.ambient_light = ColourValue(0.3, 0.3, 0.3)
game.scene.shadows.enable()
game.scene.shadows.texture_size = 4096
game.scene.shadows.max_distance = 50

ground = physics_create_ground()
game.scene.sky_dome = SkyDomeInfo("CloudySky")

#We take care of creating mouse trigger for picking purposes:
mtrigger = game.event_manager.createMouseTrigger()
mtrigger.addButtonDownListener(pick_handler)
mtrigger.addButtonUpListener(unpick_handler)
#m_trigu = game.event_manager.createMouseTrigger()
#hiiri = Mouse(m_trigu)

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

#Create shapes of pacmen:
pacman_msh = game.mesh_manager.loadMesh("pacman")
pacman_shape = ConcaveHullShape.create(pacman_msh)
pacman_cube_msh = game.mesh_manager.loadMesh("pacman_cube")
pacman_cube_shape = ConcaveHullShape.create(pacman_cube_msh)

#Now it's time for rigidbodies:
rblist.append(createRB("pacman", pacman_shape, 1.0, Vector3(0.33,0.33,0.33)))
rblist.append(createRB("pacman_cube", pacman_cube_shape, 1.0, Vector3(0.33,0.33,0.33)))

result1 = game.physics_world.cast_ray(Vector3(0.0,2.0,0.0), Vector3(0.0,-2.0,0.0))
result2 = game.physics_world.cast_ray(Vector3(0.0,0.0,10.0), Vector3(0.0,0.0,-10.0))

printtaa_resultti(result1, "pakka1")
printtaa_resultti(result2, "pakka2")

"""
#applying force with key F:
class Sormi:
    def __init__(self, rb, key, magnitude):
        self.rb = rb
        self.force_vector = Vector3(0.0, 0.0, 0.0)
        self.force_magnitude = magnitude
        #game.event_manager.frame_trigger.addListener(progress)
        #self.ftrigger = game.event_manager.frame_trigger
        #self.ftrigger.addListener(self.progress)

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


   
    rigidbodies['bucket'].body.anisotropic_friction = Vector3(1.3, 1.3, 1.3)
    rigidbodies['track_L'].body.anisotropic_friction = Vector3(6.0, 6.0, 6.0)
    rigidbodies['track_R'].body.anisotropic_friction = Vector3(6.0, 6.0, 6.0)

"""
