Maintainer: Joonatan Kuosa <joonatan.kuosa@tut.fi>

Released: 2011-04-18
Version: 0.3.1

Blender version: only 2.57 release is supported
Older versions: 2.4* and 2.5* betas will not work because of API changes.

Installation:
- Copy all files (README not necessary) to Blender addons directory
${BLENDER_INSTALL_DIR}/2.57/scripts/addons
Or the application data directory in USER/Blender/2.57/scripts/addons
- Start Blender
- Go to User Preferences (Editor Window)
	- Addons
	- Import/Export
	- Find OGRE Exporter
	- Enable addon

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

Defects:
- Do not disable the addon after enabling it, does not work correctly
- Seems like the addon is not persistant
so if you close Blender you need to enable the addon again next time.

