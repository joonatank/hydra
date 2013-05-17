# - Locate PCANBasic library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# PCANBasic_INCLUDE_DIR, where to find headers.
# PCANBasic_LIBRARIES, all the PCANBasic libraries and dependencies,
#	both debug and optimized
# PCANBasic_FOUND, if false, do not try to link to PCANBasic
#
# PCANBasic_LIBRARY, the base library, both optimized and debug
# PCANBasic_LIBRARY_RELEASE, the base library, optimized
# PCANBasic_LIBRARY_DEBUG, the base library, debug
#

if( PCANBasic_LIBRARY AND PCANBasic_INCLUDE_DIR )
	# in cache already
	set( PCANBasic_FIND_QUIETLY TRUE )
endif( PCANBasic_LIBRARY AND PCANBasic_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{PCANBasic_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{PCANBasic_DIR}/lib
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

find_path( PCANBasic_INCLUDE_DIR
	NAMES PCANBasic.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${PCANBasic_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "PCANBasic library" )
lib_finder("PCANBasic" PCANBasic)

set( PCANBasic_LIBRARIES ${PCANBasic_LIBRARY})

if( PCANBasic_LIBRARIES AND PCANBasic_INCLUDE_DIR )
	set( PCANBasic_FOUND "YES" )
	if( NOT PCANBasic_FIND_QUIETLY )
		message( STATUS "Found PCANBasic: ${PCANBasic_LIBRARIES}" )
	endif( NOT PCANBasic_FIND_QUIETLY )
else()
	if( NOT PCANBasic_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find PCANBasic!" )
	endif( NOT PCANBasic_FIND_QUIETLY )
endif()

