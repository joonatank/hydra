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
	constraint.lower_limit = min
	constraint.upper_limit = max
	return constraint

def createFixedConstraint(sn0, sn1, transform) :
	body0 = game.kinematic_world.create_kinematic_body(sn0)
	body1 = game.kinematic_world.create_kinematic_body(sn1)
	constraint = game.kinematic_world.create_constraint('fixed', body0, body1, transform)
	return constraint

class Cylinder:
	def __init__(self, rod, piston, rod_fixing, piston_fixing):
		self.rod= rod
		self.piston = piston
		self.rod_fixing = rod_fixing
		self.piston_fixing = piston_fixing

		rod_fixing_pos = rod_fixing.world_transformation.position
		rod_pos = rod.world_transformation.position 
		self.rod_position_diff = rod_fixing_pos - rod_pos

		piston_fixing_pos = piston_fixing.world_transformation.position
		piston_pos = piston.world_transformation.position
		self.piston_position_diff = piston_fixing_pos - piston_pos 

		print('piston position diff', self.piston_position_diff)
		print('piston position diff local', piston.world_transformation.quaternion*self.piston_position_diff)

		print('rod', self.rod)
		print('piston', self.piston)

		rod_fixing.addListener(self.moved)
	
	# Callback from the object we should follow
	# rod fixing point moved
	def moved(self, trans):
		# we no longer need trans but the callback is type safe
		# so the parameter needs to be there
		# we can just use the fixing points saved here
		# as anyway we need to update both rod and piston here

		# Set position first so the look at uses the correct position
		# difference is in fixing points coordinates
		t = self.rod_fixing.world_transformation
		self.rod.position = t.position - t.quaternion*self.rod_position_diff

		direction = self.rod_fixing.world_transformation.position - self.piston_fixing.world_transformation.position
		self.rod.set_direction(direction, Vector3(0, -1, 0))

		# Update piston
		# Pistons position does not change
		# yes it does we need to use the coordine frame from piston_fixing for it
		#t = self.piston_fixing.world_transformation
		piston_fixing_pos = self.piston_fixing.world_transformation.position
		# TODO these set local frames even though the positions are in world
		self.piston.set_direction(direction, Vector3(0, -1, 0))

		# FIXME There is a small offset with this
		self.piston.position = piston_fixing_pos - t.quaternion*self.piston_position_diff


