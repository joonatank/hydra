# - Locate OVR library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# OVR_INCLUDE_DIR, where to find headers.
# OVR_LIBRARIES, all the Oculus VR libraries and dependencies,
#	both debug and optimized
# OVR_FOUND, if false, do not try to link to OVR
#
# OVR_LIBRARY, the base library, both optimized and debug
# OVR_LIBRARY_RELEASE, the base library, optimized
# OVR_LIBRARY_DEBUG, the base library, debug
#

if( OVR_LIBRARY AND OVR_INCLUDE_DIR )
	# in cache already
	set( OVR_FIND_QUIETLY TRUE )
endif( OVR_LIBRARY AND OVR_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{OVR_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{OVR_DIR}/lib
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

find_path( OVR_INCLUDE_DIR
	NAMES OVR.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${OVR_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "OVR library" )
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	lib_finder("OVR" "libovr64")
else()
	lib_finder("OVR" "libovr")
endif()

if(WIN32)
	set( OVR_LIBRARIES ${OVR_LIBRARY} winmm.lib)
else()
	set( OVR_LIBRARIES ${OVR_LIBRARY})
endif()

if( OVR_LIBRARIES AND OVR_INCLUDE_DIR )
	set( OVR_FOUND "YES" )
	if( NOT OVR_FIND_QUIETLY )
		message( STATUS "Found OVR: ${OVR_LIBRARIES}" )
	endif( NOT OVR_FIND_QUIETLY )
else()
	if( NOT OVR_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find OVR!" )
	endif( NOT OVR_FIND_QUIETLY )
endif()

