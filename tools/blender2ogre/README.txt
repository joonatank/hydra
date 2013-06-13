Maintainer: Joonatan Kuosa <joonatan.kuosa@tut.fi>

Released: 2011-07-16
Version: 0.3.4

Blender version: only 2.57 release or newer is supported
Older versions: 2.4* and 2.5* betas will not work because of API changes.

Tested: versions
2.57	r36339
2.58	r37702
2.58.1	r38019

Tested configurations:
- Only default installation directory has been tested.
- {USER} plugin directory has not been tested.
- Test computer had python 3.2 installed, this shouldn't be necessary for blender though.
- Not tested importing old settings when updating Blender (from 2.57 to 2.58)


Installation:
- Install MSVC redist 2010 x86_64 from microsoft.com OR
copy the included mscp100.dll and msvcr100.dll with other dlls
to addons directory.
- Copy all files (README not necessary) to Blender addons directory
${BLENDER_INSTALL_DIR}/${BLENDER_VERSION}/scripts/addons
Or the application data directory in ${USER}/Blender/${BLENDER_VERSION}/scripts/addons
- Start Blender
- Go to User Preferences (Editor Window)
	- Addons
	- Import/Export
	- Find OGRE Exporter
	- Enable addon

Configuration:
Enabling the addon automatically in Blender start
	- Start new scene
	- Enable the Ogre exporter
	- Press "Save as Default" button in the lower toolbar
- Mind that Blender saves the whole scene as default,
so better not have a large model open
- Also Blender saves the user interface.

RELEASE NOTES:
Fixes from 0.3.3
- UV mapped objects with disjoint uvs (islands) are exported correctly

Fixes from 0.3.2
- Multi material support is working
- Exporting textures other than images now displays a warning in the console
but does not interrupt the exporting.
- Transparent materials have depth exported correctly in material files.
- Renamed to correspond to Blender add-on naming scheme.

Fixes from 0.3.1
- Correct normals are exported by default (so no more hard edges) except for UV seams.
- Almost halved the amount of vertices exported for a model.
- Lots of cleanup of unused GUI windows.

Does not work?
- Try running 'import pyogre' in the python console,
if it fails something wrong with the dlls
if it works then something wrong with the python script.
- Something wrong with the script?
	- Open system console from Help -> Toggle System Console
	- Press F8 to reload all addons
	- Check system console for errors
- Still problems, email maintenance :D
	- post the error logs from console (python or system) if you can

Common problems:

	Exported models have their insides out.
- Either a problem with normals or the material has a 'depth write off' parameter.
- To determine which one, move/delete the .material script exported with the model.
- If the insides are still out problem is with normals if not, depth write is the culprit.
- To solve depth_write problem ensure that your materials
don't have Transparancy set to True in Blender.
- To solve normal problem, ensure that in Blender your meshes don't have
'Double sided' parameter set to True (it defaults to true).
	- Now you should be able to see the problem in Blender
	- 'Recalculate normals outside' in Blender and check if it is solved

Defects:
- Hidden objects are not exported and the exporter will halt if you try to export them. If exporting whole scene do unhide on all layers before exporting.
(All UV mapped objects corcerned here have a uv image assigned to them, they are handled differently if they do not have an image.)
- Exporting UV mapped objects is painfully slow.
- Exported UV mapped objects have extra vertices (around 4 times the original),
some of these are needed because of islands in the UV maps but most are not.
- Using EDGE_SPLIT modifier on UV mapped objects does not produce sharp edges
	- This is a design flaw in how the exporter handles UV mapped objects.
- Export settings (in the export window) do not persist after closing the export window.
- Skeletons are not supported
- Multiple texture coordinates are not supported
- Only texture type supported is image

