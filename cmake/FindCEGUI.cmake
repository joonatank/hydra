# - Locate CEGUI library
# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2011-01
# This file is part of eqOgre build system.
# This file is in public domain, feel free to use it as you see fit
#
# TODO add parameters
# CEGUI_USE_OGRE_RENDERER
# CEGUI_USE_OPENGL_RENDERER
# Later: (these needs additions to CEGUI build system)
# CEGUI_USE_EXPAT_PARSER
# CEGUI_USE_LIBXML_PARSER
# CEGUI_USE_SILLY_CODEC
# CEGUI_USE_DEVIL_CODEC
#
# This module defines
#
# CEGUI_INCLUDE_DIR, where to find headers.
# CEGUI_LIBRARIES, all the CEGUI libraries and dependencies,
#	both debug and optimized
# CEGUI_FOUND, if false, do not try to link to CEGUI
#
# CEGUI_BASE_LIBRARY, the base library, both optimized and debug
# CEGUI_BASE_LIBRARY_RELEASE, the base library, optimized
# CEGUI_BASE_LIBRARY_DEBUG, the base library, debug
#
# CEGUI_FALAGARD_LIBRARY, the Falagard style library, both optimized and debug
# CEGUI_FALAGARD_LIBRARY_RELEASE, the Falagard style library, optimized
# CEGUI_FALAGARD_LIBRARY_DEBUG, the Falagard style library, debug
#
# CEGUI_IMAGE_LIBRARY, the default image library, both optimized and debug
# CEGUI_IMAGE_LIBRARY_RELEASE, the default image library, optimized
# CEGUI_IMAGE_LIBRARY_DEBUG, the default image library, debug
#
# CEGUI_PARSER_LIBRARY, the default parser library, both optimized and debug
# CEGUI_PARSER_LIBRARY_RELEASE, the default parser library, optimized
# CEGUI_PARSER_LIBRARY_DEBUG, the default parser library, debug
#
# CEGUI_RENDER_LIBRARY, the chosen Renderer library, both optimized and debug
# CEGUI_RENDER_LIBRARY_RELEASE, the chosen Renderer library, optimized
# CEGUI_RENDER_LIBRARY_DEBUG, the chosen Renderer library, debug
#
# TODO add CEGUI_VERSION support
# TODO add optimized and release versions
# TODO should be tested on Windows
# TODO before testing the CEGUI lib should be built on windows
# TODO this needs the addition of CMake build system for CEGUI
   
if( CEGUI_LIBRARY AND CEGUI_INCLUDE_DIR )
	# in cache already
	set( CEGUI_FIND_QUIETLY TRUE )
endif( CEGUI_LIBRARY AND CEGUI_INCLUDE_DIR )

set( INCLUDE_PATHS
	$ENV{CEGUI_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{CEGUI_DIR}/lib
	/usr/local/lib
	/usr/lib
	/usr/local/X11R6/lib
	/usr/X11R6/lib
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
	/usr/freeware/lib64
	)

find_path( CEGUI_INCLUDE_DIR
	NAMES CEGUI/CEGUI.h
	PATHS ${INCLUDE_PATHS}
	#PATH_SUFFIXES cegui CEGUI
	)

find_library( CEGUI_BASE_LIBRARY_RELEASE
	NAMES CEGUIBase
	PATHS ${LIBRARY_PATHS}
	${CEGUI_INCLUDE_DIR}/../lib/Release
	)

find_library( CEGUI_BASE_LIBRARY_DEBUG
	NAMES CEGUIBase
	PATHS ${LIBRARY_PATHS}
	${CEGUI_INCLUDE_DIR}/../lib/Debug
	)

if( CEGUI_LIBRARY AND CEGUI_INCLUDE_DIR )
	set( CEGUI_FOUND "YES" )
	if( NOT CEGUI_FIND_QUIETLY )
		message( STATUS "Found CEGUI: ${CEGUI_LIBRARY}" )
	endif( NOT CEGUI_FIND_QUIETLY )
else()
	if( NOT CEGUI_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find CEGUI!" )
	endif( NOT CEGUI_FIND_QUIETLY )
endif()

