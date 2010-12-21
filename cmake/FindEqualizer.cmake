# - Locate Equalizer library
# This module defines
#  Equalizer_LIBRARY, the library to link against
#  Equalizer_FOUND, if false, do not try to link to Equalizer
#  Equalizer_INCLUDE_DIR, where to find headers.
# TODO add Equalizer_Admin_LIBRARY
# TODO add Equalizer_Server_LIBRARY
# TODO add Equalizer_LIBRARIES

IF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIR)
	# in cache already
	SET(Equalizer_FIND_QUIETLY TRUE)
ENDIF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIR)


FIND_PATH(Equalizer_INCLUDE_DIR
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

set( Equalizer_NAMES_RELEASE Equalizer )
FIND_LIBRARY(Equalizer_LIBRARY_RELEASE
	NAMES ${Equalizer_NAMES_RELEASE}
	PATHS $ENV{Equalizer_DIR}/lib /usr/lib /usr/local/lib
	)

set( Equalizer_NAMES_DEBUG Equalizer )
FIND_LIBRARY(Equalizer_LIBRARY_DEBUG
	NAMES ${Equalizer_NAMES_DEBUG}
	PATHS $ENV{Equalizer_DIR}/lib /usr/lib /usr/local/lib
	)
if( Equalizer_LIBRARY_DEBUG AND NOT Equalizer_LIBRARY_RELEASE )
	set( Equalizer_LIBRARY_RELEASE ${Equalizer_LIBRARY_DEBUG} )
endif()

set( Equalizer_LIBRARY optimized ${Equalizer_LIBRARY_RELEASE}
	debug ${Equalizer_LIBRARY_DEBUG} )


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

