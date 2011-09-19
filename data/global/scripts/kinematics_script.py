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

class Cylinder:
	def __init__(self, rod, piston, rod_fixing, piston_fixing):
		self.rod= rod
		self.piston = piston
		self.rod_fixing = rod_fixing
		self.piston_fixing = piston_fixing
		self.up_axis = Vector3(1, 0, 0)
		self.local_dir = Vector3(0, -1, 0)

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
		t = self.rod_fixing.world_transformation
		wt = Transform(self.rod.world_transformation)
		q = self.rod_fixing.world_transformation.quaternion
		wt.position = t.position - q*self.rod_position_diff
		self.rod.world_transformation = wt

		# Update piston
		# Pistons position does not change
		# yes it does we need to use the coordine frame from piston_fixing for it
		self.piston.set_direction(direction, self.local_dir, self.up_axis)

		piston_fixing_pos = self.piston_fixing.world_transformation.position
		wt = Transform(self.piston.world_transformation)
		q = Quaternion(wt.quaternion)
		wt.position = piston_fixing_pos - q*self.piston_position_diff
		self.piston.world_transformation = wt


