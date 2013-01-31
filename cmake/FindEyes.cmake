# - Locate EYES library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# EYES_INCLUDE_DIR, where to find headers.
# EYES_LIBRARIES, all the EYES libraries and dependencies,
#	both debug and optimized
# EYES_FOUND, if false, do not try to link to EYES
#
# EYES_LIBRARY, the base library, both optimized and debug
# EYES_LIBRARY_RELEASE, the base library, optimized
# EYES_LIBRARY_DEBUG, the base library, debug
#

if( EYES_LIBRARY AND EYES_INCLUDE_DIR )
	# in cache already
	set( EYES_FIND_QUIETLY TRUE )
endif( EYES_LIBRARY AND EYES_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{EYES_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{EYES_DIR}/lib
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

find_path( EYES_INCLUDE_DIR
	NAMES eyes.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${EYES_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "EYES library" )
lib_finder("EYES" EYES)

set( EYES_LIBRARIES ${EYES_LIBRARY})

if( EYES_LIBRARIES AND EYES_INCLUDE_DIR )
	set( EYES_FOUND "YES" )
	if( NOT EYES_FIND_QUIETLY )
		message( STATUS "Found EYES: ${EYES_LIBRARIES}" )
	endif( NOT EYES_FIND_QUIETLY )
else()
	if( NOT EYES_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find EYES!" )
	endif( NOT EYES_FIND_QUIETLY )
endif()

