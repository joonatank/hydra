#-------------------------------------------------------------------
# This file is part of the CMake build system for eqOgre
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------
# TODO this should be tested on Windows
# FIXME will not work on Windows because Boost library will have both
# optimized and debug in it in Windows

# For now installing dependencies is only supported on Windows
# Shouldn't be a problem
if (NOT WIN32)
	return()
endif()

option(EQOGRE_INSTALL_DEPENDENCIES "Install dependency libs" TRUE)

# Copy Release versions of libs
install( FILES
	${Equalizer_LIBRARY}
	# These doesn't work on Linux
	${OGRE_BINARY_REL}
	${OGRE_RenderSystem_GL_BINARY_REL}
	${VRPN_LIBRARY}
	# This doesn't work on Linux
 	${OIS_BINARY_REL}
 	${PYTHON_LIBRARIES}
	${Boost_PYTHON_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	${CAUDIO_LIBRARY}
	DESTINATION
	./ )
