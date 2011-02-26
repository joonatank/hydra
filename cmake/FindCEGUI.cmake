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

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

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

set( INCLUDE_DIR ${CEGUI_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "CEGUI Base library" )
lib_finder( "CEGUI_BASE" CEGUIBase )

message( "${LIB_HELP_STRING} = ${CEGUI_BASE_LIBRARY}" )

# Find the Falagar library
set( LIB_HELP_STRING "CEGUI Falagard library" )
lib_finder( "CEGUI_FALAGARD" CEGUIFalagardWRBase )

message( "${LIB_HELP_STRING} = ${CEGUI_FALAGARD_LIBRARY}" )

# Find the Renderer library, only Ogre supported at the moment
set( LIB_HELP_STRING "CEGUI Ogre Rendering library" )
lib_finder( "CEGUI_OGRE_RENDERER" CEGUIOgreRenderer )

message( "${LIB_HELP_STRING} = ${CEGUI_OGRE_RENDERER_LIBRARY}" )

# Find the XML parser library, only RapidXML supported at the moment
set( LIB_HELP_STRING "CEGUI RapidXML parser library" )
lib_finder( "CEGUI_RAPIDXML_PARSER" CEGUIRapidXMLParser )

message( "${LIB_HELP_STRING} = ${CEGUI_RAPIDXML_PARSER_LIBRARY}" )

# Find the Image Codec library, only FreeImage supported at the moment
set( LIB_HELP_STRING "CEGUI FreeImage Image Codec library" )
lib_finder( "CEGUI_FREEIMAGE_IMAGE_CODEC" CEGUIFreeImageImageCodec )

message( "${LIB_HELP_STRING} = ${CEGUI_FREEIMAGE_IMAGE_CODEC_LIBRARY}" )

set( CEGUI_LIBRARIES
	${CEGUI_BASE_LIBRARY}
	${CEGUI_FALAGARD_LIBRARY}
	${CEGUI_OGRE_RENDERER_LIBRARY}
	${CEGUI_RAPIDXML_PARSER_LIBRARY}
	${CEGUI_FREEIMAGE_IMAGE_CODEC_LIBRARY}
	)

if( CEGUI_LIBRARIES AND CEGUI_INCLUDE_DIR )
	set( CEGUI_FOUND "YES" )
	if( NOT CEGUI_FIND_QUIETLY )
		message( STATUS "Found CEGUI: ${CEGUI_LIBRARIES}" )
	endif( NOT CEGUI_FIND_QUIETLY )
else()
	if( NOT CEGUI_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find CEGUI!" )
	endif( NOT CEGUI_FIND_QUIETLY )
endif()

