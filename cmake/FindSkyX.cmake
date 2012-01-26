# - Locate SkyX library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# SkyX_INCLUDE_DIR, where to find headers.
# SkyX_LIBRARIES, all the SkyX libraries and dependencies,
#	both debug and optimized
# SkyX_FOUND, if false, do not try to link to SkyX
#
# SkyX_LIBRARY, the base library, both optimized and debug
# SkyX_LIBRARY_RELEASE, the base library, optimized
# SkyX_LIBRARY_DEBUG, the base library, debug
#

if( SkyX_LIBRARY AND SkyX_INCLUDE_DIR )
	# in cache already
	set( SkyX_FIND_QUIETLY TRUE )
endif( SkyX_LIBRARY AND SkyX_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{SkyX_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{SkyX_DIR}/lib
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

find_path( SkyX_INCLUDE_DIR
	NAMES SkyX.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${SkyX_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "SkyX library" )
lib_finder("SkyX" SkyX)

set( SkyX_LIBRARIES ${SkyX_LIBRARY})

if( SkyX_LIBRARIES AND SkyX_INCLUDE_DIR )
	set( SkyX_FOUND "YES" )
	if( NOT SkyX_FIND_QUIETLY )
		message( STATUS "Found SkyX: ${SkyX_LIBRARIES}" )
	endif( NOT SkyX_FIND_QUIETLY )
else()
	if( NOT SkyX_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find SkyX!" )
	endif( NOT SkyX_FIND_QUIETLY )
endif()

