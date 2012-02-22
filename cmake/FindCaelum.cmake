# - Locate Caelum library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# Caelum_INCLUDE_DIR, where to find headers.
# Caelum_LIBRARIES, all the Caelum libraries and dependencies,
#	both debug and optimized
# Caelum_FOUND, if false, do not try to link to Caelum
#
# Caelum_LIBRARY, the base library, both optimized and debug
# Caelum_LIBRARY_RELEASE, the base library, optimized
# Caelum_LIBRARY_DEBUG, the base library, debug
#

if( Caelum_LIBRARY AND Caelum_INCLUDE_DIR )
	# in cache already
	set( Caelum_FIND_QUIETLY TRUE )
endif( Caelum_LIBRARY AND Caelum_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set( INCLUDE_PATHS
	$ENV{Caelum_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	)

set( LIBRARY_PATHS
	$ENV{Caelum_DIR}/lib
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

find_path( Caelum_INCLUDE_DIR
	NAMES Caelum.h
	PATHS ${INCLUDE_PATHS}
	)

set( INCLUDE_DIR ${Caelum_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "Caelum library" )
lib_finder("Caelum" Caelum)

set( Caelum_LIBRARIES ${Caelum_LIBRARY})

if( Caelum_LIBRARIES AND Caelum_INCLUDE_DIR )
	set( Caelum_FOUND "YES" )
	if( NOT Caelum_FIND_QUIETLY )
		message( STATUS "Found Caelum: ${Caelum_LIBRARIES}" )
	endif( NOT Caelum_FIND_QUIETLY )
else()
	if( NOT Caelum_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find Caelum!" )
	endif( NOT Caelum_FIND_QUIETLY )
endif()

