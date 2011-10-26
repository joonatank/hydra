# -*- coding: utf-8 -*-

""""
Script that defines some common Kinematics helper functions and structures.
Used with our own kinematic animations engine.
Incompatible with physics animation but uses the same syntax.
"""

# Create a translation joint between two bodies along z-axis
# min and max defines the freedom of the joint in meters
# Depending on wether the original coordinates have positive or negative
# z forward, you need to use positive or negative velocity for driving
# positive z forward positive velocity
def createTranslationConstraint(sn0, sn1, transform, min, max) :
	# SliderConstraint works on x-axis, so we rotate the reference
	# z-axis to x-axis
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('slider', body0, body1, transform)
	constraint.lower_limit = min
	constraint.upper_limit = max
	return constraint

# Define a hinge constraint using objects +y axis
# +y because the objects were modeled in Blender with +z as the rotation axis
# but the exporter flips y and z
def createHingeConstraint(sn0, sn1, transform, min = Radian(), max = Radian()) :
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('hinge', body0, body1, transform)
	constraint.lower_limit = Radian(min)
	constraint.upper_limit = Radian(max)
	return constraint

def createFixedConstraint(sn0, sn1, transform) :
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('fixed', body0, body1, transform)
	return constraint

# Constraint functions that work on KinematicBodies and not SceneNodes

# Create a translation joint between two bodies along z-axis
# min and max defines the freedom of the joint in meters
# Depending on wether the original coordinates have positive or negative
# z forward, you need to use positive or negative velocity for driving
# positive z forward positive velocity
def slider_constraint(body0, body1, world_transform, min = 0, max = 0) :
	constraint = game.kinematic_world.create_constraint('slider', body0, body1, world_transform)
	constraint.lower_limit = min
	constraint.upper_limit = max
	return constraint

# Define a hinge constraint using objects +y axis
# +y because the objects were modeled in Blender with +z as the rotation axis
# but the exporter flips y and z
def hinge_constraint(body0, body1, world_transform, min = Radian(), max = Radian()) :
	constraint = game.kinematic_world.create_constraint('hinge', body0, body1, world_transform)
	constraint.lower_limit = Radian(min)
	constraint.upper_limit = Radian(max)
	return constraint

def fixed_constraint(body0, body1):
	# Hard coded transform because it should not matter for fixed constraints
	return game.kinematic_world.create_constraint('fixed', body0, body1, body1.world_transformation)


def _create_body(name) :
	node = game.scene.getSceneNode(name)
	body = game.kinematic_world.create_kinematic_body(node)
	assert(body)
	return body


# Cylinder type actuator
# Modifies the orientation and position of the cylinder rod and piston
# TODO rod and piston are incorrectly named
# rod is used here for the body that is nearer to the root of the kinematic
# chain, piston for the body that is farther away
# example root -> rod_fixing -> rod -> piston -> piston_fixing -> body
# @param rod, SceneNode (needs to have setDirection method)
# @param piston, SceneNode (needs to have setDirection method)
# @param rod_fixing, object with world_transformation property
# @param piston_fixing, object with world_transformation property
# NOTE Cylinder object rod only works correctly if they have an
# offset from their geometric centers. Best for now has been an origin in
# the end where the body is fixed.
# TODO not sure if this true also for piston, all of our models have pistons
# that have origin different than their geometric origin.
class Cylinder:
	def __init__(self, rod, piston, rod_fixing, piston_fixing):
		self.rod= rod
		self.piston = piston
		self.rod_fixing = rod_fixing
		self.piston_fixing = piston_fixing
		self.up_axis = Vector3(1, 0, 0)
		self.local_dir = Vector3(0, -1, 0)
		self.piston_up_axis = self.up_axis
		self.piston_rotation = Quaternion(1, 0, 0, 0)
		self.debug_print = False

		# Rod is a "parent" of rod fixing so the difference needs to be
		# in the rod fixings coordinate frame
		rod_fixing_pos = rod_fixing.world_transformation.position
		rod_pos = rod.world_transformation.position 
		q = self.rod_fixing.world_transformation.quaternion.inverse()
		self.rod_position_diff = q*(rod_fixing_pos - rod_pos)

		# Piston is "child" of the piston fixing so the difference needs to be
		# in its coordinate frame.
		piston_fixing_pos = piston_fixing.world_transformation.position
		piston_pos = piston.world_transformation.position
		q = self.piston.world_transformation.quaternion.inverse()
		self.piston_position_diff = q*(piston_fixing_pos - piston_pos)
		
		rod_fixing.addListener(self.moved)
		# TODO should we add the listener to also piston_fixing
		# this would avoid rather akward problem with the excavator model
		piston_fixing.addListener(self.moved)
	
	# Callback from the object we should follow
	# rod fixing point moved
	def moved(self, trans):
		# we no longer need trans but the callback is type safe
		# so the parameter needs to be there
		# we can just use the fixing points saved here
		# as anyway we need to update both rod and piston here

		# First update the direction because we need to modify
		# the world transformation for the position.
		direction = self.rod_fixing.world_transformation.position - self.piston_fixing.world_transformation.position
		self.rod.set_direction(direction, self.local_dir, self.up_axis)
		# Set position first so the look at uses the correct position
		# difference is in fixing points coordinates
		wt_fixing = self.rod_fixing.world_transformation
		wt = Transform(self.rod.world_transformation)
		# As the rod fixing is the 'parent' of the rod we use it's frame
		q = wt_fixing.quaternion
		wt.position = wt_fixing.position - q*self.rod_position_diff
		if self.debug_print :
			print("Fixing orientation : ", q, " position offset : ", q*self.rod_position_diff)
		self.rod.world_transformation = wt

		# Update piston
		# Pistons position does not change
		# yes it does we need to use the coordine frame from piston_fixing for it
		local_dir = self.local_dir
		piston_up_axis = self.up_axis
		#piston_up_axis = self.piston_up_axis
		self.piston.set_direction(direction, local_dir, piston_up_axis)

		wt_fixing = Transform(self.piston_fixing.world_transformation)
		wt = Transform(self.piston.world_transformation)
		# As the piston is the 'parent' of the fixing we use it's frame
		q = wt.quaternion
		wt.position = wt_fixing.position - q*self.piston_position_diff
		self.piston.world_transformation = wt

		# Hack to handle incorrect rotation of the piston
		self.piston.rotate(self.piston_rotation)

# Rotates the rod and piston around their fixing points
# Trying to align them.
# Problem can be unsolvable if the axis we should rotate around is incorrect
# or if the fixing points move relative to each other in more than one DOF.
# TODO fix naming rod = tube
class Cylinder2 :
	def __init__(self, barrel, rod, barrel_fixing, rod_fixing):
		self.barrel = barrel
		self.rod = rod
		self.barrel_fixing = barrel_fixing
		self.rod_fixing = rod_fixing
		# Around what axis should we rotate
		self.barrel_axis = Vector3(-1, 0, 0)
		self.rod_axis = Vector3(-1, 0, 0)

		# Propably the direction should not be configurable
		self.local_dir = Vector3(0, 0, -1)
		# How much to progress at each step
		self.step_size = Degree(0.1)
		# Debug variable for how many times this was left unsolved
		self.unsolved = 0
		self.max_iterations = 20
		self.times_solve_took = []

		# NOTE using SceneGraph for the solver because we don't
		# have access to the Kinematic graph from python.
		# avoids ineffient copying of transformations.
		self.rod_fixing.scene_node.addChild(self.rod)
		self.barrel_fixing.scene_node.addChild(self.barrel)
		
		self.current_dir = self._get_direction_vec()

		self.barrel_fixing.addListener(self.moved)
		self.rod_fixing.addListener(self.moved)
	
	def __str__(self):
		avg_time = time()
		if len(self.times_solve_took) > 0:
			for t in self.times_solve_took:
				avg_time += t
			avg_time /= len(self.times_solve_took)

		s = "Cylinder : has been left unsolved {:0d} times\n"\
			"   barrel = '{:1s}' and rod '{:2s}'\n"\
			"   step size {:3s}\n"\
			"   took {:4s} at avarage to solve\n"

		return s.format(self.unsolved, self.barrel.name, self.rod.name, self.step_size, avg_time)

	# Callback from fixing points
	def moved(self, trans):
		t = timer()


		# get the direction vector
		direction = self._get_direction_vec()

		# TODO add tolerance
		# TODO break into two parts one for barrel and one for rod
		if self.current_dir != direction:
			barrel_solved = self._run_solver_loop(direction, self.barrel, self.barrel_axis)
			if not barrel_solved:
				barrel_solved = self._run_solver_loop(direction, self.barrel, -self.barrel_axis)

			rod_solved = self._run_solver_loop(direction, self.rod, self.rod_axis)
			if not rod_solved:
				rod_solved = self._run_solver_loop(direction, self.rod, -self.rod_axis)

			if not barrel_solved or not rod_solved:
				self.unsolved += 1
			self.current_dir = self._get_direction_vec() 

		self.times_solve_took.append(t.elapsed())

	# Return true if the problem is solved, false if unsolvable
	def _run_solver_loop(self, direction, node, axis):
		iteration = 0
		not_solved = True
		while self._progress(direction, node, axis):

			# TODO add checking if we are near enough the limit

			++iteration
			# Took too long
			if iteration == self.max_iterations:
				return False

		# The first iteration fails only if we are rotating along the negative axis
		if iteration == 0:
			return False
		return True

	def _get_direction_vec(self):
		d = self.barrel_fixing.world_transformation.position - self.rod_fixing.world_transformation.position
		d.normalise()
		return d

	# Returns true if we have a better result than before, False otherwise
	def _progress(self, desired_dir, node, axis):
		assert(node)

		d_old = node.world_transformation.quaternion * self.local_dir
		
		node.rotate(Quaternion(Radian(self.step_size), axis))
		
		# Check the new direction vector
		d = node.world_transformation.quaternion * self.local_dir

		# Check if new direction vector is closer to desired direction than old one
		if not closer(desired_dir, d_old, d):
			# Pop old result
			node.rotate(Quaternion(Radian(-self.step_size), axis))
			return False
		else :
			return True


# Develop the rotate around method
# FIXME this doesn't work
# TODO create a simple test case for this and move it to global script
def rotate_around(node, q, ref):
	ref_world = ref.world_transformation
	pos = ref_world.position - q*ref_world.position
	t = Transform(q, pos)
	#t = t*Transform(ref_world.quaternion)

	# The problem is that this does not take into account the local
	# frame of the node
	# So if we have different than default orientation on the node this does
	# not work
	node.transformation = node.transformation*t

def closer(compared, old, new):
	if compared.dot(old) < compared.dot(new): return True
	else : return False


