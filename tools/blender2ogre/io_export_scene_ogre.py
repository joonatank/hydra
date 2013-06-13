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


VERSION = 'Ogre Exporter v37'
__devnotes__ = '''
Oct16	. Fixed copy textures (works only with unpacked textures)
		. Uses at maximum a precission of 6, removing trailing zeros.
		. Removed bloating of the scene file with the physics attributes.
		. Remove more useless tool panels.
		. Removed useless external files and user attributes.
		By Joonatan Kuosa

Jul16	. Added splitting of vertices and preserving normals for uv mapped objects by Joonatan Kuosa.

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

def float_to_string(f):
	return '{0:.6g}'.format(f)

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
Q: how do i export extra information to my game engine via OgreDotScene?
A: You can assign 'Custom Properties' to objects in blender, and these will be saved to the .scene file.

Q: what version of this script am i running?
A: %s
''' %VERSION


_doc_installing_ = '''
Installing:
	Installing the Addon:
		Have a look at the included README.txt

	Required:
		1. Blender 2.63
'''

## ICONS are in blender/editors/include/UI_icons.h
## TODO are 1e-10 exponent numbers ok in the xml files?

bl_info = {
    "name": "OGRE Exporter (.scene, .mesh)",
    "author": "Joonatan Kuosa",
    "version": (0,3,7),
    "blender": (2, 6, 3),
    "api": 36339,
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
					if ob not in obs:
						obs.append( ob )
					break
	return obs

def get_materials_using_image( img ):
	mats = []
	for mat in bpy.data.materials:
		for slot in get_image_textures( mat ):
			if slot.texture.image == img:
				if mat not in mats: mats.append( mat )
	return mats

class ReportSingleton(object):
	def show(self):
		bpy.ops.wm.call_menu( name='Ogre_User_Report' )

	def __init__(self):
		self.reset()

	def reset(self):
		self.materials = []
		self.textures = []
		self.meshes = []
		self.lights = []
		self.cameras = []
		self.armatures = []
		self.armature_animations = []
		self.shape_animations = []
		self.vertices = 0
		self.faces = 0
		self.triangles = 0
		self.warnings = []
		self.errors = []
		self.messages = []
		self.paths = []
		self.time = 0.0
		self.triangle_mesh_time = 0.0
		self.fixing_normals_time = 0.0

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

		r.append('  Total Export time: %.3f seconds' %self.time)
		if self.triangle_mesh_time != 0.0:
			r.append('  Time it took to create triangle meshes : %.3f seconds' %self.triangle_mesh_time)
		if self.fixing_normals_time != 0.0:
			r.append('  Time it took to fix normals : %.3f seconds' %self.fixing_normals_time )

		if self.vertices:
			r.append('  Total Vertices: %s' %self.vertices)
			r.append('  Total Faces: %s' %self.faces)
			r.append('  Total Triangles: %s' %self.triangles)
			## TODO report file sizes, meshes and textures

		# TODO fix this to something bit more readable
		for tag in 'meshes lights cameras armatures armature_animations shape_animations materials textures'.split():
			attr = getattr(self, tag)
			if attr:
				name = tag.replace('_',' ').upper()
				r.append( '  %s: %s' %(name, len(attr)) )
				if attr:
					ex.append( '  %s:' %name )
					for a in attr:
						ex.append( '    . %s' %a )

		txt = '\n'.join( r )
		ex = '\n'.join( ex )		# console only - extended report
		print('_'*80)
		print(txt)
		print(ex)
		print('_'*80)
		return txt

Report = ReportSingleton()

class Ogre_User_Report(bpy.types.Menu):
	bl_label = "Mini-Report | (see console for full report)"
	def draw(self, context):
		layout = self.layout
		txt = Report.report()
		for line in txt.splitlines():
			layout.label(text=line)

##################################################################
_game_logic_intro_doc_ = '''
Hijacking the BGE

Blender contains a fully functional game engine (BGE) that is highly useful for learning the concepts of game programming by breaking it down into three simple parts: Sensor, Controller, and Actuator.  An Ogre based game engine will likely have similar concepts in its internal API and game logic scripting.  Without a custom interface to define game logic, very often game designers may have to resort to having programmers implement their ideas in purely handwritten script.  This is prone to breakage because object names then end up being hard-coded.  Not only does this lead to non-reusable code, its also a slow process.  Why should we have to resort to this when Blender already contains a very rich interface for game logic?  By hijacking a subset of the BGE interface we can make this workflow between game designer and game programmer much better.

The OgreDocScene format can easily be extened to include extra game logic data.  While the BGE contains some features that can not be easily mapped to other game engines, there are many are highly useful generic features we can exploit, including many of the Sensors and Actuators.  Blender uses the paradigm of: 1. Sensor -> 2. Controller -> 3. Actuator.  In pseudo-code, this can be thought of as: 1. on-event -> 2. conditional logic -> 3. do-action.  The designer is most often concerned with the on-events (the Sensors), and the do-actions (the Actuators); and the BGE interface provides a clear way for defining and editing those.  Its a harder task to provide a good interface for the conditional logic (Controller), that is flexible enough to fit everyones different Ogre engine and requirements, so that is outside the scope of this exporter at this time.  A programmer will still be required to fill the gap between Sensor and Actuator, but hopefully his work is greatly reduced and can write more generic/reuseable code.

The rules for which Sensors trigger which Actuators is left undefined, as explained above we are hijacking the BGE interface not trying to export and reimplement everything.  BGE Controllers and all links are ignored by the exporter, so whats the best way to define Sensor/Actuator relationships?  One convention that seems logical is to group Sensors and Actuators by name.  More complex syntax could be used in Sensor/Actuators names, or they could be completely ignored and instead all the mapping is done by the game programmer using other rules.  This issue is not easily solved so designers and the engine programmers will have to decide upon their own conventions, there is no one size fits all solution.
'''


##################################################################

OPTIONS = {
	'SWAP_AXIS' : '-x z y',
}

_ogre_doc_classic_textures_ = '''
Ogre texture blending is far more limited than Blender's texture slots.  While many of the blending options are the same or similar, only the blend mode "Mix" is allowed to have a variable setting.  All other texture blend modes are either on or off, for example you can not use the "Add" blend mode and set the amount to anything other than fully on (1.0).  The user also has to take into consideration the hardware multi-texturing limitations of their target platform - for example the GeForce3 can only do four texture blend operations in a single pass.  Note that Ogre will fallback to multipass rendering when the hardware won't accelerate it.

==Supported Blending Modes:==
	* Mix			- blend_manual -
	* Multiply		- modulate -
	* Screen		- modulate_x2 -
	* Lighten		- modulate_x4 -
	* Add			- add -
	* Subtract		- subtract -
	* Overlay		- add_signed -
	* Difference		- dotproduct -

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

def has_property( a, name ):
	for prop in a.items():
		n,val = prop
		if n == name: return True


def guess_uv_layer( layer ):
	## small issue: in blender layer is a string, multiple objects may have the same material assigned, 
	## but having different named UVTex slots, most often the user will never rename these so they get
	## named UVTex.000 etc...   assume this to always be true.
	idx = 0
	if '.' in layer:
		a = layer.split('.')[-1]
		if a.isdigit(): idx = int(a)+1
		else:
			print('warning: it is not allowed to give custom names to UVTexture channels ->', layer)
	return idx

def wordwrap( txt ):
	r = ['']
	for word in txt.split(' '):	# do not split on tabs
		word = word.replace('\t', ' '*3)
		r[-1] += word + ' '
		if len(r[-1]) > 90: r.append( '' )
	return r

_OGRE_DOCS_ = []
def ogredoc( cls ):
	tag = cls.__name__.split('_ogredoc_')[-1]

	cls.bl_label = tag.replace('_', ' ')
	_OGRE_DOCS_.append( cls )

	return cls


class INFO_MT_ogre_helper(bpy.types.Menu):
	bl_label = 'ogre_helper'
	def draw(self, context):
		layout = self.layout

		for line in self.mydoc.splitlines():
			if line.strip():
				for ww in wordwrap( line ):
					layout.label(text=ww)
		layout.separator()


class INFO_MT_ogre_docs(bpy.types.Menu):
	bl_label = "Ogre Help"
	def draw(self, context):
		layout = self.layout
		for cls in _OGRE_DOCS_:
			layout.menu( cls.__name__ )
			layout.separator()
		layout.separator()
		layout.label(text='bug reports to: joonatan.kuosa@savantsimulators.com')

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
		location, rotation, scale
		environment colors
		fog settings
		lights, colors
		optimize instances
		selected only

	Export .mesh
		vertices, normals, one set of uvs

	Export .material
		diffuse color
		ambient intensity
		emission
		specular
		receive shadows on/off
		multiple materials per mesh

	Export .skeleton
		NA
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
class _ogredoc_Warnings( INFO_MT_ogre_helper ):
	mydoc = '''
General Warnings:
	. extra vertex groups, can mess up an armature weights (new vgroups must come after armature assignment, not before)
	. no sharp vertex color edges (face level vertex colors)
	. quadratic lights falloff not supported (needs pre calc)
	. do not enable subsurf modifier on meshes that have shape or armature animation.  
		(Any modifier that changes the vertex count is bad with shape anim or armature anim)
'''


@ogredoc
class _ogredoc_Bugs( INFO_MT_ogre_helper ):
	mydoc = '''
Known Issues:
	. UV coordinates that are disconnected and have no seams will be exported incorrectly
	. Skeleton exporting is not supported
	. Bones and bone weights are not exported
	. Only single set of uv coordinates is exported
	- Scale is not exported correctly, some issues with rotations at least for negative scales.
'''


############ Ogre v.17 Doc ######


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

# Ogre supports .dds in both directx and opengl
# http://www.ogre3d.org/forums/viewtopic.php?f=5&t=46847
IMAGE_FORMATS = {
	'dds',
	'png',
	'jpg',
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

	EX_MESH = BoolProperty(name="Export Meshes", description="export meshes", default=True)
	EX_MESH_OVERWRITE = BoolProperty(name="Export Meshes (overwrite)",
			description="export meshes (overwrite existing files)", default=True)

	EX_MATERIALS = BoolProperty(name="Export Materials", description="exports .material script", default=True)

	EX_COPY_TEXTURES = BoolProperty(name="Copy Textures",
			description="Copies the texture files to export directory.",
			default=True)

	# Use custom inherited ogre materials
	EX_CUSTOM_MATERIALS = BoolProperty(name="Export Custom GLSL Materials",
			description="uses custom inherited materials rather than Ogre's default materials", default=False)

	@classmethod
	def poll(cls, context):
		return True

	def invoke(self, context, event):
		wm = context.window_manager
		wm.fileselect_add(self)		# writes to filepath
		return {'RUNNING_MODAL'}

	def execute(self, context):
		self.export_scene(self.filepath, context);
		return {'FINISHED'}

	# @param materials list of materials to export
	# @param file_name the name of the material file, usually scene name
	# @param path to the export directory
	def dot_material(self, materials, file_name, file_dir):
		print('updating .material')

		if not materials:
			Report.warnings.append('no materials, not writting .material script');
			return

		M = MISSING_MATERIAL + '\n'
		for mat in materials:
			Report.materials.append(mat.name)
			M += (self.gen_dot_material(mat) + '\n')

		# Write the material file
		if not file_name.endswith('.material'):
			file_name += '.material'
		url = os.path.join(file_dir, file_name)
		print('Writing material file to ', url)
		f = open( url, 'wb' )
		f.write( bytes(M,'utf-8') )
		f.close()
	# End of dot_material #


	## python note: classmethods prefer attributes defined at the classlevel,
	# kinda makes sense, (even if called by an instance)
	# @param mat the blender material
	# @return the ogre material string
	@classmethod
	def gen_dot_material(self, mat):
		M = ''
		# TODO add support for custom GLSL materials
		M += 'material %s \n{\n'		%mat.name
		if mat.use_shadows:
			M += indent(1, 'receive_shadows on')
		else:
			M += indent(1, 'receive_shadows off')

		# technique GLSL
		M += indent(1, 'technique', '{' )

		# Multiple passes are not supported
		M += indent(2, 'pass', '{')

		# Depth write and diffuse lighting should be off
		mat.use_sky

		# Invisible object, only used for shadow casting
		# Not sure what Ogre parameter this is
		#mat.use_cast_shadows_only

		# Ambient parameters
		ambient = convert_color(mat.diffuse_color, mat.ambient, mat.alpha)
		line = 'ambient ' + convert_list_to_string(ambient)
		M += indent(3, line)
		emissive = convert_color(mat.diffuse_color, mat.emit, mat.alpha)
		line = 'emissive ' + convert_list_to_string(emissive)
		M += indent(3, line)

		# Diffuse
		# shader parammeters not supported for now
		# mat.diffuse_shader
		diff = convert_color(mat.diffuse_color, mat.diffuse_intensity, mat.alpha)
		line = 'diffuse ' + convert_list_to_string(diff)
		M += indent(3, line)

		# Specular
		# shader parammeters not supported for now
		# mat.specular_shader
		spec = convert_color(mat.specular_color, mat.specular_intensity, mat.specular_alpha)
		spec.append(mat.specular_hardness)
		line = 'specular ' + convert_list_to_string(spec)
		M += indent(3, line)
		# If we have transparency and also we should take depth write off
		# TODO should this be done or not?
		if mat.use_transparency and mat.alpha < 1.0:
			M += indent(3, 'depth_write off')

		# TODO add vertex colour
		#mat.use_vertex_color_paint

		# Textures
		if mat.active_texture:
			M += textures_to_string(mat)

		M += indent(2, '}' )	# end pass
		M += indent(1, '}' )	# end technique
		M += '}\n'	# end material

		return M

	def _write_environment(self, xml_doc, xml_scene, world):
		## environ settings ##
		environ = xml_doc.createElement('environment')
		xml_scene.appendChild(environ)

		_c = {'colourAmbient':world.ambient_color, 'colourBackground':world.horizon_color}

		for ctag in _c:
			a = xml_doc.createElement(ctag)
			environ.appendChild(a)
			color = _c[ctag]

			a.setAttribute('r', float_to_string(color.r))
			a.setAttribute('g', float_to_string(color.g))
			a.setAttribute('b', float_to_string(color.b))
		if world.mist_settings.use_mist:
			fog = xml_doc.createElement('fog')
			environ.appendChild(fog)
			fog.setAttribute('linearStart', float_to_string(world.mist_settings.start))
			# only linear supported?
			fog.setAttribute('mode', world.mist_settings.falloff.lower() )
			fog.setAttribute('linearEnd', float_to_string(world.mist_settings.start+world.mist_settings.depth))



	def export_scene(self, url, context):
		timer = Timer()
		global OPTIONS
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
		self._write_environment(xml_doc=doc, xml_scene=xml_scene, world = context.scene.world)

		## nodes (objects) ##
		objects = []
		# gather because macros will change selection state
		# This doesn't export objects not linked to the Blender scene
		for ob in context.scene.objects:
			if self.EX_SELONLY and not ob.select:
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
			self._write_node( root, doc=doc,
				meshes=export_meshes,
				xmlparent=xml_nodes )

		basepath = os.path.splitext(self.filepath)[0]
		#mesh_dir = os.path.split(url)[0]
		mesh_dir = os.path.split(basepath)[0]
		if len(export_meshes) > 0 :
			materials = self._export_ogre_meshes(export_meshes, mesh_dir)
		else :
			Report.warnings.append("No Meshes to export.")
			materials = []

		if self.EX_SCENE:
			self._write_scene_file(url, doc)

		if self.EX_MATERIALS and len(materials) > 0 :
			material_path = os.path.split(basepath)

			material_dir = material_path[0]
			self.dot_material(materials, file_name=material_path[1], file_dir=material_dir)
			if self.EX_COPY_TEXTURES:
				texture_dir = os.path.join(material_dir, "textures")
				textures = []
				for mat in materials:
					textures += collect_used_textures(mat)

				print("Copying textures to ", texture_dir)
				if len(textures) > 0:
					copy_textures(textures, texture_dir)

		Report.time = timer.elapsedSecs()

		# Needs to be the last call so that we have all the reported issues
		bpy.ops.wm.call_menu( name='Ogre_User_Report' )


	# @brief exports meshes does checking for not exporting a mesh multiple times
	# @param meshes, a list of meshes to export can contain same mesh multiple times
	# @param url, the directory used to export the meshes
	# @return a list of materials used by the meshes exported
	# @todo fix the url to directory instead of scene.file or something
	def _export_ogre_meshes(self, meshes, mesh_dir):
		# Create the export directory if necessary
		# TODO this does not work if the mesh_dir exists and is a file
		if not os.path.isdir(mesh_dir):
			print('creating directory', path)
			os.makedirs(path)

		# don't export same data multiple times
		already_exported = []
		materials = []
		for mesh in meshes:
			# What url is this really?
			murl = os.path.join(mesh_dir, '%s.mesh' %mesh.data.name)
			exists = os.path.isfile(murl)
			if self.EX_MESH_OVERWRITE or not exists:
				if mesh.data.name not in already_exported:
					materials.extend(export_ogre_mesh(mesh, file_path=murl))
					already_exported.append(mesh.data.name)

		# Cleanup doubles
		return list(set(materials))
	# end of _export_ogre_meshes #


	def _write_scene_file(self, url, xml_doc):
		data = xml_doc.toprettyxml()
		if not url.endswith('.scene'):
			url += '.scene'
		f = open( url, 'wb' );
		f.write( bytes(data,'utf-8') );
		f.close()
		print('ogre scene dumped', url)


	############# node export - recursive ###############
	def _write_node(self, ob, doc, meshes=[], xmlparent=None):
		xml_obj = _ogre_node_helper(doc=doc, ob=ob)
		xmlparent.appendChild(xml_obj)

		## custom user props ##
		"""
		for prop in ob.items():
			propname, propvalue = prop
			if not propname.startswith('_'):
				user = doc.createElement('user_data')
				xml_obj.appendChild( user )
				user.setAttribute( 'name', propname )
				user.setAttribute( 'value', str(propvalue) )
				user.setAttribute( 'type', type(propvalue).__name__ )
		"""

		if ob.type == 'MESH' and len(ob.data.polygons):
			self._write_entity( ob,
					doc=doc,
					meshes=meshes,
					xmlparent=xml_obj)
		elif ob.type == "MESH" :
			Report.warnings.append("Empty Mesh " + ob.name + " not exporting.")

		elif ob.type == 'CAMERA':
			self._write_camera(ob, doc, xml_par=xml_obj)

		elif ob.type == 'LAMP':
			self._write_light(ob, doc, xml_par=xml_obj)

		for child in ob.children:
			self._write_node( child, doc=doc,
				meshes=meshes, xmlparent=xml_obj )

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
			diff.setAttribute('r', float_to_string(ob.data.color.r))
			diff.setAttribute('g', float_to_string(ob.data.color.g))
			diff.setAttribute('b', float_to_string(ob.data.color.b))

		if ob.data.use_specular:
			spec = doc.createElement('colourSpecular');
			light.appendChild(spec)
			spec.setAttribute('r', float_to_string(ob.data.color.r))
			spec.setAttribute('g', float_to_string(ob.data.color.g))
			spec.setAttribute('b', float_to_string(ob.data.color.b))

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

		
		clip.setAttribute('nearPlaneDist', float_to_string(ob.data.clip_start))
		clip.setAttribute('farPlaneDist', float_to_string(ob.data.clip_end))
	## end _camera_export

	#def _write_entity(self, ent, doc)

	# TODO separate the aux node stuff to another function
	# FIXME exported name is the name of the mesh which is not unique replace with separate name
	def _write_entity(self, ob, doc, entities=[], meshes=[], xmlparent=None):
		entities.append(ob)
		ent = doc.createElement('entity') 
		xmlparent.appendChild(ent);

		ent.setAttribute('name', ob.data.name)
		ent.setAttribute('meshFile', '%s.mesh' %(ob.data.name) )

		#_mesh_entity_helper(doc, ob, ent)

		if self.EX_MESH:
			if ob not in meshes:
				meshes.append(ob)

		## deal with Array mod ##
		""" We don't use
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
		"""

	## end _write_entity

from shutil import copy2

def is_image_texture(tex):
	valid = tex.type == 'IMAGE'
	if valid:
		valid = tex.image and tex.image.type == 'IMAGE' and tex.image.filepath != ''
	return valid

def copy_textures(textures, path):
	if not os.path.exists(path):
		print("Creating texture export directory : ", path)
		os.mkdir(path)
	elif os.path.exists(path) and not os.path.isdir(path):
		print("Changing texture export directory : ", path)
		path = os.path.dirname(path)

	for tex in textures:
		if is_image_texture(tex) :
			abs_path = bpy.path.abspath(tex.image.filepath)
			if(os.path.exists(abs_path)):
				copy2(abs_path, path)
				Report.textures.append(tex.name)
			else:
				warn = str(abs_path) + " does not exists. Texture not copied."
				Report.warnings.append(warn)
		else :
			warn = tex.name + " is not an image texture. Texture not copied."
			Report.warnings.append(warn)




# @brief get used textures for a material
# @return a list of used textures
def collect_used_textures(material):
	# TODO add support for multiple textures
	textures = []
	if material.active_texture:
		textures.append(material.texture_slots[material.active_texture_index].texture)

	# Get all texture slots that are used and valid (existing)
	for i, use in enumerate(material.use_textures):
		if use and i != material.active_texture_index and material.texture_slots[i]:
			textures.append(material.texture_slots[i].texture)

	return textures

# @brief convert blender materials textures to a string
# @param mat the blender material which textures to convert
# @return string containing the textures part of the material
def textures_to_string(mat):
	textures = collect_used_textures(mat)

	M = ''
	# Only file textures are supported
	for tex in textures:
		if is_image_texture(tex) :
			blender_tex_path = bpy.path.abspath(tex.image.filepath)
			tex_path = os.path.split(blender_tex_path)[1]
			if(len(tex_path) == 0):
				Report.warnings.append("Problem in the texture file path.")
			else :
				texture = 'texture ' + tex_path
				M += indent(3, 'texture_unit', '{')
				# FIXME the filepath should be the new one where textures are copied
				M += indent(4, texture)
				M += indent(3, '}')	#end texture_unit
				# TODO add scale, offset and other parameters

		else:
			s = str("texture type " + str(tex.type) + " used by texture " + tex.name + " is not supported.")
			Report.warnings.append(s)

	return M





def _ogre_node_helper(doc, ob, prefix=''):
	mat = ob.matrix_local

	node = doc.createElement('node')
	node.setAttribute('name',prefix+ob.name)
	xml_pos = doc.createElement('position')
	xml_q = doc.createElement('quaternion')
	xml_scale = doc.createElement('scale')
	for n in (xml_pos,xml_q,xml_scale):
		node.appendChild(n)

	v = swap(mat.to_translation())
	xml_pos.setAttribute('x', float_to_string(v.x))
	xml_pos.setAttribute('y', float_to_string(v.y))
	xml_pos.setAttribute('z', float_to_string(v.z))

	#q = swap(ob.rotation_quaternion)
	q = swap(mat.to_quaternion())
	xml_q.setAttribute('x', float_to_string(q.x))
	xml_q.setAttribute('y', float_to_string(q.y))
	xml_q.setAttribute('z', float_to_string(q.z))
	xml_q.setAttribute('w', float_to_string(q.w))

	# We may not swap the scale, this would mirror all our models :S
	# FIXME Scale does not work at this moment
	#s = swap(ob.scale)
	s = mat.to_scale()
	xml_scale.setAttribute('x', float_to_string(s.x))
	xml_scale.setAttribute('y', float_to_string(s.y))
	xml_scale.setAttribute('z', float_to_string(s.z))

	return node


# sometimes the groups are out of order, this finds the right index.
def find_bone_index( ob, arm, groupidx):
	vg = ob.vertex_groups[ groupidx ]
	for i,bone in enumerate(arm.pose.bones):
		if bone.name == vg.name:
			return i

def mesh_is_smooth( mesh ):
	for face in mesh.tessfaces:
		if face.use_smooth:
			return True

# @brief get the vertices used for the particular uv layer
# @param mesh
# @param layer the uv layer to get the list for
# @return the list containing vertices for the particular uvs
# @todo not fineshed!
def get_uv_vertex_map(mesh, layer=0):
	uvOfVertex = {}

	for layer in mesh.uv_layers:
		uvOfVertex[layer] = {}
		for fidx, uvface in enumerate(layer.data):
			face = mesh.tessfaces[ fidx ]
			for vertex in face.vertices:
				if vertex not in uvOfVertex[layer]:
					uv = uvface.uv[ list(face.vertices).index(vertex) ]
					uvOfVertex[layer][vertex] = uv

	return uvOfVertex

def has_uv_material(b_object):
	for mat_s in b_object.material_slots :
		for tex_s in mat_s.material.texture_slots:
			if tex_s != None:
				if tex_s.texture != None:
					if tex_s.texture.type == 'IMAGE' and tex_s.texture_coords == 'UV':
						if tex_s.texture.image.type == 'IMAGE':
							return True
	
	return False


import pyogre

# @brief Returns object copy with a new mesh linked to obj_copy.data
# @param obj blender object with a mesh to copy
# @return copy of the blender object with a copied mesh
# Converts the copied mesh to triangles and splits the vertices if it has
# face uvs and an uv mapped image texture.
#
# All scene states like selection and active object are returned to their initial values
# Creates two new data blocks, object copy and mesh copy. User needs to remove them.
#
# @todo Does not handle hidden objects correctly
def triangle_mesh_copy(obj) :
	copy = obj.copy()

	try:
		# remove armature and array modifiers before collaspe
		rem = []
		for mod in copy.modifiers:
			if mod.type in 'ARMATURE ARRAY'.split():
				rem.append( mod )
		for mod in rem:
			copy.modifiers.remove( mod )

		# Remapping Blender face indexes for meshes that need splitting
		is_splitted = has_uv_material(copy)

		# Assuming that the mesh has no duplicated vertices
		vertex_normal_map = {}
		for v in copy.data.vertices:
			v_t = (v.co[0], v.co[1], v.co[2])
			vertex_normal_map[v_t] = vertex_normal_map.get(v_t, v.normal)

		## hijack blender's edge-split modifier to make this easy ##
		# The edge-split is needed to get the Texture coordinates correct
		# Without them a vertex is mapped to texture coordinates of multiple faces
		# Will bloat the mesh because multiplies all of the vertices in the mesh.
		# Will preserve all UV coordinates though as long as the mesh has a uv texture
		# assigned to it when exporting.
		# Objects without uv coordinates or without uv textures in them are exported
		# using sticky uvs. For auto mapped uvs this is well suited and will minimise the
		# bloating across a scene (CAD models in particularly).
		# Also because this method is so slow (copying the normals) it should not
		# be used for huge objects.
		# O(n2) complexity where n is the number of vertices
		if is_splitted:
			e = copy.modifiers.new('_hack_', type='EDGE_SPLIT')
			e.use_edge_angle = False
			e.use_edge_sharp = True
			for edge in copy.data.edges:
				# All edges are splitted at the moment
				# TODO
				# should really find the ones that have vertices with two
				# different uv coordinates
				# to minimise the number of duplicated vertices
				# and also store a list of them so we can preserve the ones which were
				# marked as sharp
				edge.use_edge_sharp = True

		## bake mesh ##
		mesh = copy.to_mesh(bpy.context.scene, True, "PREVIEW")
		copy.data = mesh

		original_selection = bpy.context.selected_objects
		original_active_object = bpy.context.active_object
		for o in original_selection:
			o.select = False
		bpy.context.scene.objects.link(copy)
		copy.select = True
		copy.hide = False
		bpy.context.scene.objects.active = copy

		# enter edit mode, selection is only allowed there
		# FIXME this doesn't work for objects that are hidden
		# TODO test if this works for objects that are in a different layer
		# than the user is using
		bpy.ops.object.mode_set(mode='EDIT', toggle=False)

		# Select all faces
		bpy.ops.mesh.select_all(action='SELECT')
		bpy.ops.mesh.quads_convert_to_tris()

		# Invert states
		# This shouldn't be necessary as we are operating on the copy
		# FIXME this does not revert them to the ones they were before runing the script
		#bpy.ops.mesh.select_all(action='DESELECT')
		# Back to object mode because we need it for next phase
		bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
		#bpy.ops.object.select_all(action='DESELECT')

		# Return original state
		# TODO this should be wrapped into a class so we could do this in destructor
		# TODO does this need to switch the object mode back also?
		for obj in original_selection:
			obj.select = True
		bpy.context.scene.objects.active = original_active_object
		bpy.context.scene.objects.unlink(copy)

		# Clear sharp edges created by edge split
		# TODO this will remove all sharp edges from the mesh which is not what we want
		# we want only the edges that we created to handle face UV maps to be cleared
		if is_splitted:
			t = Timer()
			for v in mesh.vertices:
				v_t = (v.co[0], v.co[1], v.co[2])
				# This should never raise an exception as all vertices should be in the map
				v.normal = vertex_normal_map[v_t]
			Report.fixing_normals_time += t.elapsedSecs()

	except :
		# Cleanup created objects
		if copy:
			bpy.context.scene.objects.unlink(copy)
			mesh = copy.data
			bpy.data.objects.remove(copy)
			if mesh:
				bpy.data.meshes.remove(mesh)
		raise

	return copy;

	
# Ogre Mesh binary exporter #
# @param ob, mesh to export
# @param path, path where to write the mesh file
# @return all the Blender materials used by that mesh
def export_ogre_mesh(ob, file_path):
	print('Exporting Ogre mesh ', ob.name, " to ", file_path)

	Report.meshes.append(ob.name)

	# This shouldn't happen
	if not len(ob.data.polygons):
		Report.errors.append(ob.data.name + 'mesh without faces, skipping!')
		return [];

	# Return variable
	t = Timer()
	copy = triangle_mesh_copy(ob)
	# Bake the tessfaces
	copy.data.update(False, True)
	Report.triangle_mesh_time += t.elapsedSecs()

	# Use the copy for report as we use it for exporting
	Report.faces += len(copy.data.tessfaces)
	Report.vertices += len(copy.data.vertices)

	writer = pyogre.MeshWriter()
	og_mesh = writer.createMesh()

	mats = []
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

	if( len(copy.data.uv_layers) > 1):
		warn = "Mesh with multiple texture coordinates, only active is exported"
		Report.warnings.append(warn)

	# Single uv coordinate set (active) is supported
	# Maps vertices to uv coordinates as blender uses face uvs we need to
	# create a map for vertex uvs
	vertexUVmap = {}
	layer = copy.data.tessface_uv_textures.active
	if layer :
		already_existed = 0
		for fidx, uvface in enumerate(layer.data):
			face = copy.data.tessfaces[fidx]
			for vertex in face.vertices:
				uv = uvface.uv[ list(face.vertices).index(vertex) ]
				if vertex in vertexUVmap:
					already_existed += 1
				vertexUVmap[vertex] = uv
		if already_existed > 0:
			warn = str(already_existed) + " vertices already had their "\
				"UV coordinates and were overwriten."
			Report.warnings.append(warn)

	for vidx, v in enumerate(copy.data.vertices):
		og_vertex = pyogre.Vertex()

		x,y,z = swap(v.co)

		# position
		og_vertex.position = pyogre.Vector3(x,y,z)

		x,y,z = swap(v.normal)
		og_vertex.normal = pyogre.Vector3(x,y,z)

		# TODO add support for multiple texture coordinates
		if vidx in vertexUVmap:
			uv = vertexUVmap[vidx]
			og_vertex.uv = pyogre.Vector2(uv[0], 1.0-uv[1])
		
		og_mesh.addVertex(og_vertex)
	## end vert loop

	######################################################
	used_materials = []
	matnames = []
	for mat in ob.data.materials:
		material_name = ""
		if mat:
			material_name = mat.name

			matnames.append(mat.name)
			if mat.name not in used_materials:
				used_materials.append(mat.name)
		else:
			material_name = '_missing_material_'
			warn = ob.data.name + ' has bad material data'
			Report.warnings.append(warn)


	if not matnames:
		matnames.append( '_missing_material_' )

	# Data structure for dividing the Blender mesh to Ogre SubMeshes
	# variable faces : Set of tri tuples of vertex indexes
	# variable material_name : name of the material used by this mesh name
	class SubMeshData:
		def __init__(self):
			self.material_name = ""
			self.faces= []

	mesh_datas = []
	for matidx, matname in enumerate( matnames ):
		mdata = SubMeshData()
		mdata.material_name = matname

		## faces ##
		for fidx, F in enumerate(copy.data.tessfaces):
			## skip faces not in this material index ##
			if F.material_index == matidx:
				mdata.faces.append((F.vertices[0], F.vertices[1], F.vertices[2]))


		# Do not add empty sub meshes, not sure if this is necessary though
		if(len(mdata.faces) > 0):
			mesh_datas.append(mdata)

	for m in mesh_datas:
		sm = og_mesh.createSubMesh()
		sm.material = m.material_name

		# Preallocate the faces for efficiency
		sm.allocateFaces(len(m.faces))

		for i, F in enumerate(m.faces):
			sm.setFace(i, F[0], F[1], F[2])

	for i in range(og_mesh.getNumSubMeshes()):
		Report.triangles += og_mesh.getSubMesh(i).getNumFaces()

	writer.writeMesh(og_mesh, file_path)

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

	for name in used_materials:
		# TODO this check should be useless now anyway
		if name != '_missing_material_':
			mats.append( bpy.data.materials[name] )

	# Cleanup after exception handling
	mesh = copy.data
	bpy.data.objects.remove(copy)
	bpy.data.meshes.remove(mesh)

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
		if not pbone.bone.use_deform:
			print('warning: bone <%s> is non-deformabled, this is inefficient!' %self.name)
		#TODO test
		#if pbone.bone.use_inherit_scale:
		#	print('warning: bone <%s> is using inherit scaling, Ogre has no support for this' %self.name)
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

		if arm.name not in Report.armatures:
			Report.armatures.append( arm.name )

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
			## NOTE: Ogre bones by default do not pass down their
			# scaling in animation, in blender all bones are like
			# 'do-not-inherit-scaling'

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

				# do not use the action's name
				anim.setAttribute('name', nla.name)
				# TODO proper fps
				anim.setAttribute('length', str( (strip.frame_end-strip.frame_start)/30.0 ) )
				## using the fcurves directly is useless, because:
				## we need to support constraints and the interpolation between keys
				## is Ogre smart enough that if a track only has a set of bones,
				## then blend animation with current animation?
				## the exporter will not be smart enough to know which bones
				## are active for a given track...
				## can hijack blender NLA, user sets a single keyframe for only selected bones,
				## and keys last frame
				stripbones = []
				# check if the user has keyed only some of the bones (for anim blending)
				for group in strip.action.groups:	
					if group.name in arm.pose.bones: stripbones.append( group.name )
				# otherwise we use all bones
				if not stripbones:
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

				# every other frame
				for frame in range( int(strip.frame_start), int(strip.frame_end), 2):
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

						# note "rotate" - bug fixed Dec2nd
						rot =  doc.createElement( 'rotate' )
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

############ tools ##############

def get_image_textures( mat ):
	r = []
	for s in mat.texture_slots:
		if s and s.texture.type == 'IMAGE': r.append( s )
	return r

# @brief converts a list of floats to a string separated with a space
# @param l a list of floating point numbers
# Rounds the floats to 6 decimals, as it's the one used by Ogre
def convert_list_to_string(l):
	s = ''
	for elem in l:
		s += (float_to_string(elem) + ' ')
	return s

# @brief convert colour to list
# @param col the base three component colour to convert
# @param intnsity the base colour intensity
# @param alpha the alpha channel
# @return four element list
def convert_color(col, intensity=1.0, alpha=1.0):
	return [col.r*intensity, col.g*intensity, col.b*intensity, alpha]

# @brief indent correctly and write a number of lines
# @param level how many indentiation levels
# @param args lines to write
def indent( level, *args ):
	if not args: return '\t' * level
	else:
		a = ''
		for line in args:
			a += '\t' * level
			a += line
			a += '\n'

		return a

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

##############################################################################
##### Register ######
def export_menu_func(self, context):
	#ext = os.path.splitext(bpy.app.binary_path)[-1]
	#default_blend_path = bpy.data.filepath.replace(".blend", ext)
	path,name = os.path.split( context.blend_data.filepath )
	op = self.layout.operator("ogre.export", text="Ogre3D (.scene)")
	# TODO should split the path using splitext and only replace the ext
	# if it's equal to scene
	op.filepath=os.path.join( path, name.split('.')[0]+'.scene' )

def register():
	bpy.utils.register_module(__name__)

	bpy.types.INFO_MT_file_export.append(export_menu_func)

def unregister():
	bpy.utils.unregister_module(__name__)

	bpy.types.INFO_MT_file_export.remove(export_menu_func)

if __name__ == "__main__":
	register()

