# Copyright (C) 2005  Michel Reimpell	[ blender24 version ]
# Copyright (C) 2010 Brett Hartshorn		[ blender25 version ]
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


VERSION = 'Ogre Exporter v31'
__devnotes__ = '''
Jun9	. Removed most of the unnecessary functionality by Joonatan Kuosa.

Apr18
	. ported to Blender 2.57 final by Joonatan Kuosa

--final bug fix milestone--

Nov24:
	. fixed triangles flipped wrong
	. fixed .xml_mesh, .xml_skeleton (caused ogrecommandlinetools to fail)
Nov30:
	. fixed multi-mats preview
	. fixed material with use_nodes off but with dangling empty sub-materials
Dec1st:
	. added bone class to wrap blenders bone types
Dec2nd:
	. fixed bug in armature anim, rest pose xml is "rotation" while keyframe is "rotate"
Dec3rd:
	. fixed bug, armature is hidden, can not get into edit_bones
	. fixed bug in armature anim, wrong mult order of rotation pose
	. fixed restore default INFO header
	. fixed vertex colours
	. fixed UV seams
Dec4th:
	. fixed bone index out of order, and allows extra vertex groups
	. started FAQ
	. added dot-rex output
	. fixed vertex normals edge-split mod bug, reported by Sanni
Dec5th:
	. added object hierarchy (preserves parent/child relationships from blender)
Dec8th:
	. generateTangents can not be default True because it requires texture coords
	. supports more than 65,535 vertices per mesh
	. fixed if no normals, then normals set to false (was crashing OgreCommandLineTools)
	. added user data (custom properties) to .scene
	<node>
		<user_data name="mycustprop" type="float" value="1.0"/>
	. updated the FAQ
	. added MeshMagick merge ( but seems useless? )
	. added blender mesh merge
Dec9th:
	. fixing bugs in mesh merge
	. added physics panel
	. added panels for Sensor/Actuator hijacking, and xml output
Dec10th:
	. added support for unpacking texture images
	. fixed Ogre Logic in all tabs - now only in physics
	. added support for classic materials with basic options and Panel
	. fixed meshed with zero faces, or submaterials with no faces
Dec11th:
	. changed tex-slot.offset to be offset and animation to custom prop
	. added material panel
Dec14th:
	. dropped to 6 decimal precision
	. added support for vertex alpha by using second vertex-color-channel named "alpha"
	. added extra tool bake-tex-2-vertex-colors
Dec16th:
	. added collision LOD panel
	. fixed inverse texture scaling (blender inverse of ogre).
	. trying unflipped UV - reported by Reyn
Dec 17th:
	. fixed inverse texture scaling (1.0/x) not (-1.0/x)
	. added respect blender's slot.use (texture slot on/off)
Dec 22nd:
	. added relocate missing textures helper
	. zero faces fixes
	. fixed transparent mats by forcing cull_hardware to none, also ignore material.alpha if use_alpha at texture level.
Dec 23rd:
	. faster material preview in OgreMeshy
Dec 31st:
	. read/write: UUIDs, categories, titles, and notes

-- end of milestones --
-- post fixes --
Jan 6th 2011:
	. updated for blender256
	. fixed Nvidia DDS output
	. add swap axis options

'''

##2.49 code reference: "<quaternion x=\"%.6f\" y=\"%.6f\" z=\"%.6f\" w=\"%.6f\"/>\n" % (rot.x, rot.z, -rot.y, rot.w))
def swap(vec):
	if OPTIONS['SWAP_AXIS'] == 'x y z': return vec
	elif OPTIONS['SWAP_AXIS'] == 'x z y':
		if len(vec) == 3: return mathutils.Vector( [vec.x, vec.z, vec.y] )
		elif len(vec) == 4: return mathutils.Quaternion( [ vec.w, vec.x, vec.z, vec.y] )
	elif OPTIONS['SWAP_AXIS'] == '-x z y':
		if len(vec) == 3: return mathutils.Vector( [-vec.x, vec.z, vec.y] )
		elif len(vec) == 4: return mathutils.Quaternion( [ vec.w, -vec.x, vec.z, vec.y] )
	elif OPTIONS['SWAP_AXIS'] == 'x z -y':
		if len(vec) == 3: return mathutils.Vector( [vec.x, vec.z, -vec.y] )
		elif len(vec) == 4: return mathutils.Quaternion( [ vec.w, vec.x, vec.z, -vec.y] )
	else:
		print( 'unknown swap axis mode', OPTIONS['SWAP_AXIS'] )

_faq_ = '''
Q: my model is over 10,000 faces and it seems to take forever to export it!!!
A: the current code uses an inefficient means for dealing with smooth normals,
	you can make your export 100x faster by making dense models flat shaded.
	(select all; then, View3D->Object Tools->Shading->Flat)
	TIP:
		Try making the model flat shaded and using a subsurf or multi-res modifer,
		this will give you semi-smoothed normals without a speed hit.

Q: i have hundres of objects, is there a way i can merge them on export only?
A: yes, just add them to a group named starting with "merge"

Q: can i use subsurf or multi-res on a mesh with an armature?
A: yes.

Q: can i use subsurf or multi-res on a mesh with shape animation?
A: no.

Q: i don't see any objects when i export?
A: you must select the objects you wish to export.

Q: how can i change the name of my .material file, it is always named Scene.material?
A: rename your scene in blender.  The .material script will follow the name of the scene.

Q: i don't see my animations when exported?
A: make sure you created an NLA strip on the armature, and if you keyed bones in pose mode then only those will be exported.

Q: do i need to bake my IK and other constraints into FK on my armature before export?
A: no.

Q: how do i export extra information to my game engine via OgreDotScene?
A: You can assign 'Custom Properties' to objects in blender, and these will be saved to the .scene file.

Q: i want to use a low-resolution mesh as a collision proxy for my high-resolution mesh, how?
A: make the lowres mesh the child of the hires mesh, and name the low res mesh starting with "collision"

Q: i do not want to use triangle mesh collision, can i use simple collision primitives?
A: yes, go to Properties->Physics->Ogre Physics.  This gets save to the OgreDotScene file.

Q: what version of this script am i running?
A: %s
''' %VERSION


_doc_installing_ = '''
Installing:
	Installing the Addon:
		You can simply copy addon_ogreDotScene.py to your blender installation under blender/2.56/scripts/addons/
		Or you can use blenders interface, under user-prefs, click addons, and click 'install-addon'
		( its a good idea to delete the old version first )

	Required:
		1. blender2.57
'''

## Nov23, tried PyOgre, it sucks - new plan: Tundra

## KeyError: 'the length of IDProperty names is limited to 31 characters'		- chat with Jesterking request 64
## ICONS are in blender/editors/include/UI_icons.h
## TODO exploit pynodes, currently they are disabled pending python3 support: nodes/intern/SHD_dynamic.c
## TODO future idea: render procedural texture to images, more optimal to save settings and reimplment in Ogre, blender could also benifit from GLSL procedural shaders ##
## TODO request IDProperties support for material-nodes
## TODO check out https://github.com/realXtend/naali/blob/develop/bin/pymodules/webserver/webcontroller.py
## TODO are 1e-10 exponent numbers ok in the xml files?
## TODO count verts, and count collision verts
## TODO - image compression browser (previews total size)
## useful for online content - texture load is speed hit


bl_info = {
    "name": "OGRE Exporter (.scene, .mesh, .skeleton)",
    "author": "HartsAntler",
    "version": (0,3,1),
    "blender": (2, 5, 7),
    "location": "INFO Menu",
    "description": "Export to Ogre scene xml and mesh binary formats",
    "warning": "",
    #"wiki_url": "http://wiki.blender.org/index.php/Extensions:2.5/Py/"\
    #"tracker_url": "https://projects.blender.org/tracker/index.php?"\
    "category": "Import-Export"}



import os, sys, time, hashlib, getpass

# customize missing material - red flags for users so they can quickly see what they forgot to assign a material to.
# (do not crash if no material on object - thats annoying for the user)
MISSING_MATERIAL = '''
material _missing_material_ 
{
	receive_shadows off
	technique
	{
		pass
		{
			ambient 0.1 0.1 0.1 1.0
			diffuse 0.8 0.0 0.0 1.0
			specular 0.5 0.5 0.5 1.0 12.5
			emissive 0.3 0.3 0.3 1.0
		}
	}
}
'''


######
# imports
######
import xml.dom.minidom as dom
import math, subprocess
import time
import os

class Timer :
	def __init__(self) :
		self.reset()

	def reset(self) :
		if( os.name == 'nt' ) :
			self.last_time = time.clock()
		else :
			self.last_time = time.time()

	# Returns time since reset in milliseconds
	def elapsed(self) :
		return self.elapsedSecs()*1000

	def elapsedSecs(self) :
		return (self.curr_time() - self.last_time)

	def curr_time(self) :
		if( os.name == 'nt' ) :
			return time.clock()
		else :
			return time.time()

try:
	import bpy, mathutils
	from bpy.props import *
except ImportError:
	sys.exit("This script is an addon for blender, you must install it from blender.")

def get_objects_using_materials( mats ):
	obs = []
	for ob in bpy.data.objects:
		if ob.type == 'MESH':
			for mat in ob.data.materials:
				if mat in mats:
					if ob not in obs: obs.append( ob )
					break
	return obs

def get_materials_using_image( img ):
	mats = []
	for mat in bpy.data.materials:
		for slot in get_image_textures( mat ):
			if slot.texture.image == img:
				if mat not in mats: mats.append( mat )
	return mats

class Ogre_relocate_textures_op(bpy.types.Operator):                
	'''operator: finds missing textures - checks directories with textures to see if missing are there.'''  
	bl_idname = "ogre.relocate_textures"  
	bl_label = "relocate textures"                    
	bl_options = {'REGISTER', 'UNDO'}                              # Options for this panel type

	@classmethod
	def poll(cls, context): return True

	def invoke(self, context, event):
		Report.reset()
		badmats = []
		for img in bpy.data.images:
			url = bpy.path.abspath( img.filepath )
			path,name = os.path.split(url)
			if not name: continue		# user cleared the value

			if os.path.isfile( url ):
				if path not in Report.paths: Report.paths.append( path )
				#Report.messages.append( 'OK: %s' %name )
			else:
				Report.messages.append( 'MISSING: %s' %name )
				found = None
				for p in Report.paths:
					if os.path.isfile( os.path.join(p,name) ):
						Report.messages.append( '  found texture in: %s' %p )
						found = os.path.join( p,name )
						break
				if not found:		# try lower case
					for p in Report.paths:
						name = name.lower()
						if os.path.isfile( os.path.join(p,name) ):
							Report.messages.append( '  found texture in (lower-case): %s' %p )
							found = os.path.join( p,name )
							break
				if found:
					img.filepath = bpy.path.relpath( found )
					Report.messages.append( '  reassigned to -> %s ' %img.filepath )
				else:
					for mat in get_materials_using_image( img ):
						if mat not in badmats:
							badmats.append( mat )

		for ob in get_objects_using_materials(badmats): ob.select=True
		for mat in badmats:
			Report.warnings.append( 'BAD-MATERIAL:  %s' %mat.name )

		if not Report.messages and not badmats: Report.messages.append( 'Everything is OK' )

		bpy.ops.wm.call_menu( name='Ogre_User_Report' )
		return {'FINISHED'}



class ReportSingleton(object):
	def show(self): bpy.ops.wm.call_menu( name='Ogre_User_Report' )
	def __init__(self): self.reset()
	def reset(self):
		self.materials = []
		self.meshes = []
		self.lights = []
		self.cameras = []
		self.armatures = []
		self.armature_animations = []
		self.shape_animations = []
		self.textures = []
		self.vertices = 0
		self.faces = 0
		self.triangles = 0
		self.warnings = []
		self.errors = []
		self.messages = []
		self.paths = []

	def report(self):
		r = ['Report:']
		ex = ['Extended Report:']
		if self.errors:
			r.append( '  ERRORS:' )
			for a in self.errors: r.append( '    . %s' %a )

		#if not bpy.context.selected_objects:
		#	self.warnings.append('YOU DID NOT SELECT ANYTHING TO EXPORT')
		if self.warnings:
			r.append( '  WARNINGS:' )
			for a in self.warnings: r.append( '    . %s' %a )

		if self.messages:
			r.append( '  MESSAGES:' )
			for a in self.messages: r.append( '    . %s' %a )
		if self.paths:
			r.append( '  PATHS:' )
			for a in self.paths: r.append( '    . %s' %a )


		if self.vertices:
			r.append( '  Total Vertices: %s' %self.vertices )
			r.append( '  Total Faces: %s' %self.faces )
			r.append( '  Total Triangles: %s' %self.triangles )
			## TODO report file sizes, meshes and textures

		# TODO fix this to something bit more readable
		for tag in 'meshes lights cameras armatures armature_animations shape_animations materials textures'.split():
			attr = getattr(self, tag)
			if attr:
				name = tag.replace('_',' ').upper()
				r.append( '  %s: %s' %(name, len(attr)) )
				if attr:
					ex.append( '  %s:' %name )
					for a in attr: ex.append( '    . %s' %a )

		txt = '\n'.join( r )
		ex = '\n'.join( ex )		# console only - extended report
		print('_'*80)
		print(txt)
		print(ex)
		print('_'*80)
		return txt

Report = ReportSingleton()

class Program(object):
	def debug( self ):
		print('GPU Program')
		print('  ', self.name)
		print('  ', self.technique)
		print('  ', self.file)
		for p in self.params + self.params_auto: print('    ', p)

	def __init__(self, name, tech, file):
		if len(name) >= 31:		#KeyError: 'the length of IDProperty names is limited to 31 characters'
			if '/' in name: name = name.split('/')[-1]
			if len(name) >= 31: name = name[ : 29 ] + '..'
		self.name = name; self.technique = tech; self.file = file
		self.source = self.entry_point = self.profiles = self.compile_args = self.type = None
		self.params = []; self.params_auto = []
		self.comments = []

	def add_param( self, line, auto=False ):
		name = line.split()[0]
		value_code = line.split()[1]
		p = {'name': name, 'value-code': value_code }
		if auto: self.params_auto.append( p )
		else: self.params.append( p )
		if len( line.split() ) >= 3:
			args = line.split( p['value-code'] )[-1]
			p['args'] = args.strip()

	def get_param( self, name ):
		for p in self.params + self.params_auto:
			if p['name'] == name: return p

class Ogre_User_Report(bpy.types.Menu):
	bl_label = "Mini-Report | (see console for full report)"
	def draw(self, context):
		layout = self.layout
		txt = Report.report()
		for line in txt.splitlines():
			layout.label(text=line)



############## mesh LOD physics #############
class Ogre_Physics_LOD(bpy.types.Panel):
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_context = "data"
	bl_label = "Ogre Collision"
	@classmethod
	def poll(cls, context):
		if context.active_object: return True
		else: return False

	def draw(self, context):
		layout = self.layout
		ob = context.active_object
		if ob.type != 'MESH': return
		game = ob.game
		box = layout.box()

		if ob.name.startswith('collision'):
			if ob.parent:
				box.label(text='object is a collision proxy for: %s' %ob.parent.name)
			else:
				box.label(text='WARNING: collision proxy missing parent')

		else:
			colchild = None
			for child in ob.children:
				if child.name.startswith('collision'): colchild = child; break


			row = box.row()
			row.prop( game, 'use_ghost', text='Disable Collision' )
			if game.use_ghost:
				if ob.show_bounds: ob.show_bounds = False
				if ob.show_wire: ob.show_wire = False
				if colchild and not colchild.hide: colchild.hide = True

			else:
				row.prop(game, "use_collision_bounds", text="Use Collision Primitive")
				row = box.row()

				if game.use_collision_bounds:
					if colchild and not colchild.hide: colchild.hide = True

					row.prop(game, "collision_bounds_type", text="Primitive Type")
					box.prop(game, "collision_margin", text="Collision Margin", slider=True)
					btype = game.collision_bounds_type
					if btype in 'BOX SPHERE CYLINDER CONE CAPSULE'.split():
						if not ob.show_bounds: ob.show_bounds = True
						if ob.draw_bounds_type != btype: ob.draw_bounds_type = btype
						if ob.show_wire: ob.show_wire = False
					elif btype == 'TRIANGLE_MESH':
						if ob.show_bounds: ob.show_bounds = False
						if not ob.show_wire: ob.show_wire = True
						#ob.draw_bounds_type = 'POLYHEDRON'	#whats this?
						box.label(text='(directly using triangles of mesh as collision)')
					else: game.collision_bounds_type = 'BOX'

				else:
					## without these if tests, object is always redrawn and slows down view
					if ob.show_bounds: ob.show_bounds = False
					if ob.show_wire: ob.show_wire = False

					if not colchild:
						box.operator("ogre.create_collision", text="create new collision mesh")
					else:
						if colchild.hide: colchild.hide = False
						if not colchild.select:
							colchild.hide_select = False
							colchild.select = True
							colchild.hide_select = True

						row.label(text='collision proxy name: %s' %colchild.name)
						if colchild.hide_select:
							row.prop( colchild, 'hide_select', text='', icon='LOCKED' )
						else:
							row.prop( colchild, 'hide_select', text='', icon='UNLOCKED' )

						decmod = None
						for mod in colchild.modifiers:
							if mod.type == 'DECIMATE': decmod = mod; break
						if not decmod:
							decmod = colchild.modifiers.new('LOD', type='DECIMATE')
							decmod.ratio = 0.5

						if decmod:
							#print(dir(decmod))
							#row = box.row()
							box.prop( decmod, 'ratio', 'vertex reduction ratio' )
							box.label(text='faces: %s' %decmod.face_count )

class Ogre_create_collision_op(bpy.types.Operator):                
	'''operator: creates new collision'''  
	bl_idname = "ogre.create_collision"  
	bl_label = "create collision mesh"                    
	bl_options = {'REGISTER', 'UNDO'}                              # Options for this panel type

	@classmethod
	def poll(cls, context): return True
	def invoke(self, context, event):
		parent = context.active_object
		child = parent.copy()
		bpy.context.scene.objects.link( child )
		child.name = 'collision'
		child.matrix_local = mathutils.Matrix()
		child.parent = parent
		child.hide_select = True
		child.draw_type = 'WIRE'
		#child.select = False
		child.lock_location = [True]*3
		child.lock_rotation = [True]*3
		child.lock_scale = [True]*3
		return {'FINISHED'}


##################################################################
_game_logic_intro_doc_ = '''
Hijacking the BGE

Blender contains a fully functional game engine (BGE) that is highly useful for learning the concepts of game programming by breaking it down into three simple parts: Sensor, Controller, and Actuator.  An Ogre based game engine will likely have similar concepts in its internal API and game logic scripting.  Without a custom interface to define game logic, very often game designers may have to resort to having programmers implement their ideas in purely handwritten script.  This is prone to breakage because object names then end up being hard-coded.  Not only does this lead to non-reusable code, its also a slow process.  Why should we have to resort to this when Blender already contains a very rich interface for game logic?  By hijacking a subset of the BGE interface we can make this workflow between game designer and game programmer much better.

The OgreDocScene format can easily be extened to include extra game logic data.  While the BGE contains some features that can not be easily mapped to other game engines, there are many are highly useful generic features we can exploit, including many of the Sensors and Actuators.  Blender uses the paradigm of: 1. Sensor -> 2. Controller -> 3. Actuator.  In pseudo-code, this can be thought of as: 1. on-event -> 2. conditional logic -> 3. do-action.  The designer is most often concerned with the on-events (the Sensors), and the do-actions (the Actuators); and the BGE interface provides a clear way for defining and editing those.  Its a harder task to provide a good interface for the conditional logic (Controller), that is flexible enough to fit everyones different Ogre engine and requirements, so that is outside the scope of this exporter at this time.  A programmer will still be required to fill the gap between Sensor and Actuator, but hopefully his work is greatly reduced and can write more generic/reuseable code.

The rules for which Sensors trigger which Actuators is left undefined, as explained above we are hijacking the BGE interface not trying to export and reimplement everything.  BGE Controllers and all links are ignored by the exporter, so whats the best way to define Sensor/Actuator relationships?  One convention that seems logical is to group Sensors and Actuators by name.  More complex syntax could be used in Sensor/Actuators names, or they could be completely ignored and instead all the mapping is done by the game programmer using other rules.  This issue is not easily solved so designers and the engine programmers will have to decide upon their own conventions, there is no one size fits all solution.
'''


class Ogre_Physics(bpy.types.Panel):
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_context = "physics"
	bl_label = "Ogre Physics"

	@classmethod
	def poll(cls, context):
		if context.active_object: return True
		else: return False

	def draw(self, context):
		layout = self.layout
		ob = context.active_object
		game = ob.game

		#if game.physics_type:	# in ('DYNAMIC', 'RIGID_BODY'):
		split = layout.split()

		col = split.column()
		col.prop(game, "physics_type", text='Type')
		col.prop(game, "use_actor")
		col.prop(game, "use_ghost")

		col = split.column()
		col.prop(game, "use_collision_bounds", text="Use Primitive Collision")
		col.prop(game, "collision_bounds_type", text="Primitive Type")
		col.prop(game, "collision_margin", text="Collision Margin", slider=True)
		#col.prop(game, "use_collision_compound", text="Compound")		TODO

		layout.separator()

		split = layout.split()

		col = split.column()
		col.label(text="Attributes:")
		col.prop(game, "mass")
		col.prop(game, "radius")
		col.prop(game, "form_factor")

		col = split.column()
		sub = col.column()
		#sub.active = (game.physics_type == 'RIGID_BODY')
		sub.prop(game, "use_anisotropic_friction")
		subsub = sub.column()
		subsub.active = game.use_anisotropic_friction
		subsub.prop(game, "friction_coefficients", text="", slider=True)

		split = layout.split()

		col = split.column()
		col.label(text="Velocity:")
		sub = col.column(align=True)
		sub.prop(game, "velocity_min", text="Minimum")
		sub.prop(game, "velocity_max", text="Maximum")

		col = split.column()
		col.label(text="Damping:")
		sub = col.column(align=True)
		sub.prop(game, "damping", text="Translation", slider=True)
		sub.prop(game, "rotation_damping", text="Rotation", slider=True)

		layout.separator()

		split = layout.split()

		col = split.column()
		col.prop(game, "lock_location_x", text="Lock Translation: X")
		col.prop(game, "lock_location_y", text="Lock Translation: Y")
		col.prop(game, "lock_location_z", text="Lock Translation: Z")

		col = split.column()
		col.prop(game, "lock_rotation_x", text="Lock Rotation: X")
		col.prop(game, "lock_rotation_y", text="Lock Rotation: Y")
		col.prop(game, "lock_rotation_z", text="Lock Rotation: Z")


##################################################################

OPTIONS = {
	'FORCE_IMAGE_FORMAT' : None,
	'TEXTURES_SUBDIR' : False,
	'PATH' : '/tmp',
	'TOUCH_TEXTURES' : False,
	'SWAP_AXIS' : '-x z y',
}

_ogre_doc_classic_textures_ = '''
Ogre texture blending is far more limited than Blender's texture slots.  While many of the blending options are the same or similar, only the blend mode "Mix" is allowed to have a variable setting.  All other texture blend modes are either on or off, for example you can not use the "Add" blend mode and set the amount to anything other than fully on (1.0).  The user also has to take into consideration the hardware multi-texturing limitations of their target platform - for example the GeForce3 can only do four texture blend operations in a single pass.  Note that Ogre will fallback to multipass rendering when the hardware won't accelerate it.

==Supported Blending Modes:==
	* Mix				- blend_manual -
	* Multiply		- modulate -
	* Screen			- modulate_x2 -
	* Lighten		- modulate_x4 -
	* Add				- add -
	* Subtract		- subtract -
	* Overlay		- add_signed -
	* Difference	- dotproduct -

==Mapping Types:==
	* UV
	* Sphere environment mapping
	* Flat environment mapping

==Animation:==
	* scroll animation
	* rotation animation
'''

TextureUnitAnimOps = {	## DEPRECATED
	'scroll_anim' : '_xspeed _yspeed'.split(),
	'rotate_anim' : ['_revs_per_second'],
	'wave_xform' : '_xform_type _wave_type _base _freq _phase _amp'.split(),
}

#	exmodes = 'one zero dest_colour src_colour one_minus_dest_colour dest_alpha src_alpha one_minus_dest_alpha one_minus_src_alpha'.split()
ogre_scene_blend_types =  [
	('one zero', 'one zero', 'solid default'),
	('alpha_blend', 'alpha_blend', 'basic alpha'),
	('add', 'add', 'additive'),
	('modulate', 'modulate', 'multiply'),
	('colour_blend', 'colour_blend', 'blend colors'),
]
for mode in 'dest_colour src_colour one_minus_dest_colour dest_alpha src_alpha one_minus_dest_alpha one_minus_src_alpha'.split():
	ogre_scene_blend_types.append( ('one %s'%mode, 'one %s'%mode, '') )
del mode

bpy.types.Material.scene_blend = EnumProperty(
	items=ogre_scene_blend_types, 
	name='scene blend', 
	description='blending operation of material to scene', 
	default='one zero'
)

class Ogre_Material_Panel( bpy.types.Panel ):
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_context = "material"
	bl_label = "Ogre Material"

	def draw(self, context):
		if not hasattr(context, "material"): return
		if not context.active_object: return
		if not context.active_object.active_material: return

		mat = context.material
		ob = context.object
		slot = context.material_slot
		layout = self.layout

		box = layout.box()
		row = box.row()
		row.prop(mat, "diffuse_color")
		row.prop(mat, "diffuse_intensity")
		row = box.row()
		row.prop(mat, "specular_color")
		row.prop(mat, "specular_intensity")
		row = box.row()
		row.prop(mat, "specular_hardness")

		row = box.row()
		row.prop(mat, "emit")
		row.prop(mat, "ambient")

		row = box.row()
		row.prop(mat, "use_shadows")
		row.prop(mat, "use_vertex_color_paint", text="Vertex Colors")


		box = layout.box()
		row = box.row()
		row.prop(mat, "use_transparency", text="Transparent")
		if mat.use_transparency: row.prop(mat, "alpha")
		box.prop(mat, 'scene_blend')


def has_property( a, name ):
	for prop in a.items():
		n,val = prop
		if n == name: return True


class Ogre_Texture_Panel(bpy.types.Panel):
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_context = "texture"
	bl_label = "Ogre Texture"
	@classmethod
	def poll(cls, context):
		if not hasattr(context, "texture_slot"):
			return False
		else: return True

	def draw(self, context):
		#if not hasattr(context, "texture_slot"):
		#	return False
		layout = self.layout
		#idblock = context_tex_datablock(context)
		slot = context.texture_slot
		if not slot or not slot.texture: return

		box = layout.box()
		row = box.row()
		row.label(text="Mapping:")
		row.prop(slot, "texture_coords", text="")
		if slot.texture_coords == 'UV':
			row.label(text="UV Layer:")
			row.prop(slot, "uv_layer", text="")
		else:
			row.label(text="Projection:")
			row.prop(slot, "mapping", text="")

		if hasattr(slot.texture, 'image') and slot.texture.image:
			row = box.row()
			row.label(text="Repeat Mode:")
			row.prop(slot.texture, "extension", text="")
			if slot.texture.extension == 'CLIP':
				row.label(text="Border Color:")
				row.prop(slot, "color", text="")

		box = layout.box()
		row = box.row()
		row.label(text="Blending:")
		row.prop(slot, "blend_type", text="")
		row.label(text="Alpha Stencil:")
		row.prop(slot, "use_stencil", text="")
		row = box.row()
		if slot.blend_type == 'MIX':
			row.label(text="Mixing:")
			row.prop(slot, "diffuse_color_factor", text="")
			#row.label(text="Enable:")
			#row.prop(slot, "use_map_color_diffuse", text="")

		row = box.row()
		row.label(text="Enable Alpha:")
		row.prop(slot, "use_map_alpha", text="")
		if context.active_object and context.active_object.active_material:
			row.label(text="Transparent:")
			row.prop(context.active_object.active_material, "use_transparency", text="")
			

		box = layout.box()
		box.prop(slot, "offset", text="X,Y = offset.  Z=rotation")

		box = layout.box()
		box.prop(slot, "scale", text="Scale in X,Y.   (Z ignored)")

		box = layout.box()
		row = box.row()
		row.label(text='scrolling animation')
		#cant use if its enabled by defaultrow.prop(slot, "use_map_density", text="")
		row.prop(slot, "use_map_scatter", text="")
		row = box.row()
		row.prop(slot, "density_factor", text="X")
		row.prop(slot, "emission_factor", text="Y")

		box = layout.box()
		row = box.row()
		row.label(text='rotation animation')
		row.prop(slot, "emission_color_factor", text="")
		row.prop(slot, "use_map_emission", text="")


def guess_uv_layer( layer ):
	## small issue: in blender layer is a string, multiple objects may have the same material assigned, 
	## but having different named UVTex slots, most often the user will never rename these so they get
	## named UVTex.000 etc...   assume this to always be true.
	idx = 0
	if '.' in layer:
		a = layer.split('.')[-1]
		if a.isdigit(): idx = int(a)+1
		else: print('warning: it is not allowed to give custom names to UVTexture channels ->', layer)
	return idx

class ShaderTree(object):
	'''
	Sometimes high resolution renderings may be required.  The user has the option of using multiple Output nodes, blender will only consider the first one, the second if named starting with 'ogre' will be used by the exporter.  This allows the user to have two branches in their node setup, one for blender software rendering, the other for Ogre output.  Also useful for baking procedurals, which then can be used by the ogre branch.
	'''
	@staticmethod
	def valid_node_material( mat ):
		# just incase the user enabled nodes but didn't do anything, then disabled nodes
		if mat.node_tree and len(mat.node_tree.nodes):
			for node in mat.node_tree.nodes:
				if node.type == 'MATERIAL':
					if node.material: return True

	Materials = []
	Output = None
	@staticmethod
	def parse( mat ):		# only called for noded materials
		print('		parsing node_tree')
		ShaderTree.Materials = []
		ShaderTree.Output = None
		outputs = []
		for link in mat.node_tree.links:
			if link.to_node and link.to_node.type == 'OUTPUT': outputs.append( link )

		root = None
		for link in outputs:
			if root is None or link.to_node.name.lower().startswith('ogre'): root = link
		if root:
			ShaderTree.Output = root.to_node
			print('setting Output node', root.to_node)
			tree = ShaderTree( node=root.to_node, parent_material=mat )
			return tree
		else:
			print('warning: no Output shader node found')

	def __init__(self, node=None, material=None, parent_material=None ):
		if node: print('		shader node ->', node)
		if node and node.type.startswith('MATERIAL'):
			ShaderTree.Materials.append( self )
			self.material = node.material
		elif material:		# standard material
			self.material = material
			self.textures = []

		self.node = node
		if node:
			self.type = node.type
			self.name = node.name
		self.children = []
		self.parents = []
		self.inputs = {}		# socket name : child
		self.outputs = {}	# parent : socket name
		if parent_material:
			for link in parent_material.node_tree.links:
				if link.to_node and link.to_node.name == self.name:
					branch = ShaderTree(
						node=link.from_node, 
						parent_material=parent_material
					)
					self.children.append( branch )
					self.inputs[ link.to_socket.name ] = branch
					branch.outputs[ self ] = link.from_socket.name
					branch.parents.append( self )

	def ancestors(self):
		if not self.parents: return []
		else: c = []; self._ancestors(c); return c
	def _ancestors(self, c):
		for p in self.parents: c.append( p ); p._ancestors( c )

	def decendents(self):
		if not self.children: return []
		else: c = []; self._decendents(c); return c
	def _decendents(self, c):
		for p in self.children: c.append( p ); p._decendents( c )

	def is_ogre_branch( self ):
		ancestors = []
		self._ancestors( ancestors )
		for parent in ancestors:
			if parent.node.name == ShaderTree.Output.name: return True
		print('node not in ogre branch', self.node)


	## returns height sorted materials, 'passes' in Ogre speak ##
	# called after tree=ShaderTree.parse( nodedmat ); mats=tree.get_passes()
	def get_passes( self ):
		mats = []
		for mat in ShaderTree.Materials:
			print('			checking material ancestors:', mat)
			# only consider materials that are connected to the ogre Output
			#if self.Output in ancestors:
			if mat.is_ogre_branch():
				print('			material is in ogre branch->', mat)
				mats.append( mat )
		mats.sort( key=lambda x: x.node.location.y, reverse=True )
		if not mats: print('WARNING: no materials connected to Output node')
		## collect and sort textures of a material ##
		for mat in mats:
			mat.textures = []
			d = mat.decendents()
			for child in d:
				if child.node.type == 'TEXTURE': mat.textures.append( child )
			mat.textures.sort( key=lambda x: x.node.location.y, reverse=True )
		return mats

	def get_texture_attributes(self):
		M = ''
		ops = {}
		for prop in self.node.texture.items():
			name,value = prop
			ops[name]=value
			M += indent(4, '%s %s' %prop )

		d = self.decendents()
		for child in d:
			if child.type == 'GEOMETRY' and child.node.uv_layer:
				idx = guess_uv_layer( child.node.uv_layer )
				M += indent(4, 'tex_coord_set %s' %idx)

			elif child.type == 'MAPPING':
				snode = child.node
				x,y,z = snode.location			# bpy bug, mapping node has two .location attrs
				if x or y:
					M += indent(4, 'scroll %s %s' %(x,y))
				angle = math.degrees(snode.rotation.x)
				if angle:
					M += indent(4, 'rotate %s' %angle)
				x,y,z = snode.scale
				if x != 1.0 or y != 1.0:
					M += indent(4, 'scale %s %s' %(1.0/x,1.0/y))	# reported by Sanni and Reyn

		return M

	def dotmat_texture(self, texture, texwrapper=None, slot=None):
		if not hasattr(texture, 'image'):
			print('WARNING: texture must be of type IMAGE->', texture)
			return ''

		if not texture.image:
			print('WARNING: texture has no image assigned->', texture)
			return ''

		#if slot: print(dir(slot))
		if slot and not slot.use:
			return ''

		path = OPTIONS['PATH']
		M = ''; _alphahack = None
		M += indent(3, 'texture_unit', '{' )

		iurl = bpy.path.abspath( texture.image.filepath )
		postname = texname = os.path.split(iurl)[-1]
		destpath = path

		## packed images - dec10th 2010 ##
		if texture.image.packed_file:
			orig = texture.image.filepath
			if sys.platform.startswith('linux'):
				iurl = '/tmp/%s' %texname
			else:
				iurl = 'C:\\tmp\\%s' %texname
				if not os.path.isdir( 'C:\\tmp' ):
					print('creating tmp directory' )
					os.makedirs( 'C:\\tmp' )
			if not os.path.isfile( iurl ):
				print('MESSAGE: unpacking image: ', iurl)
				texture.image.filepath = iurl
				texture.image.save()
				texture.image.filepath = orig
			else:
				print('MESSAGE: packed image already in temp, not updating', iurl)

		if OPTIONS['FORCE_IMAGE_FORMAT']: postname = self._reformat( texname )		#texname.split('.')[0]+'.dds'
		if OPTIONS['TEXTURES_SUBDIR']:
			destpath = os.path.join(path,'textures')
			if not os.path.isdir( destpath ): os.mkdir( destpath )
			M += indent(4, 'texture textures/%s' %postname )	
		else: 
			M += indent(4, 'texture %s' %postname )	

		exmode = texture.extension
		if exmode == 'REPEAT':
			M += indent(4, 'tex_address_mode wrap' )
		elif exmode == 'EXTEND':
			M += indent(4, 'tex_address_mode clamp' )
		elif exmode == 'CLIP':
			M += indent(4, 'tex_address_mode border' )
		elif exmode == 'CHECKER':
			M += indent(4, 'tex_address_mode mirror' )

		if texwrapper:	# shader node options
			M += texwrapper.get_texture_attributes()

		elif slot:		# class blender material slot options
			if exmode == 'CLIP': M += indent(4, 'tex_border_colour %s %s %s' %(slot.color.r, slot.color.g, slot.color.b) )	
			M += indent(4, 'scale %s %s' %(1.0/slot.scale.x, 1.0/slot.scale.y) )	# thanks Reyn
			if slot.texture_coords != 'UV':
				if slot.mapping == 'SPHERE':
					M += indent(4, 'env_map spherical' )
				elif slot.mapping == 'FLAT':
					M += indent(4, 'env_map planar' )
				else: print('WARNING: <%s> has a non-UV mapping type (%s) and not picked a proper projection type of: Sphere or Flat' %(texture.name, slot.mapping))

			ox,oy,oz = slot.offset
			if ox or oy:
				M += indent(4, 'scroll %s %s' %(ox,oy) )
			if oz:
				M += indent(4, 'rotate %s' %oz )

			if slot.use_map_emission:	# hijacked from volume shaders
				M += indent(4, 'rotate_anim %s' %slot.emission_color_factor )
			if slot.use_map_scatter:	# hijacked from volume shaders
				M += indent(4, 'scroll_anim %s %s ' %(slot.density_factor, slot.emission_factor) )


			## set uv layer
			if slot.uv_layer:
				idx = guess_uv_layer( slot.uv_layer )
				M += indent(4, 'tex_coord_set %s' %idx)

			rgba = False
			if texture.image.depth == 32: rgba = True

			btype = slot.blend_type

			if rgba and slot.use_stencil:
				texop = 'blend_current_alpha'		# 'blend_texture_alpha' shadeless
			elif btype == 'MIX':
				texop = 'blend_manual'
			elif btype == 'MULTIPLY':
				texop = 'modulate'
			elif btype == 'SCREEN':
				texop = 'modulate_x2'
			elif btype == 'LIGHTEN':
				texop = 'modulate_x4'
			elif btype == 'ADD':
				texop = 'add'
			elif btype == 'SUBTRACT':
				texop = 'subtract'
			elif btype == 'OVERLAY':
				texop = 'add_signed'		# add_smooth not very useful?
			elif btype == 'DIFFERENCE':
				texop = 'dotproduct'		# nothing closely matches blender
			elif btype == 'VALUE':
				texop = 'value'
			else:
				texop = 'blend_diffuse_colour'

			#factor = .0
			#if slot.use_map_color_diffuse:
			factor = 1.0 - slot.diffuse_color_factor

			if texop == 'blend_manual':
				M += indent(4, 'colour_op_ex %s src_current src_texture %s' %(texop, factor) )
			elif not texop == 'value':
				# For value do not do anything
				M += indent(4, 'colour_op_ex %s src_current src_texture' %texop )
				#M += indent(4, 'colour_op_ex %s src_manual src_diffuse %s' %(texop, 1.0-factor) )
				#M += indent(4, 'alpha_op_ex blend_manual src_current src_current %s' %factor )

			if slot.use_map_alpha:
				#alphafactor = 1.0 - slot.alpha_factor
				#M += indent(4, 'colour_op_ex blend_manual src_current src_texture %s' %factor )
				M += indent(4, 'alpha_op_ex modulate src_current src_texture' )


		#else:		# fallback to default options
		#	M += indent(4, 'filtering trilinear' )

		M += indent(3, '}' )	# end texture

		if OPTIONS['TOUCH_TEXTURES']:
			## copy texture only if newer ##
			if not os.path.isfile( iurl ): Report.warnings.append( 'missing texture: %s' %iurl )
			else:
				desturl = os.path.join( destpath, texname )
				if not os.path.isfile( desturl ) or os.stat( desturl ).st_mtime < os.stat( iurl ).st_mtime:
					f = open( desturl, 'wb' )
					f.write( open(iurl,'rb').read() )
					f.close()
				if OPTIONS['FORCE_IMAGE_FORMAT']:		# bug fix jan7th 2011
					if OPTIONS['FORCE_IMAGE_FORMAT'] == '.dds': self.DDS_converter( desturl )
					else: self.image_magick( desturl )

		return M


	def dotmat_pass(self):	# must be a material
		if not self.material:
			print('ERROR: material node with no submaterial block chosen')
			return ''

		mat = self.material
		color = mat.diffuse_color
		alpha = 1.0
		if mat.use_transparency:
			alpha = mat.alpha

		## textures ##
		if not self.textures:		## class style materials
			slots = get_image_textures( mat )		# returns texture_slot object
			usealpha = False
			for slot in slots:
				if slot.use_map_alpha and slot.texture.use_alpha: usealpha = True; break
		if usealpha: alpha = 1.0

		# python note, define inline function shares variables - copies?
		def _helper( child, opname, f ):
			if child.type == 'RGB':
				print('warning: RGB shader node bpy rna is incomplete, missing color attributes' )
				return indent(3, '%s %s %s %s %s' %(opname, color.r*f, color.g*f, color.b*f, alpha) )
			elif child.type == 'GEOMETRY':
				if child.outputs[self] != 'Vertex Color':
					print('warning: you are supposed to connect the vertex color output of geometry')
				return indent(3, '%s vertexcolour' %opname)
			elif child.type == 'TEXTURE':
				print( 'TODO: connecting a texture to this ',
					'slot will be supported for program-shaders in the future' )
				#return indent(3, '%s 1.0 0.0 0.0 1.0' %opname)
				return indent(3, '%s %s %s %s %s' %(opname, color.r*f, color.g*f, color.b*f, alpha) )

		M = ''
		if self.node:		# ogre combines passes with the same name, be careful!
			passname = '%s__%s' %(self.node.name,mat.name)
			passname = passname.replace(' ','_')
			M += indent(2, 'pass %s' %passname, '{' )		# be careful with pass names
		else:
			M += indent(2, 'pass', '{' )

		# TODO directx and opengl are reversed, how to deal with this?
		M += indent(3, 'cull_hardware none' )	

		f = mat.ambient
		if 'Ambient' in self.inputs:
			child = self.inputs['Ambient']
			M += _helper( child, 'ambient', f )
		elif mat.use_vertex_color_paint:
			M += indent(3, 'ambient vertexcolour' )
		else:		# fall back to basic material
			M += indent(3, 'ambient %s %s %s %s' %(color.r*f, color.g*f, color.b*f, alpha) )

		f = mat.diffuse_intensity
		if 'Color' in self.inputs:
			child = self.inputs['Color']
			M += _helper( child, 'diffuse', f )
		elif mat.use_vertex_color_paint:
			M += indent(3, 'diffuse vertexcolour' )
		else:		# fall back to basic material 
			M += indent(3, 'diffuse %s %s %s %s' %(color.r*f, color.g*f, color.b*f, alpha) )

		f = mat.specular_intensity
		if 'Spec' in self.inputs:
			child = self.inputs['Spec']
			M += _helper( child, 'specular', f ) + ' %s'%(mat.specular_hardness/4.0)
		else:
			s = mat.specular_color
			M += indent(3, 'specular %s %s %s %s %s' %(s.r*f, s.g*f, s.b*f, alpha, mat.specular_hardness/4.0) )

		f = mat.emit		# remains valid even if material_ex inputs a color node
		if 'Emit' in self.inputs:
			child = self.inputs['Emit']
			M += _helper( child, 'emissive', f )
		elif mat.use_vertex_color_light:
			M += indent(3, 'emissive vertexcolour' )
		else:
			M += indent(3, 'emissive %s %s %s %s' %(color.r*f, color.g*f, color.b*f, alpha) )

		if mat.use_transparency:
			M += indent( 3, 'scene_blend %s' %'alpha_blend')
			M += indent( 3, 'depth_write %s' %'off')
		else:
			M += indent( 3, 'scene_blend %s' %mat.scene_blend )

		for prop in mat.items():
			name,val = prop
			if not name.startswith('_'): M += indent( 3, '%s %s' %prop )

		## textures ##
		if not self.textures:		## class style materials
			slots = get_image_textures( mat )		# returns texture_slot object
			usealpha = False
			for slot in slots:
				#if slot.use_map_alpha and slot.texture.use_alpha: usealpha = True; break
				if slot.use_map_alpha: usealpha = True; break

			if usealpha:
				# defined only once per pass (global attributes)
				if mat.use_transparency: M += indent(3, 'depth_write off' )
		elif self.node:		# shader nodes
			for wrap in self.textures:
				M += self.dotmat_texture( wrap.node.texture, texwrapper=wrap )

		M += indent(2, '}' )	# end pass
		return M

	############################################
	def _reformat( self, image ):
		return image[ : image.rindex('.') ] + OPTIONS['FORCE_IMAGE_FORMAT']


SELECTED_MATERIAL_NODE = None
SELECTED_TEXTURE_NODE = None

#############################

def wordwrap( txt ):
	r = ['']
	for word in txt.split(' '):	# do not split on tabs
		word = word.replace('\t', ' '*3)
		r[-1] += word + ' '
		if len(r[-1]) > 90: r.append( '' )
	return r

_OGRE_DOCS_ = []
_OGRE_SHADER_REF_ = []
_OGRE_SHADER_REF_TEX_ = []
def ogredoc( cls ):
	tag = cls.__name__.split('_ogredoc_')[-1]

	if tag.startswith('Shader_Nodes_'):
		cls.bl_label = cls.ogre_shader_op = tag.split('Shader_Nodes_')[-1]
		cls.ogre_shader_params = []
		if cls.ogre_shader_op not in 'ambient diffuse specular emissive'.split():
			for line in cls.mydoc.splitlines():
				if line.strip().startswith('@'):		# custom markup
					cls.ogre_shader_params.append( line.strip()[1:] )
		_OGRE_SHADER_REF_.append( cls )
	elif tag.startswith('TEX_'):
		cls.bl_label = cls.ogre_shader_tex_op = tag.split('TEX_')[-1]
		cls.ogre_shader_params = []
		if cls.ogre_shader_tex_op not in 'texture tex_coord_set'.split():
			for line in cls.mydoc.splitlines():
				if line.strip().startswith('@'):		# custom markup
					cls.ogre_shader_params.append( line.strip()[1:] )
		_OGRE_SHADER_REF_TEX_.append( cls )

	else:
		cls.bl_label = tag.replace('_', ' ')
		_OGRE_DOCS_.append( cls )
	return cls


class INFO_MT_ogre_helper(bpy.types.Menu):
	bl_label = 'ogre_helper'
	def draw(self, context):
		layout = self.layout

		for line in self.mydoc.splitlines():
			if line.strip():
				for ww in wordwrap( line ): layout.label(text=ww)
		layout.separator()


class INFO_MT_ogre_docs(bpy.types.Menu):
	bl_label = "Ogre Help"
	def draw(self, context):
		layout = self.layout
		for cls in _OGRE_DOCS_:
			layout.menu( cls.__name__ )
			layout.separator()
		layout.separator()
		layout.label(text='bug reports to: bhartsho@yahoo.com')

@ogredoc
class _ogredoc_Installing( INFO_MT_ogre_helper ):
	mydoc = _doc_installing_

@ogredoc
class _ogredoc_FAQ( INFO_MT_ogre_helper ):
	mydoc = _faq_

@ogredoc
class _ogredoc_Exporter_Features( INFO_MT_ogre_helper ):
	mydoc = '''
Ogre Exporter Features:
	Export .scene:
		pos, rot, scl
		environment colors
		fog settings
		lights, colors
		array modifier (constant offset only)
		optimize instances
		selected only
		force cameras
		force lamps
		BGE physics
		collisions prims and external meshes

	Export .mesh

		verts, normals, uv
		export `meshes` subdirectory
		bone weights
		shape animation (using NLA-hijacking)

	Export .material
		diffuse color
		ambient intensity
		emission
		specular
		receive shadows on/off
		multiple materials per mesh
		exports `textures` subdirectory

	Export .skeleton
		bones
		animation
		multi-tracks using NLA-hijacking
'''

@ogredoc
class _ogredoc_Texture_Options( INFO_MT_ogre_helper ):
	mydoc = _ogre_doc_classic_textures_

@ogredoc
class _ogredoc_Animation_System( INFO_MT_ogre_helper ):
	mydoc = '''
Armature Animation System | OgreDotSkeleton
	Quick Start:
		1. select your armature and set a single keyframe on the object (loc,rot, or scl)
			. note, this step is just a hack for creating an action so you can then create an NLA track.
			. do not key in pose mode, unless you want to only export animation on the keyed bones.
		2. open the NLA, and convert the action into an NLA strip
		3. name the NLA strip
		4. set the in and out frames for the strip
		** note that you DO NOT need to bake your constaint animation, you can keep the constaints, simply export!

	The OgreDotSkeleton (.skeleton) format supports multiple named tracks that can contain some or all of the bones of an armature.  This feature can be exploited by a game engine for segmenting and animation blending.  For example: lets say we want to animate the upper torso independently of the lower body while still using a single armature.  This can be done by hijacking the NLA of the armature.

	NLA Hijacking:
		. define an action and keyframe the bones you want to 'group', ie. key all the upper torso bones

		. import the action into the NLA
		. name the NLA track (this becomes the track name in Ogre)
		. adjust the start and end frames of the strip (only the first strip is considered)

	Benefits:
		. cleaner and lighter source code
		. blender user can stay within blender's interface, that they are already familiar with

		. frame ranges and groupings are saved in the blend file and easy to adjust


	Cons:
		. the user has to remember which bones were keyed per action grouping
			...if the user names their actions clearly, then its not an issue
		. what if the user wants to mix constraints and forward kinematics driven by the NLA?
			...can work if the track is not muted...
			...do constraints override the NLA?

'''

@ogredoc
class _ogredoc_Unresolved_Development_Issues( INFO_MT_ogre_helper ):
	mydoc = '''
Unresolved Development Issues:

 1. Texture Face (texface) - NOT SUPPORTED
     from object-data tab, when in game-mode, meshes with a UV texture are allowed to have per-face options that affect BGE, the old exporter had limited support for these per-face options.

     pros:
          . per face control of double sided
          . per face control over transparency blending (but with limited options: opaque, add, alpha, clip alpha)
          . per face visible
          . per face object color (very old option, why would any one want object color when vertex colors can be used?)

     cons:
          . generates a new material shader for each combination of options (slower rendering, could end up with many submeshes)
          . limited blender viewport visual feedback, the artist can easily lose track of which faces have which settings.  Blender has no select-all by texface option, and only double-sided and visible are shown in the viewport.
          . many of the options make sense only for the BGE

     user workarounds:
          . per face control of double sided: user breaks into muliple objects when he needs control over single/double sided and uses object level double sided option (under Object tab -> Normals panel)
          . per face transparency blending options: user creates mutiple shaders, this should help the user stay more organized, and help the user keep track of performance bottlenecks
          . per face visible: user creates mutiple objects, and sets visiblity per object.

     programmed workarounds (pending):
          . per face collision is cool, and does not require generating multiple shaders per mesh.  Not that hard to script: delete those faces and auto-assign a collision mesh.  Although there is already support for user defined collision meshes, and they should be used most of the time because its optimal, so the user can delete those faces while making the collision mesh.

'''

@ogredoc
class _ogredoc_Physics( INFO_MT_ogre_helper ):
	mydoc = '''
Ogre Dot Scene + BGE Physics
	extended format including external collision mesh, and BGE physics settings
<node name="...">
	<entity name="..." meshFile="..." collisionFile="..." collisionPrim="..." [and all BGE physics attributes] />
</node>

collisionFile : sets path to .mesh that is used for collision (ignored if collisionPrim is set)
collisionPrim : sets optimal collision type [ cube, sphere, capsule, cylinder ]
*these collisions are static meshes, animated deforming meshes should give the user a warning that they have chosen a static mesh collision type with an object that has an armature

Blender Collision Setup:
	1. If a mesh object has a child mesh with a name starting with 'collision', then the child becomes the collision mesh for the parent mesh.

	2. If 'Collision Bounds' game option is checked, the bounds type [box, sphere, etc] is used. This will override above rule.

	3. Instances (and instances generated by optimal array modifier) will share the same collision type of the first instance, you DO NOT need to set the collision type for each instance.

	Tips and Tricks:
		. instance your mesh, parent it under the source, add a Decimate modifier, set the draw type to wire.  boom! easy optimized collision mesh
		. sphere collision type is the fastest

	TODO support composite collision objects?

'''

@ogredoc
class _ogredoc_Warnings( INFO_MT_ogre_helper ):
	mydoc = '''
General Warnings:
	. extra vertex groups, can mess up an armature weights (new vgroups must come after armature assignment, not before)
	. no sharp vertex color edges (face level vertex colors)
	. quadratic lights falloff not supported (needs pre calc)
	. do not enable subsurf modifier on meshes that have shape or armature animation.  
		(Any modifier that changes the vertex count is bad with shape anim or armature anim)

Current Shader Issues:
	1. You can not create a shader that ends up being more than 16 passes (Ogre limitation)
	2. Shader nodes do not support custom attributes, this limits material attribute overloading.
	3. At this time GLSL in Blender can not compile complex node trees, this will prevent
	you from seeing the output of the shader in the viewport, not seeing your textures in
	the viewport is a serious problem.  There are two workarounds:
		(a) disable 'use_nodes' and assign textures to the material in the normal manner.
			[ the shader nodes are still exported to Ogre ]
		(b) use a second branch connected to the first 'Output' and preview with software renderer.

'''


@ogredoc
class _ogredoc_Bugs( INFO_MT_ogre_helper ):
	mydoc = '''
Known Issues:
	. all bones must be connected
	. exporting with normals is very slow

'''

"""
@ogredoc
class _ogredoc_Node_Shaders_Introduction( INFO_MT_ogre_helper ):
	mydoc = _shader_intro_doc_

@ogredoc
class _ogredoc_Node_Shaders_Doc( INFO_MT_ogre_helper ):
	mydoc = _shader_using_doc_

@ogredoc
class _ogredoc_Node_Shaders_Tips_and_Tricks( INFO_MT_ogre_helper ):
	mydoc = _shader_tips_doc_

@ogredoc
class _ogredoc_Node_Shaders_Textures( INFO_MT_ogre_helper ):
	mydoc = _shader_tex_doc_

@ogredoc
class _ogredoc_Node_Shaders_Linking_Intro( INFO_MT_ogre_helper ):
	mydoc = _shader_linking_intro_doc_
@ogredoc
class _ogredoc_Node_Shaders_Linking_Steps( INFO_MT_ogre_helper ):
	mydoc = _shader_linking_steps_doc_
"""


############ Ogre v.17 Doc ######
@ogredoc
class _ogredoc_Shader_Nodes_ambient( INFO_MT_ogre_helper ):
	mydoc = '''
Sets the ambient colour reflectance properties of this pass. This attribute has no effect if a asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state. 

Format: ambient (<red> <green> <blue> [<alpha>]| vertexcolour)
NB valid colour values are between 0.0 and 1.0.

Example: ambient 0.0 0.8 0.0

The base colour of a pass is determined by how much red, green and blue light is reflects at each vertex. This property determines how much ambient light (directionless global light) is reflected. It is also possible to make the ambient reflectance track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. The default is full white, meaning objects are completely globally illuminated. Reduce this if you want to see diffuse or specular light effects, or change the blend of colours to make the object have a base colour other than white. This setting has no effect if dynamic lighting is disabled using the 'lighting off' attribute, or if any texture layer has a 'colour_op replace' attribute.

Default: ambient 1.0 1.0 1.0 1.0

'''

@ogredoc
class _ogredoc_Shader_Nodes_diffuse( INFO_MT_ogre_helper ):
	mydoc = '''
Sets the diffuse colour reflectance properties of this pass. This attribute has no effect if a asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.

Format: diffuse (<red> <green> <blue> [<alpha>]| vertexcolour)
NB valid colour values are between 0.0 and 1.0.

Example: diffuse 1.0 0.5 0.5

The base colour of a pass is determined by how much red, green and blue light is reflects at each vertex. This property determines how much diffuse light (light from instances of the Light class in the scene) is reflected. It is also possible to make the diffuse reflectance track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. The default is full white, meaning objects reflect the maximum white light they can from Light objects. This setting has no effect if dynamic lighting is disabled using the 'lighting off' attribute, or if any texture layer has a 'colour_op replace' attribute.

Default: diffuse 1.0 1.0 1.0 1.0
'''

@ogredoc
class _ogredoc_Shader_Nodes_specular( INFO_MT_ogre_helper ):
	mydoc = '''
Sets the specular colour reflectance properties of this pass. This attribute has no effect if a asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.

Format: specular (<red> <green> <blue> [<alpha>]| vertexcolour) <shininess>
NB valid colour values are between 0.0 and 1.0. Shininess can be any value greater than 0.

Example: specular 1.0 1.0 1.0 12.5

The base colour of a pass is determined by how much red, green and blue light is reflects at each vertex. This property determines how much specular light (highlights from instances of the Light class in the scene) is reflected. It is also possible to make the diffuse reflectance track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. The default is to reflect no specular light. The colour of the specular highlights is determined by the colour parameters, and the size of the highlights by the separate shininess parameter.. The higher the value of the shininess parameter, the sharper the highlight ie the radius is smaller. Beware of using shininess values in the range of 0 to 1 since this causes the the specular colour to be applied to the whole surface that has the material applied to it. When the viewing angle to the surface changes, ugly flickering will also occur when shininess is in the range of 0 to 1. Shininess values between 1 and 128 work best in both DirectX and OpenGL renderers. This setting has no effect if dynamic lighting is disabled using the 'lighting off' attribute, or if any texture layer has a 'colour_op replace' attribute.

Default: specular 0.0 0.0 0.0 0.0 0.0
'''

@ogredoc
class _ogredoc_Shader_Nodes_emissive( INFO_MT_ogre_helper ):
	mydoc = '''
emissive

Sets the amount of self-illumination an object has. This attribute has no effect if a asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.

Format: emissive (<red> <green> <blue> [<alpha>]| vertexcolour)
NB valid colour values are between 0.0 and 1.0.

Example: emissive 1.0 0.0 0.0

If an object is self-illuminating, it does not need external sources to light it, ambient or otherwise. It's like the object has it's own personal ambient light. Unlike the name suggests, this object doesn't act as a light source for other objects in the scene (if you want it to, you have to create a light which is centered on the object). It is also possible to make the emissive colour track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. This setting has no effect if dynamic lighting is disabled using the 'lighting off' attribute, or if any texture layer has a 'colour_op replace' attribute.

Default: emissive 0.0 0.0 0.0 0.0
'''

@ogredoc
class _ogredoc_Shader_Nodes_separate_scene_blend( INFO_MT_ogre_helper ):
	mydoc = '''
separate_scene_blend

This option operates in exactly the same way as scene_blend, except that it allows you to specify the operations to perform between the rendered pixel and the frame buffer separately for colour and alpha components. By nature this option is only useful when rendering to targets which have an alpha channel which you'll use for later processing, such as a render texture.

Format1: separate_scene_blend <simple_colour_blend> <simple_alpha_blend>
Example: separate_scene_blend add modulate

This example would add colour components but multiply alpha components. The blend modes available are as in scene_blend. The more advanced form is also available:

Params:
	@add  modulate
	@add  alpha_blend
	@add  color_blend

	@modulate  add
	@modulate  alpha_blend
	@modulate  color_blend

	@alpha_blend	add
	@alpha_blend	modulate
	@alpha_blend	color_blend

	@colour_blend  add
	@colour_blend  modulate
	@colour_blend  alpha_blend

Format2: separate_scene_blend <colour_src_factor> <colour_dest_factor> <alpha_src_factor> <alpha_dest_factor>
Example: separate_scene_blend one one_minus_dest_alpha one one 

Again the options available in the second format are the same as those in the second format of scene_blend.
'''

@ogredoc
class _ogredoc_Shader_Nodes_scene_blend_op( INFO_MT_ogre_helper ):
	mydoc = '''
scene_blend_op

This directive changes the operation which is applied between the two components of the scene blending equation, which by default is 'add' (sourceFactor * source + destFactor * dest). You may change this to 'add', 'subtract', 'reverse_subtract', 'min' or 'max'.

Format: scene_blend_op <add|subtract|reverse_subtract|min|max>
Default: scene_blend_op add

Params:
	@add
	@subtract
	@reverse_subtract
	@min
	@max

----------------------------------------------------
Math shader node can support: add, subtract, min and max
'''

@ogredoc
class _ogredoc_Shader_Nodes_separate_scene_blend_op( INFO_MT_ogre_helper ):
	mydoc = '''

separate_scene_blend_op

This directive is as scene_blend_op, except that you can set the operation for colour and alpha separately. 
Format: separate_scene_blend_op <colourOp> <alphaOp>
Default: separate_scene_blend_op add add

Params:
	@add  subtract
	@add  reverse_subtract
	@add  min
	@add  max

	@subtract  add
	@subtract  reverse_subtract
	@subtract  min
	@subtract  max

	@reverse_subtract  subtract
	@reverse_subtract  add
	@reverse_subtract  min
	@reverse_subtract  max

	@min  add
	@min  subtract
	@min  reverse_subtract
	@min  max

	@max  add
	@max  subtract
	@max  reverse_subtract
	@max  min

'''


@ogredoc
class _ogredoc_Shader_Nodes_depth_check( INFO_MT_ogre_helper ):
	mydoc = '''
depth_check
Params:
	@on
	@off

Sets whether or not this pass renders with depth-buffer checking on or not.

Format: depth_check <on|off>

If depth-buffer checking is on, whenever a pixel is about to be written to the frame buffer the depth buffer is checked to see if the pixel is in front of all other pixels written at that point. If not, the pixel is not written. If depth checking is off, pixels are written no matter what has been rendered before. Also see depth_func for more advanced depth check configuration.

Default: depth_check on
'''

@ogredoc
class _ogredoc_Shader_Nodes_depth_write( INFO_MT_ogre_helper ):
	mydoc = '''
depth_write
Params:
	@on
	@off

Sets whether or not this pass renders with depth-buffer writing on or not.
Format: depth_write <on|off>

If depth-buffer writing is on, whenever a pixel is written to the frame buffer the depth buffer is updated with the depth value of that new pixel, thus affecting future rendering operations if future pixels are behind this one. If depth writing is off, pixels are written without updating the depth buffer. Depth writing should normally be on but can be turned off when rendering static backgrounds or when rendering a collection of transparent objects at the end of a scene so that they overlap each other correctly.

Default: depth_write on
'''

@ogredoc
class _ogredoc_Shader_Nodes_depth_func( INFO_MT_ogre_helper ):
	mydoc = '''
depth_func

Sets the function used to compare depth values when depth checking is on.

Format: depth_func <func>

If depth checking is enabled (see depth_check) a comparison occurs between the depth value of the pixel to be written and the current contents of the buffer. This comparison is normally less_equal, i.e. the pixel is written if it is closer (or at the same distance) than the current contents. The possible functions are:
	@always_fail
		Never writes a pixel to the render target
	@always_pass
		Always writes a pixel to the render target
	@less
		Write if (new_Z < existing_Z)
	@less_equal
		Write if (new_Z <= existing_Z)
	@equal
		Write if (new_Z == existing_Z)
	@not_equal
		Write if (new_Z != existing_Z)
	@greater_equal
		Write if (new_Z >= existing_Z)
	@greater
		Write if (new_Z >existing_Z)

Default: depth_func less_equal
'''

@ogredoc
class _ogredoc_Shader_Nodes_depth_bias( INFO_MT_ogre_helper ):
	mydoc = '''
depth_bias
Params:
	@1
	@2
	@3
	@4

Sets the bias applied to the depth value of this pass. Can be used to make coplanar polygons appear on top of others e.g. for decals. 

Format: depth_bias <constant_bias> [<slopescale_bias>]

The final depth bias value is constant_bias * minObservableDepth + maxSlope * slopescale_bias. Slope scale biasing is relative to the angle of the polygon to the camera, which makes for a more appropriate bias value, but this is ignored on some older hardware. Constant biasing is expressed as a factor of the minimum depth value, so a value of 1 will nudge the depth by one 'notch' if you will. Also see iteration_depth_bias
'''

@ogredoc
class _ogredoc_Shader_Nodes_iteration_depth_bias( INFO_MT_ogre_helper ):
	mydoc = '''
iteration_depth_bias
Params:
	@1
	@2
	@3
	@4

Sets an additional bias derived from the number of times a given pass has been iterated. Operates just like depth_bias except that it applies an additional bias factor to the base depth_bias value, multiplying the provided value by the number of times this pass has been iterated before, through one of the iteration variants. So the first time the pass will get the depth_bias value, the second time it will get depth_bias + iteration_depth_bias, the third time it will get depth_bias + iteration_depth_bias * 2, and so on. The default is zero. 

Format: iteration_depth_bias <bias_per_iteration>
'''

@ogredoc
class _ogredoc_Shader_Nodes_alpha_rejection( INFO_MT_ogre_helper ):
	mydoc = '''
alpha_rejection
Params:
	@always_pass
	@greater_equal 0
	@greater_equal 16
	@greater_equal 32
	@greater_equal 64

Sets the way the pass will have use alpha to totally reject pixels from the pipeline.

Format: alpha_rejection <function> <value>

Example: alpha_rejection greater_equal 128


The function parameter can be any of the options listed in the material depth_function attribute. The value parameter can theoretically be any value between 0 and 255, but is best limited to 0 or 128 for hardware compatibility.

Default: alpha_rejection always_pass
'''

@ogredoc
class _ogredoc_Shader_Nodes_alpha_to_coverage( INFO_MT_ogre_helper ):
	mydoc = '''
alpha_to_coverage
Params:
	@on
	@off

Sets whether this pass will use 'alpha to coverage', a way to multisample alpha texture edges so they blend more seamlessly with the background. This facility is typically only available on cards from around 2006 onwards, but it is safe to enable it anyway - Ogre will just ignore it if the hardware does not support it. The common use for alpha to coverage is foliage rendering and chain-link fence style textures. 

Format: alpha_to_coverage <on|off>
Default: alpha_to_coverage off
'''

@ogredoc
class _ogredoc_Shader_Nodes_light_scissor( INFO_MT_ogre_helper ):
	mydoc = '''
light_scissor
Params:
	@on
	@off

Sets whether when rendering this pass, rendering will be limited to a screen-space scissor rectangle representing the coverage of the light(s) being used in this pass, derived from their attenuation ranges.

Format: light_scissor <on|off>
Default: light_scissor off

This option is usually only useful if this pass is an additive lighting pass, and is at least the second one in the technique. Ie areas which are not affected by the current light(s) will never need to be rendered. If there is more than one light being passed to the pass, then the scissor is defined to be the rectangle which covers all lights in screen-space. Directional lights are ignored since they are infinite.

This option does not need to be specified if you are using a standard additive shadow mode, i.e. SHADOWTYPE_STENCIL_ADDITIVE or SHADOWTYPE_TEXTURE_ADDITIVE, since it is the default behaviour to use a scissor for each additive shadow pass. However, if you're not using shadows, or you're using Integrated Texture Shadows where passes are specified in a custom manner, then this could be of use to you.
'''

@ogredoc
class _ogredoc_Shader_Nodes_light_clip_planes( INFO_MT_ogre_helper ):
	mydoc = '''
light_clip_planes
Params:
	@on
	@off
Sets whether when rendering this pass, triangle setup will be limited to clipping volume covered by the light. Directional lights are ignored, point lights clip to a cube the size of the attenuation range or the light, and spotlights clip to a pyramid bounding the spotlight angle and attenuation range.

Format: light_clip_planes <on|off>
Default: light_clip_planes off

This option will only function if there is a single non-directional light being used in this pass. If there is more than one light, or only directional lights, then no clipping will occur. If there are no lights at all then the objects won't be rendered at all.

When using a standard additive shadow mode, ie SHADOWTYPE_STENCIL_ADDITIVE or SHADOWTYPE_TEXTURE_ADDITIVE, you have the option of enabling clipping for all light passes by calling SceneManager::setShadowUseLightClipPlanes regardless of this pass setting, since rendering is done lightwise anyway. This is off by default since using clip planes is not always faster - it depends on how much of the scene the light volumes cover. Generally the smaller your lights are the more chance you'll see a benefit rather than a penalty from clipping. If you're not using shadows, or you're using Integrated Texture Shadows where passes are specified in a custom manner, then specify the option per-pass using this attribute.

A specific note about OpenGL: user clip planes are completely ignored when you use an ARB vertex program. This means light clip planes won't help much if you use ARB vertex programs on GL, although OGRE will perform some optimisation of its own, in that if it sees that the clip volume is completely off-screen, it won't perform a render at all. When using GLSL, user clipping can be used but you have to use glClipVertex in your shader, see the GLSL documentation for more information. In Direct3D user clip planes are always respected.
'''

@ogredoc
class _ogredoc_Shader_Nodes_illumination_stage( INFO_MT_ogre_helper ):
	mydoc = '''
illumination_stage
Params:
	@ambient
	@per_light
	@decal

When using an additive lighting mode (SHADOWTYPE_STENCIL_ADDITIVE or SHADOWTYPE_TEXTURE_ADDITIVE), the scene is rendered in 3 discrete stages, ambient (or pre-lighting), per-light (once per light, with shadowing) and decal (or post-lighting). Usually OGRE figures out how to categorise your passes automatically, but there are some effects you cannot achieve without manually controlling the illumination. For example specular effects are muted by the typical sequence because all textures are saved until the 'decal' stage which mutes the specular effect. Instead, you could do texturing within the per-light stage if it's possible for your material and thus add the specular on after the decal texturing, and have no post-light rendering. 

If you assign an illumination stage to a pass you have to assign it to all passes in the technique otherwise it will be ignored. Also note that whilst you can have more than one pass in each group, they cannot alternate, ie all ambient passes will be before all per-light passes, which will also be before all decal passes. Within their categories the passes will retain their ordering though. Format: illumination_stage <ambient|per_light|decal>

Default: none (autodetect)
'''


@ogredoc
class _ogredoc_Shader_Nodes_normalise_normals( INFO_MT_ogre_helper ):
	mydoc = '''
normalise_normals
Params:
	@on
	@off

Sets whether or not this pass renders with all vertex normals being automatically re-normalised.
Format: normalise_normals <on|off>

Scaling objects causes normals to also change magnitude, which can throw off your lighting calculations. By default, the SceneManager detects this and will automatically re-normalise normals for any scaled object, but this has a cost. If you'd prefer to control this manually, call SceneManager::setNormaliseNormalsOnScale(false) and then use this option on materials which are sensitive to normals being resized. 

Default: normalise_normals off
'''

@ogredoc
class _ogredoc_Shader_Nodes_transparent_sorting( INFO_MT_ogre_helper ):
	mydoc = '''
transparent_sorting
Params:
	@on
	@off
	@force

Sets if transparent textures should be sorted by depth or not.

Format: transparent_sorting <on|off|force>

By default all transparent materials are sorted such that renderables furthest away from the camera are rendered first. This is usually the desired behaviour but in certain cases this depth sorting may be unnecessary and undesirable. If for example it is necessary to ensure the rendering order does not change from one frame to the next. In this case you could set the value to 'off' to prevent sorting.

You can also use the keyword 'force' to force transparent sorting on, regardless of other circumstances. Usually sorting is only used when the pass is also transparent, and has a depth write or read which indicates it cannot reliably render without sorting. By using 'force', you tell OGRE to sort this pass no matter what other circumstances are present.

Default: transparent_sorting on
'''

@ogredoc
class _ogredoc_Shader_Nodes_cull_hardware( INFO_MT_ogre_helper ):
	mydoc = '''
cull_hardware
Params:
	@clockwise
	@anticlockwise
	@none

Sets the hardware culling mode for this pass.

Format: cull_hardware <clockwise|anticlockwise|none>

A typical way for the hardware rendering engine to cull triangles is based on the 'vertex winding' of triangles. Vertex winding refers to the direction in which the vertices are passed or indexed to in the rendering operation as viewed from the camera, and will wither be clockwise or anticlockwise (that's 'counterclockwise' for you Americans out there ;). If the option 'cull_hardware clockwise' is set, all triangles whose vertices are viewed in clockwise order from the camera will be culled by the hardware. 'anticlockwise' is the reverse (obviously), and 'none' turns off hardware culling so all triagles are rendered (useful for creating 2-sided passes).

Default: cull_hardware clockwise
NB this is the same as OpenGL's default but the opposite of Direct3D's default (because Ogre uses a right-handed coordinate system like OpenGL).
'''

@ogredoc
class _ogredoc_Shader_Nodes_cull_software( INFO_MT_ogre_helper ):
	mydoc = '''
cull_software
Params:
	@back
	@front
	@none

Sets the software culling mode for this pass.

Format: cull_software <back|front|none>

In some situations the engine will also cull geometry in software before sending it to the hardware renderer. This setting only takes effect on SceneManager's that use it (since it is best used on large groups of planar world geometry rather than on movable geometry since this would be expensive), but if used can cull geometry before it is sent to the hardware. In this case the culling is based on whether the 'back' or 'front' of the triangle is facing the camera - this definition is based on the face normal (a vector which sticks out of the front side of the polygon perpendicular to the face). Since Ogre expects face normals to be on anticlockwise side of the face, 'cull_software back' is the software equivalent of 'cull_hardware clockwise' setting, which is why they are both the default. The naming is different to reflect the way the culling is done though, since most of the time face normals are pre-calculated and they don't have to be the way Ogre expects - you could set 'cull_hardware none' and completely cull in software based on your own face normals, if you have the right SceneManager which uses them.

Default: cull_software back

'''

@ogredoc
class _ogredoc_Shader_Nodes_lighting( INFO_MT_ogre_helper ):
	mydoc = '''
lighting
Params:
	@on
	@off

Sets whether or not dynamic lighting is turned on for this pass or not. If lighting is turned off, all objects rendered using the pass will be fully lit. This attribute has no effect if a vertex program is used.

Format: lighting <on|off>

Turning dynamic lighting off makes any ambient, diffuse, specular, emissive and shading properties for this pass redundant. When lighting is turned on, objects are lit according to their vertex normals for diffuse and specular light, and globally for ambient and emissive.

Default: lighting on
'''

@ogredoc
class _ogredoc_Shader_Nodes_shading( INFO_MT_ogre_helper ):
	mydoc = '''
shading
Params:
	@flat
	@gouraud
	@phong

Sets the kind of shading which should be used for representing dynamic lighting for this pass.

Format: shading <flat|gouraud|phong>

When dynamic lighting is turned on, the effect is to generate colour values at each vertex. Whether these values are interpolated across the face (and how) depends on this setting.

flat
	No interpolation takes place. Each face is shaded with a single colour determined from the first vertex in the face.
gouraud
	Colour at each vertex is linearly interpolated across the face.
phong
	Vertex normals are interpolated across the face, and these are used to determine colour at each pixel. Gives a more natural lighting effect but is more expensive and works better at high levels of tessellation. Not supported on all hardware.

Default: shading gouraud
'''

@ogredoc
class _ogredoc_Shader_Nodes_polygon_mode( INFO_MT_ogre_helper ):
	mydoc = '''
polygon_mode
Params:
	@solid
	@wireframe
	@points

Sets how polygons should be rasterised, i.e. whether they should be filled in, or just drawn as lines or points.

Format: polygon_mode <solid|wireframe|points>

solid
The normal situation - polygons are filled in.
wireframe
Polygons are drawn in outline only.
points
Only the points of each polygon are rendered.
Default: polygon_mode solid
'''

@ogredoc
class _ogredoc_Shader_Nodes_polygon_mode_overrideable( INFO_MT_ogre_helper ):
	mydoc = '''
polygon_mode_overrideable
Params:
	@true
	@false

Sets whether or not the polygon_mode set on this pass can be downgraded by the camera, if the camera itself is set to a lower polygon mode. If set to false, this pass will always be rendered at its own chosen polygon mode no matter what the camera says. The default is true.

Format: polygon_mode_overrideable <true|false>
'''


@ogredoc
class _ogredoc_Shader_Nodes_fog_override( INFO_MT_ogre_helper ):
	mydoc = '''
fog_override
Params:
	@true
	@false
	@true exp 1 1 1 0.002 100 10000

Tells the pass whether it should override the scene fog settings, and enforce it's own. Very useful for things that you don't want to be affected by fog when the rest of the scene is fogged, or vice versa. Note that this only affects fixed-function fog - the original scene fog parameters are still sent to shaders which use the fog_params parameter binding (this allows you to turn off fixed function fog and calculate it in the shader instead; if you want to disable shader fog you can do that through shader parameters anyway). 

Format: fog_override <override?> [<type> <colour> <density> <start> <end>]

Default: fog_override false

If you specify 'true' for the first parameter and you supply the rest of the parameters, you are telling the pass to use these fog settings in preference to the scene settings, whatever they might be. If you specify 'true' but provide no further parameters, you are telling this pass to never use fogging no matter what the scene says. Here is an explanation of the parameters:
type
	none = No fog, equivalent of just using 'fog_override true'
	linear = Linear fog from the <start> and <end> distances
	exp = Fog increases exponentially from the camera (fog = 1/e^(distance * density)), use <density> param to control it
	exp2 = Fog increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2), use <density> param to control it
colour
	Sequence of 3 floating point values from 0 to 1 indicating the red, green and blue intensities
density
	The density parameter used in the 'exp' or 'exp2' fog types. Not used in linear mode but param must still be there as a placeholder 
start
	The start distance from the camera of linear fog. Must still be present in other modes, even though it is not used.
end
	The end distance from the camera of linear fog. Must still be present in other modes, even though it is not used.

Example: fog_override true exp 1 1 1 0.002 100 10000
'''

@ogredoc
class _ogredoc_Shader_Nodes_colour_write( INFO_MT_ogre_helper ):
	mydoc = '''
colour_write
Params:
	@on
	@off

Sets whether or not this pass renders with colour writing on or not.
Format: colour_write <on|off>

If colour writing is off no visible pixels are written to the screen during this pass. You might think this is useless, but if you render with colour writing off, and with very minimal other settings, you can use this pass to initialise the depth buffer before subsequently rendering other passes which fill in the colour data. This can give you significant performance boosts on some newer cards, especially when using complex fragment programs, because if the depth check fails then the fragment program is never run. 

Default: colour_write on
'''

@ogredoc
class _ogredoc_Shader_Nodes_start_light( INFO_MT_ogre_helper ):
	mydoc = '''
start_light
Params:
	@0
	@1
	@2
	@3
	@4
	@5
	@6

Sets the first light which will be considered for use with this pass.

Format: start_light <number>

You can use this attribute to offset the starting point of the lights for this pass. In other words, if you set start_light to 2 then the first light to be processed in that pass will be the third actual light in the applicable list. You could use this option to use different passes to process the first couple of lights versus the second couple of lights for example, or use it in conjunction with the iteration option to start the iteration from a given point in the list (e.g. doing the first 2 lights in the first pass, and then iterating every 2 lights from then on perhaps). 

Default: start_light 0
'''

@ogredoc
class _ogredoc_Shader_Nodes_max_lights( INFO_MT_ogre_helper ):
	mydoc = '''
max_lights
Params:
	@1
	@2
	@3
	@4
	@5
	@6

Sets the maximum number of lights which will be considered for use with this pass.

Format: max_lights <number>

The maximum number of lights which can be used when rendering fixed-function materials is set by the rendering system, and is typically set at 8. When you are using the programmable pipeline (See section 3.1.9 Using Vertex/Geometry/Fragment Programs in a Pass) this limit is dependent on the program you are running, or, if you use 'iteration once_per_light' or a variant (See section iteration), it effectively only bounded by the number of passes you are willing to use. If you are not using pass iteration, the light limit applies once for this pass. If you are using pass iteration, the light limit applies across all iterations of this pass - for example if you have 12 lights in range with an 'iteration once_per_light' setup but your max_lights is set to 4 for that pass, the pass will only iterate 4 times. 

Default: max_lights 8
'''

@ogredoc
class _ogredoc_Shader_Nodes_iteration( INFO_MT_ogre_helper ):
	mydoc = '''
iteration
Params:
	@once
	@once_per_light
	@1 once
	@1 once_per_light
	@2 once
	@2 once_per_light

Sets whether or not this pass is iterated, i.e. issued more than once.

Format 1: iteration <once | once_per_light> [lightType]

Format 2: iteration <number> [<per_light> [lightType]]

Format 3: iteration <number> [<per_n_lights> <num_lights> [lightType]]

Examples:
	iteration once
		The pass is only executed once which is the default behaviour.
	iteration once_per_light point
		The pass is executed once for each point light.
	iteration 5
		The render state for the pass will be setup and then the draw call will execute 5 times.
	iteration 5 per_light point
		The render state for the pass will be setup and then the draw call will execute 5 times. This will be done for each point light.
	iteration 1 per_n_lights 2 point
		The render state for the pass will be setup and the draw call executed once for every 2 lights.

By default, passes are only issued once. However, if you use the programmable pipeline, or you wish to exceed the normal limits on the number of lights which are supported, you might want to use the once_per_light option. In this case, only light index 0 is ever used, and the pass is issued multiple times, each time with a different light in light index 0. Clearly this will make the pass more expensive, but it may be the only way to achieve certain effects such as per-pixel lighting effects which take into account 1..n lights.

Using a number instead of "once" instructs the pass to iterate more than once after the render state is setup. The render state is not changed after the initial setup so repeated draw calls are very fast and ideal for passes using programmable shaders that must iterate more than once with the same render state i.e. shaders that do fur, motion blur, special filtering.

If you use once_per_light, you should also add an ambient pass to the technique before this pass, otherwise when no lights are in range of this object it will not get rendered at all; this is important even when you have no ambient light in the scene, because you would still want the objects silhouette to appear.

The lightType parameter to the attribute only applies if you use once_per_light, per_light, or per_n_lights and restricts the pass to being run for lights of a single type (either 'point', 'directional' or 'spot'). In the example, the pass will be run once per point light. This can be useful because when you're writing a vertex / fragment program it is a lot easier if you can assume the kind of lights you'll be dealing with. However at least point and directional lights can be dealt with in one way. 

Default: iteration once

'''

@ogredoc
class _ogredoc_Shader_Nodes_point_size( INFO_MT_ogre_helper ):
	mydoc = '''
point_size
Params:
	@4
	@16
	@32

This setting allows you to change the size of points when rendering a point list, or a list of point sprites. The interpretation of this command depends on the point_size_attenuation option - if it is off (the default), the point size is in screen pixels, if it is on, it expressed as normalised screen coordinates (1.0 is the height of the screen) when the point is at the origin. 

NOTE: Some drivers have an upper limit on the size of points they support - this can even vary between APIs on the same card! Don't rely on point sizes that cause the points to get very large on screen, since they may get clamped on some cards. Upper sizes can range from 64 to 256 pixels.

Format: point_size <size>

Default: point_size 1.0
'''

@ogredoc
class _ogredoc_Shader_Nodes_point_sprites( INFO_MT_ogre_helper ):
	mydoc = '''
point_sprites
Params:
	@on
	@off

This setting specifies whether or not hardware point sprite rendering is enabled for this pass. Enabling it means that a point list is rendered as a list of quads rather than a list of dots. It is very useful to use this option if you're using a BillboardSet and only need to use point oriented billboards which are all of the same size. You can also use it for any other point list render. 

Format: point_sprites <on|off>

Default: point_sprites off
'''

@ogredoc
class _ogredoc_Shader_Nodes_point_size_attenuation( INFO_MT_ogre_helper ):
	mydoc = '''
point_size_attenuation
Params:
	@on
	@off
	@on constant
	@on linear
	@on quadratic

Defines whether point size is attenuated with view space distance, and in what fashion. This option is especially useful when you're using point sprites (See section point_sprites) since it defines how they reduce in size as they get further away from the camera. You can also disable this option to make point sprites a constant screen size (like points), or enable it for points so they change size with distance.

You only have to provide the final 3 parameters if you turn attenuation on. The formula for attenuation is that the size of the point is multiplied by 1 / (constant + linear * dist + quadratic * d^2); therefore turning it off is equivalent to (constant = 1, linear = 0, quadratic = 0) and standard perspective attenuation is (constant = 0, linear = 1, quadratic = 0). The latter is assumed if you leave out the final 3 parameters when you specify 'on'.

Note that the resulting attenuated size is clamped to the minimum and maximum point size, see the next section.

Format: point_size_attenuation <on|off> [constant linear quadratic] Default: point_size_attenuation off
'''

@ogredoc
class _ogredoc_Shader_Nodes_point_size_min( INFO_MT_ogre_helper ):
	mydoc = '''
point_size_min
Params:
	@1
	@4
	@8

Sets the minimum point size after attenuation (point_size_attenuation). For details on the size metrics, See section point_size.

Format: point_size_min <size> Default: point_size_min 0
'''

@ogredoc
class _ogredoc_Shader_Nodes_point_size_max( INFO_MT_ogre_helper ):
	mydoc = '''
point_size_max
Params:
	@32
	@64
	@128

Sets the maximum point size after attenuation (point_size_attenuation). For details on the size metrics, See section point_size. A value of 0 means the maximum is set to the same as the max size reported by the current card. 

Format: point_size_max <size> Default: point_size_max 0
'''

## Ogre 1.7 doc - 3.1.3 Texture Units ##
@ogredoc
class _ogredoc_TEX_texture_alias( INFO_MT_ogre_helper ):
	mydoc = '''
texture_alias
Params:
	@myname
Sets the alias name for this texture unit.
Format: texture_alias <name>
Example: texture_alias NormalMap
Setting the texture alias name is useful if this material is to be inherited by other other materials and only the textures will be changed in the new material.(See section 3.1.12 Texture Aliases)

Default: If a texture_unit has a name then the texture_alias defaults to the texture_unit name.
'''

@ogredoc
class _ogredoc_TEX_texture( INFO_MT_ogre_helper ):
	mydoc = '''
texture

Sets the name of the static texture image this layer will use.

Format: texture <texturename> [<type>] [unlimited | numMipMaps] [alpha] [<PixelFormat>] [gamma]

Example: texture funkywall.jpg

This setting is mutually exclusive with the anim_texture attribute. Note that the texture file cannot include spaces. Those of you Windows users who like spaces in filenames, please get over it and use underscores instead.

The 'type' parameter allows you to specify a the type of texture to create - the default is '2d', but you can override this; here's the full list:
1d
	A 1-dimensional texture; that is, a texture which is only 1 pixel high. These kinds of textures can be useful when you need to encode a function in a texture and use it as a simple lookup, perhaps in a fragment program. It is important that you use this setting when you use a fragment program which uses 1-dimensional texture coordinates, since GL requires you to use a texture type that matches (D3D will let you get away with it, but you ought to plan for cross-compatibility). Your texture widths should still be a power of 2 for best compatibility and performance.
2d
	The default type which is assumed if you omit it, your texture has a width and a height, both of which should preferably be powers of 2, and if you can, make them square because this will look best on the most hardware. These can be addressed with 2D texture coordinates.
3d
	A 3 dimensional texture i.e. volume texture. Your texture has a width, a height, both of which should be powers of 2, and has depth. These can be addressed with 3d texture coordinates i.e. through a pixel shader.
cubic
	This texture is made up of 6 2D textures which are pasted around the inside of a cube. Can be addressed with 3D texture coordinates and are useful for cubic reflection maps and normal maps.
The 'numMipMaps' option allows you to specify the number of mipmaps to generate for this texture. The default is 'unlimited' which means mips down to 1x1 size are generated. You can specify a fixed number (even 0) if you like instead. Note that if you use the same texture in many material scripts, the number of mipmaps generated will conform to the number specified in the first texture_unit used to load the texture - so be consistent with your usage.

The 'alpha' option allows you to specify that a single channel (luminance) texture should be loaded as alpha, rather than the default which is to load it into the red channel. This can be helpful if you want to use alpha-only textures in the fixed function pipeline. Default: none
'''

@ogredoc
class _ogredoc_TEX_anim_texture( INFO_MT_ogre_helper ):
	mydoc = '''
anim_texture

Sets the images to be used in an animated texture layer. In this case an animated texture layer means one which has multiple frames, each of which is a separate image file. There are 2 formats, one for implicitly determined image names, one for explicitly named images.

Format1 (short): anim_texture <base_name> <num_frames> <duration>
Example: anim_texture flame.jpg 5 2.5

This sets up an animated texture layer made up of 5 frames named flame_0.jpg, flame_1.jpg, flame_2.jpg etc, with an animation length of 2.5 seconds (2fps). If duration is set to 0, then no automatic transition takes place and frames must be changed manually in code.

Format2 (long): anim_texture <frame1> <frame2> ... <duration>
Example: anim_texture flamestart.jpg flamemore.png flameagain.jpg moreflame.jpg lastflame.tga 2.5

This sets up the same duration animation but from 5 separately named image files. The first format is more concise, but the second is provided if you cannot make your images conform to the naming standard required for it. 

Default: none
'''

@ogredoc
class _ogredoc_TEX_cubic_texture( INFO_MT_ogre_helper ):
	mydoc = '''
cubic_texture
	@mybasename separateUV
	@mybasename combinedUVW
	@front.jpg front.jpg back.jpg left.jpg right.jpg up.jpg down.jpg separateUV

Sets the images used in a cubic texture, i.e. one made up of 6 individual images making up the faces of a cube. These kinds of textures are used for reflection maps (if hardware supports cubic reflection maps) or skyboxes. There are 2 formats, a brief format expecting image names of a particular format and a more flexible but longer format for arbitrarily named textures.

Format1 (short): cubic_texture <base_name> <combinedUVW|separateUV>

The base_name in this format is something like 'skybox.jpg', and the system will expect you to provide skybox_fr.jpg, skybox_bk.jpg, skybox_up.jpg, skybox_dn.jpg, skybox_lf.jpg, and skybox_rt.jpg for the individual faces.

Format2 (long): cubic_texture <front> <back> <left> <right> <up> <down> separateUV

In this case each face is specified explicitly, incase you don't want to conform to the image naming standards above. You can only use this for the separateUV version since the combinedUVW version requires a single texture name to be assigned to the combined 3D texture (see below).

In both cases the final parameter means the following:
combinedUVW
The 6 textures are combined into a single 'cubic' texture map which is then addressed using 3D texture coordinates with U, V and W components. Necessary for reflection maps since you never know which face of the box you are going to need. Note that not all cards support cubic environment mapping.
separateUV
The 6 textures are kept separate but are all referenced by this single texture layer. One texture at a time is active (they are actually stored as 6 frames), and they are addressed using standard 2D UV coordinates. This type is good for skyboxes since only one face is rendered at one time and this has more guaranteed hardware support on older cards.

Default: none
'''

@ogredoc
class _ogredoc_TEX_binding_type( INFO_MT_ogre_helper ):
	mydoc = '''
binding_type
Params:
	@vertex
	@fragment
Tells this texture unit to bind to either the fragment processing unit or the vertex processing unit (for 3.1.10 Vertex Texture Fetch). 

Format: binding_type <vertex|fragment> Default: binding_type fragment
'''

@ogredoc
class _ogredoc_TEX_content_type( INFO_MT_ogre_helper ):
	mydoc = '''
content_type
	@compositor DepthCompositor OutputTexture

Tells this texture unit where it should get its content from. The default is to get texture content from a named texture, as defined with the texture, cubic_texture, anim_texture attributes. However you can also pull texture information from other automated sources. The options are:
@named
	The default option, this derives texture content from a texture name, loaded by ordinary means from a file or having been manually created with a given name.
@shadow
	This option allows you to pull in a shadow texture, and is only valid when you use texture shadows and one of the 'custom sequence' shadowing types (See section 7. Shadows). The shadow texture in question will be from the 'n'th closest light that casts shadows, unless you use light-based pass iteration or the light_start option which may start the light index higher. When you use this option in multiple texture units within the same pass, each one references the next shadow texture. The shadow texture index is reset in the next pass, in case you want to take into account the same shadow textures again in another pass (e.g. a separate specular / gloss pass). By using this option, the correct light frustum projection is set up for you for use in fixed-function, if you use shaders just reference the texture_viewproj_matrix auto parameter in your shader.
@compositor
	This option allows you to reference a texture from a compositor, and is only valid when the pass is rendered within a compositor sequence. This can be either in a render_scene directive inside a compositor script, or in a general pass in a viewport that has a compositor attached. Note that this is a reference only, meaning that it does not change the render order. You must make sure that the order is reasonable for what you are trying to achieve (for example, texture pooling might cause the referenced texture to be overwritten by something else by the time it is referenced). 

The extra parameters for the content_type are only required for this type: 
The first is the name of the compositor being referenced. (Required) 
The second is the name of the texture to reference in the compositor. (Required) 

The third is the index of the texture to take, in case of an MRT. (Optional)
Format: content_type <named|shadow|compositor> [<Referenced Compositor Name>] [<Referenced Texture Name>] [<Referenced MRT Index>] 

Default: content_type named 

Example: content_type compositor DepthCompositor OutputTexture 
'''

@ogredoc
class _ogredoc_TEX_tex_coord_set( INFO_MT_ogre_helper ):
	mydoc = '''
tex_coord_set

Sets which texture coordinate set is to be used for this texture layer. A mesh can define multiple sets of texture coordinates, this sets which one this material uses.

Format: tex_coord_set <set_num>
Example: tex_coord_set 2
Default: tex_coord_set 0
'''

@ogredoc
class _ogredoc_TEX_tex_address_mode( INFO_MT_ogre_helper ):
	mydoc = '''
tex_address_mode

Defines what happens when texture coordinates exceed 1.0 for this texture layer.You can use the simple format to specify the addressing mode for all 3 potential texture coordinates at once, or you can use the 2/3 parameter extended format to specify a different mode per texture coordinate. 

Simple Format: tex_address_mode <uvw_mode> 
Extended Format: tex_address_mode <u_mode> <v_mode> [<w_mode>]

@wrap
	Any value beyond 1.0 wraps back to 0.0. Texture is repeated.
@clamp
	Values beyond 1.0 are clamped to 1.0. Texture 'streaks' beyond 1.0 since last line of pixels is used across the rest of the address space. Useful for textures which need exact coverage from 0.0 to 1.0 without the 'fuzzy edge' wrap gives when combined with filtering.
@mirror
	Texture flips every boundary, meaning texture is mirrored every 1.0 u or v
@border
	Values outside the range [0.0, 1.0] are set to the border colour, you might also set the tex_border_colour attribute too.

Default: tex_address_mode wrap
'''

@ogredoc
class _ogredoc_TEX_tex_border_colour( INFO_MT_ogre_helper ):
	mydoc = '''
tex_border_colour
	@0.0 0.5 0.0 0.25
Sets the border colour of border texture address mode (see tex_address_mode). 
Format: tex_border_colour <red> <green> <blue> [<alpha>]
NB valid colour values are between 0.0 and 1.0.
Example: tex_border_colour 0.0 1.0 0.3
Default: tex_border_colour 0.0 0.0 0.0 1.0
'''

@ogredoc
class _ogredoc_TEX_filtering( INFO_MT_ogre_helper ):
	mydoc = '''
filtering

Sets the type of texture filtering used when magnifying or minifying a texture. There are 2 formats to this attribute, the simple format where you simply specify the name of a predefined set of filtering options, and the complex format, where you individually set the minification, magnification, and mip filters yourself.

Simple Format
Format: filtering <none|bilinear|trilinear|anisotropic>
Default: filtering bilinear

With this format, you only need to provide a single parameter which is one of the following:
@none
	No filtering or mipmapping is used. This is equivalent to the complex format 'filtering point point none'.
@bilinear
	2x2 box filtering is performed when magnifying or reducing a texture, and a mipmap is picked from the list but no filtering is done between the levels of the mipmaps. This is equivalent to the complex format 'filtering linear linear point'.

@trilinear
	2x2 box filtering is performed when magnifying and reducing a texture, and the closest 2 mipmaps are filtered together. This is equivalent to the complex format 'filtering linear linear linear'.
@anisotropic
	This is the same as 'trilinear', except the filtering algorithm takes account of the slope of the triangle in relation to the camera rather than simply doing a 2x2 pixel filter in all cases. This makes triangles at acute angles look less fuzzy. Equivalent to the complex format 'filtering anisotropic anisotropic linear'. Note that in order for this to make any difference, you must also set the max_anisotropy attribute too.
'''

@ogredoc
class _ogredoc_TEX_max_anisotropy( INFO_MT_ogre_helper ):
	mydoc = '''
max_anisotropy
	@2
	@4
	@8
	@16
Sets the maximum degree of anisotropy that the renderer will try to compensate for when filtering textures. The degree of anisotropy is the ratio between the height of the texture segment visible in a screen space region versus the width - so for example a floor plane, which stretches on into the distance and thus the vertical texture coordinates change much faster than the horizontal ones, has a higher anisotropy than a wall which is facing you head on (which has an anisotropy of 1 if your line of sight is perfectly perpendicular to it). You should set the max_anisotropy value to something greater than 1 to begin compensating; higher values can compensate for more acute angles. The maximum value is determined by the hardware, but it is usually 8 or 16. 

In order for this to be used, you have to set the minification and/or the magnification filtering option on this texture to anisotropic. Format: max_anisotropy <value>
Default: max_anisotropy 1
'''

@ogredoc
class _ogredoc_TEX_mipmap_bias( INFO_MT_ogre_helper ):
	mydoc = '''
mipmap_bias
	@1
	@4
	@8

Sets the bias value applied to the mipmapping calculation, thus allowing you to alter the decision of which level of detail of the texture to use at any distance. The bias value is applied after the regular distance calculation, and adjusts the mipmap level by 1 level for each unit of bias. Negative bias values force larger mip levels to be used, positive bias values force smaller mip levels to be used. The bias is a floating point value so you can use values in between whole numbers for fine tuning.

In order for this option to be used, your hardware has to support mipmap biasing (exposed through the render system capabilities), and your minification filtering has to be set to point or linear. Format: mipmap_bias <value>
Default: mipmap_bias 0
'''

@ogredoc
class _ogredoc_TEX_colour_op( INFO_MT_ogre_helper ):
	mydoc = '''
colour_op

Determines how the colour of this texture layer is combined with the one below it (or the lighting effect on the geometry if this is the first layer).
Format: colour_op <replace|add|modulate|alpha_blend>

This method is the simplest way to blend texture layers, because it requires only one parameter, gives you the most common blending types, and automatically sets up 2 blending methods: one for if single-pass multitexturing hardware is available, and another for if it is not and the blending must be achieved through multiple rendering passes. It is, however, quite limited and does not expose the more flexible multitexturing operations, simply because these can't be automatically supported in multipass fallback mode. If want to use the fancier options, use colour_op_ex, but you'll either have to be sure that enough multitexturing units will be available, or you should explicitly set a fallback using colour_op_multipass_fallback.
@replace
	Replace all colour with texture with no adjustment.
@add
	Add colour components together.
@modulate
	Multiply colour components together.
@alpha_blend
	Blend based on texture alpha.

Default: colour_op modulate
'''

@ogredoc
class _ogredoc_TEX_colour_op_ex( INFO_MT_ogre_helper ):
	mydoc = '''
colour_op_ex
	@add_signed src_manual src_current 0.5

This is an extended version of the colour_op attribute which allows extremely detailed control over the blending applied between this and earlier layers. Multitexturing hardware can apply more complex blending operations that multipass blending, but you are limited to the number of texture units which are available in hardware.

Format: colour_op_ex <operation> <source1> <source2> [<manual_factor>] [<manual_colour1>] [<manual_colour2>]
Example colour_op_ex add_signed src_manual src_current 0.5

See the IMPORTANT note below about the issues between multipass and multitexturing that using this method can create. Texture colour operations determine how the final colour of the surface appears when rendered. Texture units are used to combine colour values from various sources (e.g. the diffuse colour of the surface from lighting calculations, combined with the colour of the texture). This method allows you to specify the 'operation' to be used, i.e. the calculation such as adds or multiplies, and which values to use as arguments, such as a fixed value or a value from a previous calculation.

Operation options:
	source1
		Use source1 without modification
	source2
		Use source2 without modification
	modulate
		Multiply source1 and source2 together.
	modulate_x2
		Multiply source1 and source2 together, then by 2 (brightening).
	modulate_x4
		Multiply source1 and source2 together, then by 4 (brightening).
	add
		Add source1 and source2 together.
	add_signed
		Add source1 and source2 then subtract 0.5.
	add_smooth
		Add source1 and source2, subtract the product
	subtract
		Subtract source2 from source1
	blend_diffuse_alpha
		Use interpolated alpha value from vertices to scale source1, then add source2 scaled by (1-alpha).
	blend_texture_alpha
		As blend_diffuse_alpha but use alpha from texture
	blend_current_alpha
		As blend_diffuse_alpha but use current alpha from previous stages (same as blend_diffuse_alpha for first layer)
	blend_manual
		As blend_diffuse_alpha but use a constant manual alpha value specified in <manual>
	dotproduct
		The dot product of source1 and source2
	blend_diffuse_colour

		Use interpolated colour value from vertices to scale source1, then add source2 scaled by (1-colour).

Source1 and source2 options:
	src_current
		The colour as built up from previous stages.
	src_texture
		The colour derived from the texture assigned to this layer.
	src_diffuse
		The interpolated diffuse colour from the vertices (same as 'src_current' for first layer).
	src_specular
		The interpolated specular colour from the vertices.
	src_manual
		The manual colour specified at the end of the command.

For example 'modulate' takes the colour results of the previous layer, and multiplies them with the new texture being applied. Bear in mind that colours are RGB values from 0.0-1.0 so multiplying them together will result in values in the same range, 'tinted' by the multiply. Note however that a straight multiply normally has the effect of darkening the textures - for this reason there are brightening operations like modulate_x2. Note that because of the limitations on some underlying APIs (Direct3D included) the 'texture' argument can only be used as the first argument, not the second. 

Note that the last parameter is only required if you decide to pass a value manually into the operation. Hence you only need to fill these in if you use the 'blend_manual' operation.

'''

@ogredoc
class _ogredoc_TEX_colour_op_multipass_fallback( INFO_MT_ogre_helper ):
	mydoc = '''
colour_op_multipass_fallback
	@one one_minus_dest_alpha

Sets the multipass fallback operation for this layer, if you used colour_op_ex and not enough multitexturing hardware is available.

Format: colour_op_multipass_fallback <src_factor> <dest_factor>
Example: colour_op_multipass_fallback one one_minus_dest_alpha

Because some of the effects you can create using colour_op_ex are only supported under multitexturing hardware, if the hardware is lacking the system must fallback on multipass rendering, which unfortunately doesn't support as many effects. This attribute is for you to specify the fallback operation which most suits you.

The parameters are the same as in the scene_blend attribute; this is because multipass rendering IS effectively scene blending, since each layer is rendered on top of the last using the same mechanism as making an object transparent, it's just being rendered in the same place repeatedly to get the multitexture effect. If you use the simpler (and less flexible) colour_op attribute you don't need to call this as the system sets up the fallback for you.

'''

@ogredoc
class _ogredoc_TEX_alpha_op_ex( INFO_MT_ogre_helper ):
	mydoc = '''
alpha_op_ex

Behaves in exactly the same away as colour_op_ex except that it determines how alpha values are combined between texture layers rather than colour values.The only difference is that the 2 manual colours at the end of colour_op_ex are just single floating-point values in alpha_op_ex.
'''

@ogredoc
class _ogredoc_TEX_env_map( INFO_MT_ogre_helper ):
	mydoc = '''
env_map

Turns on/off texture coordinate effect that makes this layer an environment map.

Format: env_map <off|spherical|planar|cubic_reflection|cubic_normal>

Environment maps make an object look reflective by using automatic texture coordinate generation depending on the relationship between the objects vertices or normals and the eye.

@spherical
	A spherical environment map. Requires a single texture which is either a fish-eye lens view of the reflected scene, or some other texture which looks good as a spherical map (a texture of glossy highlights is popular especially in car sims). This effect is based on the relationship between the eye direction and the vertex normals of the object, so works best when there are a lot of gradually changing normals, i.e. curved objects.
@planar
	Similar to the spherical environment map, but the effect is based on the position of the vertices in the viewport rather than vertex normals. This effect is therefore useful for planar geometry (where a spherical env_map would not look good because the normals are all the same) or objects without normals.
@cubic_reflection
	A more advanced form of reflection mapping which uses a group of 6 textures making up the inside of a cube, each of which is a view if the scene down each axis. Works extremely well in all cases but has a higher technical requirement from the card than spherical mapping. Requires that you bind a cubic_texture to this texture unit and use the 'combinedUVW' option.
@cubic_normal
	Generates 3D texture coordinates containing the camera space normal vector from the normal information held in the vertex data. Again, full use of this feature requires a cubic_texture with the 'combinedUVW' option.

Default: env_map off
'''

@ogredoc
class _ogredoc_TEX_scroll( INFO_MT_ogre_helper ):
	mydoc = '''
scroll

Sets a fixed scroll offset for the texture.
Format: scroll <x> <y>

This method offsets the texture in this layer by a fixed amount. Useful for small adjustments without altering texture coordinates in models. However if you wish to have an animated scroll effect, see the scroll_anim attribute.

[use mapping node location x and y]
'''

@ogredoc
class _ogredoc_TEX_scroll_anim( INFO_MT_ogre_helper ):
	mydoc = '''
scroll_anim
	@0.1 0.1
	@0.5 0.5
	@2.0 3.0

Sets up an animated scroll for the texture layer. Useful for creating fixed-speed scrolling effects on a texture layer (for varying scroll speeds, see wave_xform).

Format: scroll_anim <xspeed> <yspeed>
'''

@ogredoc
class _ogredoc_TEX_rotate( INFO_MT_ogre_helper ):
	mydoc = '''
rotate
Rotates a texture to a fixed angle. This attribute changes the rotational orientation of a texture to a fixed angle, useful for fixed adjustments. If you wish to animate the rotation, see rotate_anim.
Format: rotate <angle>
The parameter is a anti-clockwise angle in degrees.
[ use mapping node rotation x ]
'''

@ogredoc
class _ogredoc_TEX_rotate_anim( INFO_MT_ogre_helper ):
	mydoc = '''
rotate_anim
	@0.1
	@0.2
	@0.4
Sets up an animated rotation effect of this layer. Useful for creating fixed-speed rotation animations (for varying speeds, see wave_xform).
Format: rotate_anim <revs_per_second>
The parameter is a number of anti-clockwise revolutions per second.
'''

@ogredoc
class _ogredoc_TEX_scale( INFO_MT_ogre_helper ):
	mydoc = '''
scale
Adjusts the scaling factor applied to this texture layer. Useful for adjusting the size of textures without making changes to geometry. This is a fixed scaling factor, if you wish to animate this see wave_xform.
Format: scale <x_scale> <y_scale>
Valid scale values are greater than 0, with a scale factor of 2 making the texture twice as big in that dimension etc.
[ use mapping node scale x and y ]
'''

@ogredoc
class _ogredoc_TEX_wave_xform( INFO_MT_ogre_helper ):
	mydoc = '''
wave_xform
	@scale_x sine 1.0 0.2 0.0 5.0

Sets up a transformation animation based on a wave function. Useful for more advanced texture layer transform effects. You can add multiple instances of this attribute to a single texture layer if you wish.

Format: wave_xform <xform_type> <wave_type> <base> <frequency> <phase> <amplitude>

Example: wave_xform scale_x sine 1.0 0.2 0.0 5.0

xform_type:
	scroll_x
		Animate the x scroll value
	scroll_y
		Animate the y scroll value
	rotate
		Animate the rotate value
	scale_x
		Animate the x scale value
	scale_y
		Animate the y scale value

wave_type
	sine
		A typical sine wave which smoothly loops between min and max values
	triangle
		An angled wave which increases & decreases at constant speed, changing instantly at the extremes
	square
		Max for half the wavelength, min for the rest with instant transition between
	sawtooth
		Gradual steady increase from min to max over the period with an instant return to min at the end.
	inverse_sawtooth
		Gradual steady decrease from max to min over the period, with an instant return to max at the end.

base
	The base value, the minimum if amplitude > 0, the maximum if amplitude < 0
frequency
	The number of wave iterations per second, i.e. speed
phase
	Offset of the wave start
amplitude
	The size of the wave

The range of the output of the wave will be {base, base+amplitude}. So the example above scales the texture in the x direction between 1 (normal size) and 5 along a sine wave at one cycle every 5 second (0.2 waves per second).
'''

@ogredoc
class _ogredoc_TEX_transform( INFO_MT_ogre_helper ):
	mydoc = '''
transform
	@m00 m01 m02 m03 m10 m11 m12 m13 m20 m21 m22 m23 m30 m31 m32 m33
This attribute allows you to specify a static 4x4 transformation matrix for the texture unit, thus replacing the individual scroll, rotate and scale attributes mentioned above. 
Format: transform m00 m01 m02 m03 m10 m11 m12 m13 m20 m21 m22 m23 m30 m31 m32 m33
The indexes of the 4x4 matrix value above are expressed as m<row><col>.
'''

def _mesh_entity_helper( doc, ob, o ):
	## extended format - BGE Physics ##
	o.setAttribute('mass', str(ob.game.mass))
	o.setAttribute('mass_radius', str(ob.game.radius))
	o.setAttribute('physics_type', ob.game.physics_type)
	o.setAttribute('actor', str(ob.game.use_actor))
	o.setAttribute('ghost', str(ob.game.use_ghost))
	o.setAttribute('velocity_min', str(ob.game.velocity_min))
	o.setAttribute('velocity_max', str(ob.game.velocity_max))

	o.setAttribute('lock_trans_x', str(ob.game.lock_location_x))
	o.setAttribute('lock_trans_y', str(ob.game.lock_location_y))
	o.setAttribute('lock_trans_z', str(ob.game.lock_location_z))

	o.setAttribute('lock_rot_x', str(ob.game.lock_rotation_x))
	o.setAttribute('lock_rot_y', str(ob.game.lock_rotation_y))
	o.setAttribute('lock_rot_z', str(ob.game.lock_rotation_z))

	o.setAttribute('anisotropic_friction', str(ob.game.use_anisotropic_friction))
	x,y,z = ob.game.friction_coefficients
	o.setAttribute('friction_x', str(x))
	o.setAttribute('friction_y', str(y))
	o.setAttribute('friction_z', str(z))

	o.setAttribute('damping_trans', str(ob.game.damping))
	o.setAttribute('damping_rot', str(ob.game.rotation_damping))

	o.setAttribute('inertia_tensor', str(ob.game.form_factor))

	mesh = ob.data
	## custom user props ##
	for prop in mesh.items():
		propname, propvalue = prop
		if not propname.startswith('_'):
			user = doc.createElement('user_data')
			o.appendChild( user )
			user.setAttribute( 'name', propname )
			user.setAttribute( 'value', str(propvalue) )
			user.setAttribute( 'type', type(propvalue).__name__ )


# Ogre supports .dds in both directx and opengl
# http://www.ogre3d.org/forums/viewtopic.php?f=5&t=46847
IMAGE_FORMATS = {
	'dds',
	'png',
	'jpg',
}

OptionsEx = {
#	'mesh-sub-dir' : False,
#	'shape-anim' : True,
#	'trim-bone-weights' : 0.01,
#	'armature-anim' : True,

#	'nuextremityPoints' : 0,
#	'generateEdgeLists' : False,

#	'generateTangents' : False,
#	'tangentSemantic' : "uvw", 
#	'tangentUseParity' : 4,
#	'tangentSplitMirrored' : False,
#	'tangentSplitRotated' : False,
#	'reorganiseBuffers' : True,
#	'optimiseAnimations' : True,

}



class INFO_OT_createOgreExport(bpy.types.Operator):              
	'''Export Ogre Scene'''                   
	bl_idname = "ogre.export"    
	bl_label = "Export Ogre"               
	bl_options = {'REGISTER', 'UNDO'}      

	filepath= StringProperty( name="File Path",
			description="Filepath used for exporting Ogre .scene file",
			maxlen=1024, default="", subtype='FILE_PATH' )

	_axis_modes =  [
		('-x z y', '-x z y', 'default'),
		('x z -y', 'x z -y', 'old default'),
		('x z y', 'x z y', 'swap y and z'),
		('x y z', 'x y z', 'no swapping'),
	]
	EX_SWAP_MODE = EnumProperty( items=_axis_modes, name='swap axis',
			description='axis swapping mode', default='-x z y' )

	## Options ##
	EX_SCENE = BoolProperty(name="Export Scene",
			description="export current scene (OgreDotScene xml)", default=True)
	EX_SELONLY = BoolProperty(name="Export Selected Only", description="export selected", default=False)
	EX_FORCE_CAMERA = BoolProperty(name="Force Camera", description="export active camera", default=True)
	EX_FORCE_LAMPS = BoolProperty(name="Force Lamps", description="export all lamps", default=True)

	EX_MESH = BoolProperty(name="Export Meshes", description="export meshes", default=True)
	EX_MESH_OVERWRITE = BoolProperty(name="Export Meshes (overwrite)",
			description="export meshes (overwrite existing files)", default=True)

	EX_MATERIALS = BoolProperty(name="Export Materials", description="exports .material script", default=True)

	# Use custom inherited ogre materials
	EX_CUSTOM_MATERIALS = BoolProperty(name="Export Custom GLSL Materials",
			description="uses custom inherited materials rather than Ogre's default materials", default=True)

	@classmethod
	def poll(cls, context):
		return True

	def invoke(self, context, event):
		wm = context.window_manager
		wm.fileselect_add(self)		# writes to filepath
		return {'RUNNING_MODAL'}

	def execute(self, context):
		self.ogre_export(self.filepath, context);
		return {'FINISHED'}

	def dot_material(self, meshes, path='/tmp'):
		print('updating .material')
		mats = []
		for ob in meshes:
			if len(ob.data.materials):
				for mat in ob.data.materials:
					if mat not in mats:
						mats.append( mat )
		if not mats:
			print('WARNING: no materials, not writting .material script');
			return

		M = MISSING_MATERIAL + '\n'
		for mat in mats:
			Report.materials.append( mat.name )
			M += self.gen_dot_material( mat, path, convert_textures=True )
		basepath = os.path.splitext(self.filepath)[0]
		url = basepath + '.material'
		f = open( url, 'wb' );
		f.write( bytes(M,'utf-8') );
		f.close()
		print('saved', url)

	## python note: classmethods prefer attributes defined at the classlevel,
	# kinda makes sense, (even if called by an instance)
	@classmethod
	def gen_dot_material( self, mat, path='/tmp', convert_textures=False ):
		M = ''
		M += 'material %s \n{\n'		%mat.name
		if mat.use_shadows: M += indent(1, 'receive_shadows on')
		else: M += indent(1, 'receive_shadows off')

		M += indent(1, 'technique', '{' )	# technique GLSL
		M += self.gen_dot_material_pass( mat, path, convert_textures )

		M += indent(1, '}' )	# end technique
		M += '}\n'	# end material
		return M

	@classmethod
	def gen_dot_material_pass( self, mat, path, convert_textures ):
		print('gen_dot_material_pass', mat)
		OPTIONS['PATH'] = path
		M = ''
		print('		standard material')
		tree = ShaderTree( material=mat )
		M += tree.dotmat_pass()
		return M


	def _export_ogre_mesh( self, ob, path='/tmp'):
		export_ogre_mesh(ob, path=path)

	def _write_environment(self, xml_doc, xml_scene, world):
		## environ settings ##
		environ = xml_doc.createElement('environment')
		xml_scene.appendChild(environ)

		_c = {'colourAmbient':world.ambient_color, 'colourBackground':world.horizon_color}

		for ctag in _c:
			a = xml_doc.createElement(ctag)
			environ.appendChild(a)
			color = _c[ctag]
			a.setAttribute('r', '%s'%color.r)
			a.setAttribute('g', '%s'%color.g)
			a.setAttribute('b', '%s'%color.b)
		if world.mist_settings.use_mist:
			fog = xml_doc.createElement('fog')
			environ.appendChild(fog)
			fog.setAttribute('linearStart', '%s'%world.mist_settings.start )
			# only linear supported?
			fog.setAttribute('mode', world.mist_settings.falloff.lower() )
			fog.setAttribute('linearEnd', '%s' %(world.mist_settings.start+world.mist_settings.depth))



	def ogre_export(self, url, context):
		timer = Timer()
		global OPTIONS
		OPTIONS['TOUCH_TEXTURES'] = True
		OPTIONS['SWAP_AXIS'] = self.EX_SWAP_MODE
		Report.reset()

		print('ogre export->', url)
		prefix = url.split('.')[0]

		now = time.time()
		doc = dom.Document()
		xml_scene = doc.createElement('scene');
		doc.appendChild(xml_scene)

		# Set the header to the scene
		xml_scene.setAttribute('formatVersion', '1.0.0')

		############################

		## extern files ##
		xml_extern = doc.createElement('externals')
		xml_scene.appendChild(xml_extern)

		item = doc.createElement('item');
		xml_extern.appendChild(item)
		item.setAttribute('type','material')
		a = doc.createElement('file');
		item.appendChild( a )
		# .material file (scene mats)
		# FIXME material name
		a.setAttribute('name', '%s.material' %context.scene.name)

		self._write_environment(xml_doc=doc, xml_scene=xml_scene, world = context.scene.world)

		## nodes (objects) ##
		objects = []
		# gather because macros will change selection state
		for ob in bpy.data.objects:
			if self.EX_SELONLY and not ob.select:
				if ob.type == 'CAMERA' and self.EX_FORCE_CAMERA:
					pass
				elif ob.type == 'LAMP' and self.EX_FORCE_LAMPS:
					pass
				else:
					continue
			objects.append(ob)

		## gather roots because ogredotscene supports parents and children ##
		# TODO this is quite frankly f up, I can't understand it
		def _flatten( _c, _f ):
			if _c.parent in objects:
				_f.append( _c.parent )
			if _c.parent:
				_flatten( _c.parent, _f )
			else:
				_f.append( _c )

		roots = []
		for ob in objects:
			flat = []
			_flatten( ob, flat )
			root = flat[-1]
			if root not in roots:
				roots.append( root )

		xml_nodes = doc.createElement('nodes')
		xml_scene.appendChild(xml_nodes)

		export_meshes = []
		for root in roots:
			print('--------------- exporting root ->', root)
			self._write_node( root, 
				doc=doc,
				meshes=export_meshes,
				objects=objects, 
				xmlparent=xml_nodes 
				)

		self._export_ogre_meshes(export_meshes, url)

		if self.EX_SCENE:
			self._write_scene_file(url, doc)

		if self.EX_MATERIALS:
			self.dot_material(export_meshes, os.path.split(url)[0])

		bpy.ops.wm.call_menu( name='Ogre_User_Report' )
		print( 'Exporting took ', ('%.3f'%(timer.elapsedSecs())), 's' )


	def _export_ogre_meshes(self, meshes, url):
		# don't export same data multiple times
		already_exported = []
		for mesh in meshes:
			# What url is this really?
			murl = os.path.join( os.path.split(url)[0], '%s.mesh'%mesh.data.name )
			exists = os.path.isfile(murl)
			if self.EX_MESH_OVERWRITE or not exists:
				if mesh.data.name not in already_exported:
					self._export_ogre_mesh(mesh, path=os.path.split(url)[0])
					already_exported.append(mesh.data.name)


	def _write_scene_file(self, url, xml_doc):
		data = xml_doc.toprettyxml()
		if not url.endswith('.scene'):
			url += '.scene'
		f = open( url, 'wb' );
		f.write( bytes(data,'utf-8') );
		f.close()
		print('ogre scene dumped', url)


	############# node export - recursive ###############
	def _write_node(self, ob, doc, meshes=[], objects=[], xmlparent=None):
		xml_obj = _ogre_node_helper( doc=doc, ob=ob, objects=objects )
		xmlparent.appendChild(xml_obj)

		## custom user props ##
		for prop in ob.items():
			propname, propvalue = prop
			if not propname.startswith('_'):
				user = doc.createElement('user_data')
				xml_obj.appendChild( user )
				user.setAttribute( 'name', propname )
				user.setAttribute( 'value', str(propvalue) )
				user.setAttribute( 'type', type(propvalue).__name__ )

		if ob.type == 'MESH' and len(ob.data.faces):
			self._write_entity( ob,
					doc=doc,
					meshes=meshes,
					objects=objects,
					xmlparent=xml_obj)

		elif ob.type == 'CAMERA':
			self._write_camera(ob, doc, xml_par=xml_obj)

		elif ob.type == 'LAMP':
			self._write_light(ob, doc, xml_par=xml_obj)

		for child in ob.children:
			self._write_node( child, doc=doc,
				meshes=meshes, objects=objects, xmlparent=xml_obj )

	## end _write_node

	def _write_light( self, ob, doc, xml_par=None):
		Report.lights.append( ob.name )
		light = doc.createElement('light')
		xml_par.appendChild(light);
		light.setAttribute('name', ob.data.name)
		light.setAttribute('type', ob.data.type.lower() )

		# Export attenuation
		att = doc.createElement('lightAttenuation');
		light.appendChild( att )
		# is range an Ogre constant?
		# FIXME these parameters are messed up
		att.setAttribute('range', '5000' )
		att.setAttribute('constant', '1.0')		
		att.setAttribute('linear', '%s'%(1.0/ob.data.distance))
		# TODO support quadratic light
		att.setAttribute('quadratic', '0.0')


		## actually need to precompute light brightness by adjusting colors below ##
		if ob.data.use_diffuse:
			diff = doc.createElement('colourDiffuse');
			light.appendChild(diff)
			diff.setAttribute('r', '%s'%ob.data.color.r)
			diff.setAttribute('g', '%s'%ob.data.color.g)
			diff.setAttribute('b', '%s'%ob.data.color.b)

		if ob.data.use_specular:
			spec = doc.createElement('colourSpecular');
			light.appendChild(spec)
			spec.setAttribute('r', '%s'%ob.data.color.r)
			spec.setAttribute('g', '%s'%ob.data.color.g)
			spec.setAttribute('b', '%s'%ob.data.color.b)

		## bug reported by C.L.B ##
		# hemi lights should actually provide info for fragment/vertex-program ambient shaders
		if ob.data.type != 'HEMI' and ob.data.shadow_method != 'NOSHADOW':
			light.setAttribute('castShadows','true')
		else:
			light.setAttribute('castShadows','false')

	## end _light_export

	def _write_camera( self, ob, doc, xml_par=None):
		Report.cameras.append( ob.name )
		cam = doc.createElement('camera')
		xml_par.appendChild(cam);
		cam.setAttribute('name', ob.data.name)
		aspx = bpy.context.scene.render.pixel_aspect_x
		aspy = bpy.context.scene.render.pixel_aspect_y
		sx = bpy.context.scene.render.resolution_x
		sy = bpy.context.scene.render.resolution_y
		fovY = 0.0
		if (sx*aspx > sy*aspy):
			fovY = 2*math.atan(sy*aspy*16.0/(ob.data.lens*sx*aspx))
		else:
			fovY = 2*math.atan(16.0/ob.data.lens)
		# fov in degree
		fov = fovY*180.0/math.pi
		cam.setAttribute('fov', '%s'%fov)
		cam.setAttribute('projectionType', "perspective")
		clip = doc.createElement('clipping');
		cam.appendChild(clip)
		clip.setAttribute('nearPlaneDist', '%s' %ob.data.clip_start)
		clip.setAttribute('farPlaneDist', '%s' %ob.data.clip_end)
	## end _camera_export

	#def _write_entity(self, ent, doc)

	# TODO separate the aux node stuff to another function
	# FIXME exported name is the name of the mesh which is not unique replace with separate name
	def _write_entity(self, ob, doc, entities=[], meshes=[], objects=[], xmlparent=None):
		entities.append(ob)
		ent = doc.createElement('entity') 
		xmlparent.appendChild(ent);

		ent.setAttribute('name', ob.data.name)
		ent.setAttribute('meshFile', '%s.mesh' %(ob.data.name) )

		_mesh_entity_helper(doc, ob, ent)

		if self.EX_MESH:
			if ob not in meshes:
				meshes.append(ob)

		## deal with Array mod ##
		vecs = [ ob.matrix_world.to_translation() ]
		for mod in ob.modifiers:
			if mod.type == 'ARRAY':
				if mod.fit_type != 'FIXED_COUNT':
					print( 'WARNING: unsupport array-modifier type->', mod.fit_type )
					continue
				if not mod.use_constant_offset:
					print( 'WARNING: unsupport array-modifier mode, must be "constant offset" type' )
					continue
				else:
					newvecs = []
					for prev in vecs:
						for i in range( mod.count-1 ):
							v = prev + mod.constant_offset_displace
							newvecs.append( v )
							ao = _ogre_node_helper( doc=doc, ob=ob, objects=objects,
									prefix='_array_%s_'%len(vecs+newvecs), pos=v )
							xmlparent.appendChild(ao)

							ent = doc.createElement('entity') 
							ao.appendChild(e); ent.setAttribute('name', ob.data.name)
							ent.setAttribute('meshFile', '%s.mesh' %ob.data.name)
					vecs += newvecs

	## end _write_entity

def _ogre_node_helper( doc, ob, objects, prefix='', pos=None, rot=None, scl=None ):
	mat = ob.matrix_local

	o = doc.createElement('node')
	o.setAttribute('name',prefix+ob.name)
	p = doc.createElement('position')
	q = doc.createElement('quaternion')
	s = doc.createElement('scale')
	for n in (p,q,s):
		o.appendChild(n)

	if pos: v = swap(pos)
	else: v = swap( mat.to_translation() )
	p.setAttribute('x', '%6f'%v.x)
	p.setAttribute('y', '%6f'%v.y)
	p.setAttribute('z', '%6f'%v.z)

	if rot: v = swap(rot)
	else: v = swap( mat.to_quaternion() )
	q.setAttribute('x', '%6f'%v.x)
	q.setAttribute('y', '%6f'%v.y)
	q.setAttribute('z', '%6f'%v.z)
	q.setAttribute('w','%6f'%v.w)

	if scl:		# this should not be used
		v = swap(scl)
		x=abs(v.x); y=abs(v.y); z=abs(v.z)
		s.setAttribute('x', '%6f'%x)
		s.setAttribute('y', '%6f'%y)
		s.setAttribute('z', '%6f'%z)
	else:		# scale is different in Ogre from blender - rotation is removed
		ri = mat.to_quaternion().inverted().to_matrix()
		scale = ri.to_4x4() * mat
		v = swap( scale.to_scale() )
		x=abs(v.x); y=abs(v.y); z=abs(v.z)
		s.setAttribute('x', '%6f'%x)
		s.setAttribute('y', '%6f'%y)
		s.setAttribute('z', '%6f'%z)

	return o


def find_bone_index( ob, arm, groupidx):	# sometimes the groups are out of order, this finds the right index.
	vg = ob.vertex_groups[ groupidx ]
	for i,bone in enumerate(arm.pose.bones):
		if bone.name == vg.name: return i

def mesh_is_smooth( mesh ):
	for face in mesh.faces:
		if face.use_smooth: return True

import pyogre

# Ogre Mesh binary exporter #
def export_ogre_mesh( ob, path='/tmp'):
	print('Exporting Ogre mesh ', ob.name, " to ", path)
	if not os.path.isdir(path):
		print('creating directory', path)
		os.makedirs(path)

	copy = ob.copy()
	rem = []
	# remove armature and array modifiers before collaspe
	for mod in copy.modifiers:
		if mod.type in 'ARMATURE ARRAY'.split(): rem.append( mod )
	for mod in rem: copy.modifiers.remove( mod )

	## hijack blender's edge-split modifier to make this easy ##
	# The edge-split is needed to get the Texture coordinates correct
	# Without them a vertex is mapped to texture coordinates of multiple faces
	# Minimal bloating by only duplicating the vertices that have seams in them
	# so we preserve the uv coordinates.
	# TODO should not use the EDGE_SPLIT for this, because it will screw smooth
	# normals completely. We need to copy the vertices, just like edge split does
	# but we want them to have same normals.
	# Good thing though that this will only screw smooth notmals for the seams.
	if copy.data.uv_textures.active:
		e = copy.modifiers.new('_hack_', type='EDGE_SPLIT')
		e.use_edge_angle = False
		e.use_edge_sharp = True
		for edge in copy.data.edges:
			# Only duplicate edges with seams
			# (and those that already are marked as sharp)
			if(edge.use_seam):
				edge.use_edge_sharp = True

	## bake mesh ##
	mesh = copy.to_mesh(bpy.context.scene, True, "PREVIEW")	# collaspe

	# This shouldn't happen and it's not the sub mesh but the mesh
	if not len(mesh.faces):		# bug fix dec10th, reported by Matti
		print('ERROR : mesh without faces, skipping!', ob)
		return;

	# Use the copy for report as we use it for exporting
	Report.meshes.append(mesh.name)
	Report.faces += len(mesh.faces)
	Report.vertices += len(mesh.vertices)

	writer = pyogre.MeshWriter()
	og_mesh = writer.createMesh()

	# FIXME add armature support
#	arm = ob.find_armature()
#	if arm:
#		skel = doc.createElement('skeletonlink')
#		skel.setAttribute('name', '%s.skeleton' %(ob.data.name) )
#		root.appendChild( skel )

	## verts ##
	"""
	if arm:
		bweights = doc.createElement('boneassignments')
		root.appendChild( bweights )

	if opts['shape-anim'] and ob.data.shape_keys and len(ob.data.shape_keys.keys):
		print('	writing shape animation')

		poses = doc.createElement('poses'); root.appendChild( poses )
		for sidx, skey in enumerate(ob.data.shape_keys.keys):
			if sidx == 0: continue
			pose = doc.createElement('pose'); poses.appendChild( pose )
			pose.setAttribute('name', skey.name)
			pose.setAttribute('index', str(sidx-1))

			pose.setAttribute('target', 'mesh')		# If target is 'mesh', targets the shared geometry, if target is submesh, targets the submesh identified by 'index'. 
			for i,p in enumerate( skey.data ):
				x,y,z = swap( ob.data.vertices[i].co - p.co )
				if x==.0 and y==.0 and z==.0: continue		# the older exporter optimized this way, is it safe?
				po = doc.createElement('poseoffset'); pose.appendChild( po )
				po.setAttribute('x', '%6f' %x)
				po.setAttribute('y', '%6f' %y)
				po.setAttribute('z', '%6f' %z)
				po.setAttribute('index', str(i))		# this was 3 values in old exporter, from triangulation?

		if ob.data.shape_keys.animation_data and len(ob.data.shape_keys.animation_data.nla_tracks):
			anims = doc.createElement('animations'); root.appendChild( anims )
			for nla in ob.data.shape_keys.animation_data.nla_tracks:
				strip = nla.strips[0]
				anim = doc.createElement('animation'); anims.appendChild( anim )
				anim.setAttribute('name', nla.name)		# do not use the action's name
				anim.setAttribute('length', str((strip.frame_end-strip.frame_start)/30.0) )		# TODO proper fps
				tracks = doc.createElement('tracks'); anim.appendChild( tracks )
				track = doc.createElement('track'); tracks.appendChild( track )
				track.setAttribute('type','pose')
				track.setAttribute('target','mesh')
				track.setAttribute('index','0')
				keyframes = doc.createElement('keyframes')
				track.appendChild( keyframes )


				for frame in range( int(strip.frame_start), int(strip.frame_end), 2):		# every other frame
					bpy.context.scene.frame_current = frame
					keyframe = doc.createElement('keyframe')
					keyframes.appendChild( keyframe )
					keyframe.setAttribute('time', str(frame/30.0))
					for sidx, skey in enumerate( ob.data.shape_keys.keys ):
						if sidx == 0: continue
						poseref = doc.createElement('poseref'); keyframe.appendChild( poseref )
						poseref.setAttribute('poseindex', str(sidx))
						poseref.setAttribute('influence', str(skey.value) )
	"""

	# Used timing infomation
	vertices_time = 0

	badverts = 0
	
	uvOfVertex = {}
	
	## texture maps - vertex dictionary ##
	if mesh.uv_textures.active:
		for layer in mesh.uv_textures:
			uvOfVertex[layer] = {}
			for fidx, uvface in enumerate(layer.data):
				face = mesh.faces[ fidx ]
				for vertex in face.vertices:
					if vertex not in uvOfVertex[layer]:
						uv = uvface.uv[ list(face.vertices).index(vertex) ]
						uvOfVertex[layer][vertex] = uv
	
	# Get the vertex buffer
	for vidx, v in enumerate(mesh.vertices):
		""" FIXME bone support
		if arm:
			check = 0
			for vgroup in v.groups:
				if vgroup.weight > opts['trim-bone-weights']:		#self.EX_TRIM_BONE_WEIGHTS:		# optimized
					bnidx = find_bone_index(ob,arm,vgroup.group)
					if bnidx is not None:		# allows other vertex groups, not just armature vertex groups
						vba = doc.createElement('vertexboneassignment')
						bweights.appendChild( vba )
						vba.setAttribute( 'vertexindex', str(vidx) )
						vba.setAttribute( 'boneindex', str(bnidx) )
						vba.setAttribute( 'weight', str(vgroup.weight) )
						check += 1
			if check > 4:
				badverts += 1
				print('WARNING: vertex %s is in more than 4 vertex groups (bone weights)\n(this maybe Ogre incompatible)' %vidx)
		"""

		og_vertex = pyogre.Vertex()

		x,y,z = swap( v.co )

		# position
		og_vertex.position = pyogre.Vector3(x,y,z)

		x,y,z = swap( v.normal )
		og_vertex.normal = pyogre.Vector3(x,y,z)


		## vertex colors ##
		"""	TODO not implemented
		if len( mesh.vertex_colors ):		# TODO need to do proper face lookup for color1,2,3,4
			#vb.setAttribute('colours_diffuse','true')		# fixed Dec3rd
			#cd = doc.createElement( 'colour_diffuse' )	#'color_diffuse')
			#vertex.appendChild( cd )
			vchan = mesh.vertex_colors[0]

			valpha = None	## hack support for vertex color alpha
			for bloc in mesh.vertex_colors:
				if bloc.name.lower().startswith('alpha'):
					valpha = bloc; break

			for f in mesh.faces:
				if vidx in f.vertices:
					k = list(f.vertices).index(vidx)
					r,g,b = getattr( vchan.data[ f.index ], 'color%s'%(k+1) )
					if valpha:
						ra,ga,ba = getattr( valpha.data[ f.index ], 'color%s'%(k+1) )
						cd.setAttribute('value', '%6f %6f %6f %6f' %(r,g,b,ra) )	
						og_vertex.diffuse = pyogre.ColourValue(x,y,z)
					else:
						cd.setAttribute('value', '%6f %6f %6f 1.0' %(r,g,b) )
						og_vertex.diffuse = pyogre.ColourValue(x,y,z)
					break
		"""

		## texture maps ##
		# TODO add support for multiple texture coordinates
		if mesh.uv_textures.active:
			layer = mesh.uv_textures.active
			#for layer in mesh.uv_textures:
			if vidx in uvOfVertex[layer]:
				uv = uvOfVertex[layer][vidx]
				og_vertex.uv = pyogre.Vector2(uv[0], 1.0-uv[1])
		
		"""
		if mesh.uv_textures.active:
			layer = mesh.uv_textures.active
			#for layer in mesh.uv_textures:
			for fidx, uvface in enumerate(layer.data):
				face = mesh.faces[ fidx ]
				if vidx in face.vertices:
					uv = uvface.uv[ list(face.vertices).index(vidx) ]
					og_vertex.uv = pyogre.Vector2(uv[0], 1.0-uv[1])
					break
		"""

		og_mesh.addVertex(og_vertex)
	## end vert loop

	if( len(mesh.uv_textures) > 1):
		print("Warning mesh with multiple texture coordinates, only first one is exported")

	if badverts:
		warning_msg = '%s has %s vertices weighted to too many bones'
		'(Ogre limits a vertex to 4 bones)\n'
		'[try increaseing the Trim-Weights threshold option]' %(mesh.name, badverts)
		Report.warnings.append(warning_msg)


	######################################################
	used_materials = []
	matnames = []
	for mat in ob.data.materials:
		if mat: matnames.append( mat.name )
		else:
			print('warning: bad material data', ob)
			matnames.append( '_missing_material_' )		# fixed dec22, keep proper index
	if not matnames:
		matnames.append( '_missing_material_' )

	# Data structure for dividing the Blender mesh to Ogre SubMeshes
	class MeshData:
		material_name = ""
		# Set of tri tuples of vertex indexes
		faces = []
		# Already swapped Ogre vector
		face_normals = []

	mesh_datas = []
	for matidx, matname in enumerate( matnames ):
		mdata = MeshData()
		mdata.material_name = matname

		## faces ##
		for F in mesh.faces:
			## skip faces not in this material index ##
			if F.material_index != matidx:
				continue
			if matname not in used_materials:
				used_materials.append( matname )

			## Ogre only supports triangles, so we will split quads
			mdata.faces.append((F.vertices[0], F.vertices[1], F.vertices[2]))
			x, y, z = swap(F.normal)
			normal = pyogre.Vector3(x, y, z) 
			mdata.face_normals.append(normal)
			if len(F.vertices) >= 4:
				mdata.faces.append((F.vertices[0], F.vertices[2], F.vertices[3]))
				mdata.face_normals.append(normal)

		# Do not add empty sub meshes, not sure if this is necessary though
		if(len(mdata.faces) > 0):
			mesh_datas.append(mdata)

	for m in mesh_datas:
		print("creating submesh with material name : ", m.material_name)
		sm = og_mesh.createSubMesh()
		sm.material = m.material_name

		# Preallocate the faces for efficiency
		sm.allocateFaces(len(m.faces))

		for i, F in enumerate(m.faces):
			sm.setFace(i, F[0], F[1], F[2])
			sm.setFaceNormal(i, m.face_normals[i])

	# Calculate smooth normals
	# This isn't working properly, but seems like we don't need it
	# after removing the EDGE_SPLIT hack from start of the exporter
	#if(mesh.use_auto_smooth):
	#	print('calculating smooth normals with angle ', mesh.auto_smooth_angle)
	#	og_mesh.smoothNormals(pyogre.Radian(pyogre.Degree(mesh.auto_smooth_angle)))


	bpy.data.objects.remove(copy)
	bpy.data.meshes.remove(mesh)

	name = ob.data.name

	meshfile = os.path.join(path, '%s.mesh' %(name) )
	writer.writeMesh(og_mesh, meshfile)

	""" FIXME no bone support
	if arm and opts['armature-anim']:		#self.EX_ANIM:
		skel = Skeleton( ob )
		data = skel.to_xml()
		name = force_name or ob.data.name
		xmlfile = os.path.join(path, '%s.skeleton.xml' %(name) )
		f = open( xmlfile, 'wb' )
		f.write( bytes(data,'utf-8') )
		f.close()
		OgreXMLConverter( xmlfile, opts )
	"""

	mats = []
	for name in used_materials:
		if name != '_missing_material_':
			mats.append( bpy.data.materials[name] )
	return mats
## end export_ogre_mesh##

class Bone(object):
	''' EditBone
	['__doc__', '__module__', '__slots__', 'align_orientation', 'align_roll', 'bbone_in', 'bbone_out', 'bbone_segments', 'bl_rna', 'envelope_distance', 'envelope_weight', 'head', 'head_radius', 'hide', 'hide_select', 'layers', 'lock', 'matrix', 'name', 'parent', 'rna_type', 'roll', 'select', 'select_head', 'select_tail', 'show_wire', 'tail', 'tail_radius', 'transform', 'use_connect', 'use_cyclic_offset', 'use_deform', 'use_envelope_multiply', 'use_inherit_rotation', 'use_inherit_scale', 'use_local_location']

	'''
	def update(self):		# called on frame update
		pose = self.bone.matrix * self.skeleton.object_space_transformation
		pose =  self.skeleton.object_space_transformation * self.bone.matrix
		self._inverse_total_trans_pose = pose.copy().invert()

		# calculate difference to parent bone
		if self.parent:
			pose = self.parent._inverse_total_trans_pose * pose
		elif self.fixUpAxis:
			pose = mathutils.Matrix(((1,0,0,0),(0,0,-1,0),(0,1,0,0),(0,0,0,1))) * pose

		# get transformation values
		# translation relative to parent coordinate system orientation
		# and as difference to rest pose translation
		#blender2.49#translation -= self.ogreRestPose.translationPart()
		self.pose_location =  pose.to_translation()  -  self.ogre_rest_matrix.to_translation()
		# rotation (and scale) relative to local coordiante system
		# calculate difference to rest pose
		#blender2.49#poseTransformation *= self.inverseOgreRestPose
		#pose = pose * self.inverse_ogre_rest_matrix		# this was wrong, fixed Dec3rd
		pose = self.inverse_ogre_rest_matrix * pose
		self.pose_rotation = pose.to_quaternion()
		self.pose_scale = pose.to_scale().copy()

		#self.pose_location = self.bone.location.copy()
		#self.pose_rotation = self.bone.rotation_quaternion.copy()


		for child in self.children: child.update()


	def __init__(self, matrix, pbone, skeleton):
		self.skeleton = skeleton
		self.name = pbone.name
		self.matrix = matrix
		#self.matrix *= mathutils.Matrix([1,0,0,0],[0,0,-1,0],[0,1,0,0],[0,0,0,1])
		self.bone = pbone		# safe to hold pointer to pose bone, not edit bone!
		if not pbone.bone.use_deform: print('warning: bone <%s> is non-deformabled, this is inefficient!' %self.name)
		#TODO test#if pbone.bone.use_inherit_scale: print('warning: bone <%s> is using inherit scaling, Ogre has no support for this' %self.name)
		self.parent = pbone.parent
		self.children = []
		self.fixUpAxis = True

	def rebuild_tree( self ):		# called first on all bones
		if self.parent:
			self.parent = self.skeleton.get_bone( self.parent.name )
			self.parent.children.append( self )

	def compute_rest( self ):	# called second, only on root bones
		if self.parent:
			inverseParentMatrix = self.parent.inverse_total_trans
		elif (self.fixUpAxis):
			inverseParentMatrix = mathutils.Matrix(((1,0,0,0),(0,0,-1,0),(0,1,0,0),(0,0,0,1)))
		else:
			inverseParentMatrix = mathutils.Matrix(((1,0,0,0),(0,1,0,0),(0,0,1,0),(0,0,0,1)))

		# bone matrix relative to armature object
		self.ogre_rest_matrix = self.matrix.copy()
		# relative to mesh object origin
		#self.ogre_rest_matrix *= self.skeleton.object_space_transformation		# 2.49 style
		self.ogre_rest_matrix = self.skeleton.object_space_transformation * self.ogre_rest_matrix

		# store total inverse transformation
		self.inverse_total_trans = self.ogre_rest_matrix.copy().invert()
		# relative to OGRE parent bone origin
		#self.ogre_rest_matrix *= inverseParentMatrix		# 2.49 style
		self.ogre_rest_matrix = inverseParentMatrix * self.ogre_rest_matrix
		self.inverse_ogre_rest_matrix = self.ogre_rest_matrix.copy().invert()

		## recursion ##
		for child in self.children:
			child.compute_rest()

class Skeleton(object):
	def get_bone( self, name ):
		for b in self.bones:
			if b.name == name: return b

	def __init__(self, ob ):
		self.object = ob
		self.bones = []
		mats = {}
		self.arm = arm = ob.find_armature()
		## ob.pose.bones[0].matrix is not the same as ob.data.edit_bones[0].matrix
		#bug?#layers = list(arm.layers)
		arm.hide = False
		arm.layers = [True]*20
		prev = bpy.context.scene.objects.active
		bpy.context.scene.objects.active = arm		# arm needs to be in edit mode to get to .edit_bones
		bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
		bpy.ops.object.mode_set(mode='EDIT', toggle=False)
		for bone in arm.data.edit_bones: mats[ bone.name ] = bone.matrix.copy()
		bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
		#bpy.ops.object.mode_set(mode='POSE', toggle=False)
		bpy.context.scene.objects.active = prev

		for pbone in arm.pose.bones:
			mybone = Bone( mats[pbone.name] ,pbone, self )
			self.bones.append( mybone )

		if arm.name not in Report.armatures: Report.armatures.append( arm.name )

		# additional transformation for root bones:
		# from armature object space into mesh object space, i.e.,
		# (x,y,z,w)*AO*MO^(-1)
		self.object_space_transformation = arm.matrix_local * ob.matrix_local.copy().invert()
		#self.object_space_transformation = ob.matrix_local.copy().invert() * arm.matrix_local 

		## setup bones for Ogre format ##
		for b in self.bones: b.rebuild_tree()
		## walk bones, convert them ##
		self.roots = []
		for b in self.bones:
			if not b.parent:
				b.compute_rest()
				self.roots.append( b )

	def to_xml( self ):
		doc = dom.Document()
		root = doc.createElement('skeleton'); doc.appendChild( root )
		bones = doc.createElement('bones'); root.appendChild( bones )
		bh = doc.createElement('bonehierarchy'); root.appendChild( bh )
		for i,bone in enumerate(self.bones):
			b = doc.createElement('bone')
			b.setAttribute('name', bone.name)
			b.setAttribute('id', str(i) )
			bones.appendChild( b )
			mat = bone.ogre_rest_matrix.copy()
			if bone.parent:
				bp = doc.createElement('boneparent')
				bp.setAttribute('bone', bone.name)
				bp.setAttribute('parent', bone.parent.name)
				bh.appendChild( bp )

			pos = doc.createElement( 'position' ); b.appendChild( pos )
			x,y,z = mat.to_translation()
			pos.setAttribute('x', '%6f' %x )
			pos.setAttribute('y', '%6f' %y )
			pos.setAttribute('z', '%6f' %z )
			rot =  doc.createElement( 'rotation' )		# note "rotation", not "rotate"
			b.appendChild( rot )

			q = mat.to_quaternion()
			rot.setAttribute('angle', '%6f' %q.angle )
			axis = doc.createElement('axis'); rot.appendChild( axis )
			x,y,z = q.axis
			axis.setAttribute('x', '%6f' %x )
			axis.setAttribute('y', '%6f' %y )
			axis.setAttribute('z', '%6f' %z )
			## Ogre bones do not have initial scaling? ##
			if 0:
				scale = doc.createElement('scale'); b.appendChild( scale )
				x,y,z = swap( mat.to_scale() )
				scale.setAttribute('x', str(x))
				scale.setAttribute('y', str(y))
				scale.setAttribute('z', str(z))
			## NOTE: Ogre bones by default do not pass down their scaling in animation, in blender all bones are like 'do-not-inherit-scaling'

		arm = self.arm
		if arm.animation_data:
			anims = doc.createElement('animations'); root.appendChild( anims )
			if not len( arm.animation_data.nla_tracks ):
				Report.warnings.append('you must assign an NLA strip to armature (%s) that defines the start and end frames' %arm.name)

			for nla in arm.animation_data.nla_tracks:		# NLA required, lone actions not supported
				if not len(nla.strips): continue
				if len( nla.strips ) > 1:
					Report.warnings.append('NLA track with multiple strips on armature (%s), using only the first strip, use multiple NLA tracks not multiple NLA strips.' %arm.name)
				nla.mute = False		# TODO is this required?
				strip = nla.strips[0]
				anim = doc.createElement('animation'); anims.appendChild( anim )
				tracks = doc.createElement('tracks'); anim.appendChild( tracks )
				Report.armature_animations.append( '%s : %s [start frame=%s  end frame=%s]' %(arm.name, nla.name, strip.frame_start, strip.frame_end) )

				anim.setAttribute('name', nla.name)		# do not use the action's name
				anim.setAttribute('length', str( (strip.frame_end-strip.frame_start)/30.0 ) )		# TODO proper fps
				## using the fcurves directly is useless, because:
				## we need to support constraints and the interpolation between keys
				## is Ogre smart enough that if a track only has a set of bones, then blend animation with current animation?
				## the exporter will not be smart enough to know which bones are active for a given track...
				## can hijack blender NLA, user sets a single keyframe for only selected bones, and keys last frame
				stripbones = []
				for group in strip.action.groups:		# check if the user has keyed only some of the bones (for anim blending)
					if group.name in arm.pose.bones: stripbones.append( group.name )
				if not stripbones:									# otherwise we use all bones
					for bone in arm.pose.bones: stripbones.append( bone.name )
				print('NLA-strip:',  nla.name)
				_keyframes = {}
				for bonename in stripbones:
					track = doc.createElement('track')
					track.setAttribute('bone', bonename)
					tracks.appendChild( track )
					keyframes = doc.createElement('keyframes')
					track.appendChild( keyframes )
					_keyframes[ bonename ] = keyframes
					print('\t', bonename)

				for frame in range( int(strip.frame_start), int(strip.frame_end), 2):		# every other frame
					#bpy.context.scene.frame_current = frame
					bpy.context.scene.frame_set(frame)
					#self.object.update( bpy.context.scene )
					#bpy.context.scene.update()
					for bone in self.roots: bone.update()
					print('\t\t', frame)
					for bonename in stripbones:
						bone = self.get_bone( bonename )
						_loc = bone.pose_location
						_rot = bone.pose_rotation
						_scl = bone.pose_scale

						keyframe = doc.createElement('keyframe')
						keyframe.setAttribute('time', str(frame/30.0))
						_keyframes[ bonename ].appendChild( keyframe )
						trans = doc.createElement('translate')
						keyframe.appendChild( trans )
						x,y,z = _loc
						trans.setAttribute('x', '%6f' %x)
						trans.setAttribute('y', '%6f' %y)
						trans.setAttribute('z', '%6f' %z)

						rot =  doc.createElement( 'rotate' )		# note "rotate" - bug fixed Dec2nd
						keyframe.appendChild( rot )
						q = _rot #swap( mat.to_quaternion() )
						rot.setAttribute('angle', '%6f' %q.angle )
						axis = doc.createElement('axis'); rot.appendChild( axis )
						x,y,z = q.axis
						axis.setAttribute('x', '%6f' %x )
						axis.setAttribute('y', '%6f' %y )
						axis.setAttribute('z', '%6f' %z )

						scale = doc.createElement('scale')
						keyframe.appendChild( scale )
						x,y,z = _scl #swap( mat.to_scale() )
						scale.setAttribute('x', '%6f' %x)
						scale.setAttribute('y', '%6f' %y)
						scale.setAttribute('z', '%6f' %z)


		return doc.documentElement.toprettyxml()


def get_image_textures( mat ):
	r = []
	for s in mat.texture_slots:
		if s and s.texture.type == 'IMAGE': r.append( s )
	return r


def indent( level, *args ):
	if not args: return '\t' * level
	else:
		a = ''
		for line in args:
			a += '\t' * level
			a += line
			a += '\n'

		return a

############ extra tools ##############


def gather_instances():
	instances = {}
	for ob in bpy.data.objects:
		if ob.data and ob.data.users > 1:
			if ob.data not in instances: instances[ ob.data ] = []
			instances[ ob.data ].append( ob )
	return instances

def select_instances( context, name ):
	for ob in bpy.data.objects: ob.select = False
	ob = bpy.data.objects[ name ]
	if ob.data:
		inst = gather_instances()
		for ob in inst[ ob.data ]: ob.select = True
		bpy.context.scene.objects.active = ob


def select_group( context, name, options={} ):
	for ob in bpy.data.objects: ob.select = False
	for grp in bpy.data.groups:
		if grp.name == name:
			#context.scene.objects.active = grp.objects
			#Note that the context is read-only. These values cannot be modified directly, though they may be changed by running API functions or by using the data API. So bpy.context.object = obj will raise an error. But bpy.context.scene.objects.active = obj will work as expected. - http://wiki.blender.org/index.php?title=Dev:2.5/Py/API/Intro&useskin=monobook
			bpy.context.scene.objects.active = grp.objects[0]
			for ob in grp.objects: ob.select = True
		else: pass

#############
class INFO_MT_actors(bpy.types.Menu):
	bl_label = "Actors"
	def draw(self, context):
		layout = self.layout
		for ob in bpy.data.objects:
			if ob.game.use_actor:
				op = layout.operator("select_actor", text=ob.name)
				op.mystring = ob.name
		layout.separator()

class INFO_MT_actor(bpy.types.Operator):                
	'''select actor'''
	bl_idname = "ogre.select_actor"
	bl_label = "Select Actor"
	# Options for this panel type
	bl_options = {'REGISTER', 'UNDO'}
	mystring= StringProperty(name="MyString", description="...", maxlen=1024, default="my string")
	@classmethod
	def poll(cls, context): return True
	def invoke(self, context, event):
		bpy.data.objects[self.mystring].select = True
		return {'FINISHED'}

class INFO_MT_dynamics(bpy.types.Menu):
	bl_label = "Dynamics"
	def draw(self, context):
		layout = self.layout
		for ob in bpy.data.objects:
			if ob.game.physics_type in 'DYNAMIC SOFT_BODY RIGID_BODY'.split():
				op = layout.operator("select_dynamic", text=ob.name)
				op.mystring = ob.name
		layout.separator()

class INFO_MT_dynamic(bpy.types.Operator):                
	'''select dynamic'''
	bl_idname = "ogre.select_dynamic"
	bl_label = "Select Dynamic"
	bl_options = {'REGISTER', 'UNDO'}                              # Options for this panel type
	mystring= StringProperty(name="MyString", description="...", maxlen=1024, default="my string")

	@classmethod
	def poll(cls, context):
		return True

	def invoke(self, context, event):
		bpy.data.objects[self.mystring].select = True
		return {'FINISHED'}

def export_menu_func(self, context):
	#ext = os.path.splitext(bpy.app.binary_path)[-1]
	#default_blend_path = bpy.data.filepath.replace(".blend", ext)
	path,name = os.path.split( context.blend_data.filepath )
	op = self.layout.operator("ogre.export", text="Ogre3D (.scene)")
	# TODO should split the path using splitext and only replace the ext
	# if it's equal to scene
	op.filepath=os.path.join( path, name.split('.')[0]+'.scene' )

def register():
	print( VERSION )
	# Register operators
	#bpy.utils.register_class( Ogre_toggle_prop_op )
	bpy.utils.register_class( Ogre_relocate_textures_op )
	bpy.utils.register_class( Ogre_create_collision_op )
	# Register INFOs
	bpy.utils.register_class( INFO_MT_ogre_helper )
	bpy.utils.register_class( INFO_MT_ogre_docs )
	bpy.utils.register_class( INFO_MT_dynamics )
	bpy.utils.register_class( INFO_MT_dynamic )
	bpy.utils.register_class( INFO_MT_actors )
	bpy.utils.register_class( INFO_OT_createOgreExport )
	bpy.utils.register_class( INFO_MT_actor )
	# Register something, user interface panels etc.
	bpy.utils.register_class( Ogre_User_Report )
	bpy.utils.register_class( Ogre_Physics )
	bpy.utils.register_class( Ogre_Material_Panel )
	bpy.utils.register_class( Ogre_Texture_Panel )

	bpy.types.INFO_MT_file_export.append(export_menu_func)

def unregister():
	print('unreg-> ogre exporter')
	#bpy.utils.unregister_class( Ogre_toggle_prop_op )
	bpy.utils.unregister_class( Ogre_relocate_textures_op )
	bpy.utils.unregister_class( Ogre_create_collision_op )
	# unregister INFOs
	bpy.utils.unregister_class( INFO_MT_ogre_helper )
	bpy.utils.unregister_class( INFO_MT_ogre_docs )
	bpy.utils.unregister_class( INFO_MT_dynamics )
	bpy.utils.unregister_class( INFO_MT_dynamic )
	bpy.utils.unregister_class( INFO_MT_actors )
	bpy.utils.unregister_class( INFO_OT_createOgreExport )
	bpy.utils.unregister_class( INFO_MT_actor )
	bpy.utils.unregister_class( Ogre_User_Report )
	bpy.utils.unregister_class( Ogre_Physics )
	bpy.utils.unregister_class( Ogre_Material_Panel )
	bpy.utils.unregister_class( Ogre_Texture_Panel )

	bpy.types.INFO_MT_file_export.remove(export_menu_func)

if __name__ == "__main__": register()

