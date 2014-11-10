# - Locate Sixsense library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2014-11
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# SIXSENSE_INCLUDE_DIR, where to find headers.
# SIXSENSE_LIBRARIES, all the Oculus VR libraries and dependencies,
#	both debug and optimized
# SIXSENSE_FOUND, if false, do not try to link to OVR
#
# SIXSENSE_LIBRARY, the base library, both optimized and debug
# SIXSENSE_LIBRARY_RELEASE, the base library, optimized
# SIXSENSE_LIBRARY_DEBUG, the base library, debug
#

if( SIXSENSE_LIBRARY AND SIXSENSE_INCLUDE_DIR )
	# in cache already
	set( SIXSENSE_FIND_QUIETLY TRUE )
endif( SIXSENSE_LIBRARY AND SIXSENSE_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{SIXSENSE_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{SIXSENSE_DIR}/lib
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

find_path( SIXSENSE_INCLUDE_DIR
	NAMES OVR.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${SIXSENSE_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "Sixsense library" )
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	lib_finder("SIXSENSE" "sixense_s_x64")
else()
	lib_finder("SIXSENSE" "sixense_s")
endif()

set(SIXSENSE_LIBRARIES ${SIXSENSE_LIBRARY})

if( SIXSENSE_LIBRARIES AND SIXSENSE_INCLUDE_DIR )
	set( SIXSENSE_FOUND "YES" )
	if( NOT SIXSENSE_FIND_QUIETLY )
		message( STATUS "Found OVR: ${SIXSENSE_LIBRARIES}" )
	endif( NOT SIXSENSE_FIND_QUIETLY )
else()
	if( NOT SIXSENSE_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find OVR!" )
	endif( NOT SIXSENSE_FIND_QUIETLY )
endif()

