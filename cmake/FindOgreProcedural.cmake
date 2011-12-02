# - Locate OgreProcedural library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# OgreProcedural_INCLUDE_DIR, where to find headers.
# OgreProcedural_LIBRARIES, all the OgreProcedural libraries and dependencies,
#	both debug and optimized
# OgreProcedural_FOUND, if false, do not try to link to OgreProcedural
#
# OgreProcedural_LIBRARY, the base library, both optimized and debug
# OgreProcedural_LIBRARY_RELEASE, the base library, optimized
# OgreProcedural_LIBRARY_DEBUG, the base library, debug
#

if( OgreProcedural_LIBRARY AND OgreProcedural_INCLUDE_DIR )
	# in cache already
	set( OgreProcedural_FIND_QUIETLY TRUE )
endif( OgreProcedural_LIBRARY AND OgreProcedural_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{OgreProcedural_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{OgreProcedural_DIR}/lib
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

find_path( OgreProcedural_INCLUDE_DIR
	NAMES Procedural.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${OgreProcedural_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "OgreProcedural library" )
lib_finder("OgreProcedural" OgreProcedural)

set( OgreProcedural_LIBRARIES ${OgreProcedural_LIBRARY})

if( OgreProcedural_LIBRARIES AND OgreProcedural_INCLUDE_DIR )
	set( OgreProcedural_FOUND "YES" )
	if( NOT OgreProcedural_FIND_QUIETLY )
		message( STATUS "Found OgreProcedural: ${OgreProcedural_LIBRARIES}" )
	endif( NOT OgreProcedural_FIND_QUIETLY )
else()
	if( NOT OgreProcedural_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find OgreProcedural!" )
	endif( NOT OgreProcedural_FIND_QUIETLY )
endif()

