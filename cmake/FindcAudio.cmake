# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# - Locate cAudio library
#
# This module defines
#  cAudio_LIBRARY, the library to link against, both versions
#  cAudio_LIBRARY_RELEASE, the library to link against, optimized version
#  cAudio_LIBRARY_DEBUG, the library to link against, debug version
#  cAudio_FOUND, if false, do not try to link to cAudio
#  cAudio_INCLUDE_DIR, where to find headers.

if( cAudio_LIBRARY AND cAudio_INCLUDE_DIR )
	# in cache already
	SET(cAudio_FIND_QUIETLY TRUE)
endif( cAudio_LIBRARY AND cAudio_INCLUDE_DIR )


FIND_PATH(cAudio_INCLUDE_DIR
	cAudio/cAudio.h
	PATHS
	$ENV{CAUDIO_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	PATH_SUFFIXES
	)

set( LIBRARY_SEARCH_PATHS
	$ENV{cAudio_DIR}/lib
	/usr/local/lib
	/usr/lib
	/opt/local/lib
	/opt/lib
	${cAudio_INCLUDE_DIR}/../lib
	)

set( cAudio_NAMES_RELEASE cAudio libcAudio )
FIND_LIBRARY( cAudio_LIBRARY_RELEASE
	NAMES ${cAudio_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${cAudio_INCLUDE_DIR}/../lib/release
	)

set( cAudio_NAMES_DEBUG cAudio_d libcAudio_d )
FIND_LIBRARY( cAudio_LIBRARY_DEBUG
	NAMES ${cAudio_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${cAudio_INCLUDE_DIR}/../lib/debug
	)

# if( cAudio_LIBRARY_DEBUG AND NOT cAudio_LIBRARY_RELEASE )
# 	set( cAudio_LIBRARY_RELEASE ${cAudio_LIBRARY_DEBUG} )
# endif()

if( cAudio_LIBRARY_DEBUG AND cAudio_LIBRARY_RELEASE )
	set( cAudio_LIBRARY debug ${cAudio_LIBRARY_DEBUG}
		optimized ${cAudio_LIBRARY_RELEASE}
		CACHE FILEPATH "cAudio library" FORCE )
elseif( cAudio_LIBRARY_RELEASE )
	set( cAudio_LIBRARY ${cAudio_LIBRARY_RELEASE} CACHE FILEPATH "cAudio library" FORCE )
elseif( cAudio_LIBRARY_DEBUG )
	set( cAudio_LIBRARY ${cAudio_LIBRARY_DEBUG} CACHE FILEPATH "cAudio library" FORCE )
else()
	#	set( cAudio_LIBRARY CACHE FILEPATH "cAudio library" FORCE )
endif()

if( cAudio_LIBRARY AND cAudio_INCLUDE_DIR )
	set( cAudio_FOUND TRUE )
else(cAudio_LIBRARY AND cAudio_INCLUDE_DIR)
	set( cAudio_FOUND FALSE )
endif(cAudio_LIBRARY AND cAudio_INCLUDE_DIR)

# Status messages
if( cAudio_FOUND )
	if(NOT cAudio_FIND_QUIETLY)
		MESSAGE(STATUS "Found cAudio: ${cAudio_LIBRARY}")
	endif(NOT cAudio_FIND_QUIETLY)
	#set(cAudio_INCLUDE_DIR "${cAudio_INCLUDE_DIR};${CAUDIO_INCLUDE_DIR}/cAudio")
else( cAudio_FOUND )
	if(NOT cAudio_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find cAudio!")
	endif(NOT cAudio_FIND_QUIETLY)
endif( cAudio_FOUND )

# Mark the variables advanced if we can assume that the user does not need them
if( cAudio_FOUND OR cAudio_FIND_QUIETLY )
	mark_as_advanced(
		cAudio_INCLUDE_DIR
		cAudio_LIBRARY
		cAudio_LIBRARY_RELEASE
		cAudio_LIBRARY_DEBUG
		)
endif()

