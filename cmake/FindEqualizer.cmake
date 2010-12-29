# - Locate Equalizer library
# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of eqOgre build system.
# This file is in public domain, feel free to use it as you see fit.

# This module defines
#  Equalizer_LIBRARY_RELEASE, the library to link against, optimized version.
#  Equalizer_LIBRARY_DEBUG, the library to link against, bebug version.
#  Equalizer_LIBRARY, the library to link against, both debug and optimized versions.
#  Equalizer_FOUND, if false, do not try to link to Equalizer
#  Equalizer_INCLUDE_DIR, where to find headers.
# TODO add Equalizer_Admin_LIBRARY
# TODO add Equalizer_Server_LIBRARY
# TODO add Equalizer_LIBRARIES

if( Equalizer_INCLUDE_DIR )
	# in cache already
	set( Equalizer_FIND_QUIETLY TRUE )
endif( Equalizer_INCLUDE_DIR )


FIND_PATH( Equalizer_INCLUDE_DIR
	eq/eq.h
	PATHS
	$ENV{Equalizer_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	PATH_SUFFIXES
	)

set( LIBRARY_SEARCH_PATHS
	$ENV{Equalizer_DIR}/lib
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	/opt/lib
	${Equalizer_INCLUDE_DIR}/../lib
	)

set( Equalizer_NAMES_RELEASE Equalizer )
FIND_LIBRARY(Equalizer_LIBRARY_RELEASE
	NAMES ${Equalizer_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Equalizer_INCLUDE_DIR}/../lib/release
	)

set( Equalizer_NAMES_DEBUG Equalizer Equalizer_d )
FIND_LIBRARY(Equalizer_LIBRARY_DEBUG
	NAMES ${Equalizer_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Equalizer_INCLUDE_DIR}/../lib/debug
	)

if( Equalizer_LIBRARY_DEBUG AND NOT Equalizer_LIBRARY_RELEASE )
	set( Equalizer_LIBRARY_RELEASE ${Equalizer_LIBRARY_DEBUG} )
endif()

if( Equalizer_LIBRARY_DEBUG AND Equalizer_LIBRARY_RELEASE )
	set( Equalizer_LIBRARY optimized ${Equalizer_LIBRARY_RELEASE}
		debug ${Equalizer_LIBRARY_DEBUG}
		CACHE FILEPATH "Equalizer library" FORCE )
else()
	#	set( Equalizer_LIBRARY CACHE FILEPATH "Equalizer library" FORCE )
endif()


IF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIR)
	SET(Equalizer_FOUND "YES")
	IF(NOT Equalizer_FIND_QUIETLY)
		MESSAGE(STATUS "Found Equalizer: ${Equalizer_LIBRARY}")
	ENDIF(NOT Equalizer_FIND_QUIETLY)
ELSE(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIR)
	IF(NOT Equalizer_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find Equalizer!")
	ENDIF(NOT Equalizer_FIND_QUIETLY)
ENDIF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIR)

if( Equalizer_FIND_QUIETLY OR Equalizer_FOUND )
	mark_as_advanced(
		Equalizer_INCLUDE_DIR
		Equalizer_LIBRARY
		Equalizer_LIBRARY_RELEASE
		Equalizer_LIBRARY_DEBUG
		)
endif()

