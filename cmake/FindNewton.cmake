#-------------------------------------------------------------------
# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2011-02
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------
#
# Finds Newton library
#
#  Newton_INCLUDE_DIR	- where to find ogg.h
#  Newton_LIBRARY		- Newton library both debug and optimized versions.
#  Newton_LIBRARY_RELEASE	- Newton library, optimized version.
#  Newton_LIBRARY_DEBUG	- Newton library, debug version.
#  Newton_FOUND		- True if Newton found.
#

if( Newton_INCLUDE_DIR AND Newton_LIBRARY )
	# Already in cache, be silent
	set(Newton_FIND_QUIETLY TRUE)
endif( Newton_INCLUDE_DIR AND Newton_LIBRARY )

find_path(Newton_INCLUDE_DIR Newton.h
	$ENV{Newton_HOME}/include
	/opt/local/include
	/usr/local/include
	/usr/include
	)

# Common library search paths for both debug and release versions
set( LIBRARY_SEARCH_PATHS
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	$ENV{Newton_HOME}/lib
	$ENV{Newton_HOME}
	${Newton_INCLUDE_DIR}/../lib
	)

set(Newton_NAMES_RELEASE newton)
find_library(Newton_LIBRARY_RELEASE
	NAMES ${Newton_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Newton_INCLUDE_DIR}/../lib/release
	)

set(Newton_NAMES_DEBUG newton_d) 
find_library(Newton_LIBRARY_DEBUG
	NAMES ${Newton_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Newton_INCLUDE_DIR}/../lib/debug
	)

if( Newton_LIBRARY_DEBUG AND Newton_LIBRARY_RELEASE )
	set( Newton_LIBRARY debug ${Newton_LIBRARY_DEBUG}
		optimized ${Newton_LIBRARY_RELEASE}
		CACHE FILEPATH "Newton library" FORCE )
elseif( Newton_LIBRARY_RELEASE )
	set( Newton_LIBRARY ${Newton_LIBRARY_RELEASE} CACHE FILEPATH "Newton library" FORCE )
elseif( Newton_LIBRARY_DEBUG )
	set( Newton_LIBRARY ${Newton_LIBRARY_DEBUG} CACHE FILEPATH "Newton library" FORCE )
else()
	#	set( Newton_LIBRARY CACHE FILEPATH "Newton library" FORCE )
endif()

if( Newton_INCLUDE_DIR AND Newton_LIBRARY )
	set(Newton_FOUND TRUE)
else( Newton_INCLUDE_DIR AND Newton_LIBRARY )
	set(Newton_FOUND FALSE)
endif( Newton_INCLUDE_DIR AND Newton_LIBRARY )

if( Newton_FOUND )
	if (NOT Newton_FIND_QUIETLY)
		message(STATUS "Found Newton: ${Newton_LIBRARY}")
	endif (NOT Newton_FIND_QUIETLY)
else( Newton_FOUND )
	if (Newton_FIND_REQUIRED)
		message(STATUS "Looked for Newton libraries named ${Newton_NAMES_RELEASE} and ${Newton_NAMES_DEBUG}.")
		message(STATUS "Include file detected: [${Newton_INCLUDE_DIR}].")
		message(STATUS "Lib file detected: [${Newton_LIBRARY}].")
		#message(FATAL_ERROR "=========> Could NOT find Newton library")
	endif (Newton_FIND_REQUIRED)
endif( Newton_FOUND )

if( Newton_FOUND OR Newton_FIND_QUIETLY )
	mark_as_advanced(
		Newton_LIBRARY
		Newton_LIBRARY_RELEASE
		Newton_LIBRARY_DEBUG
		Newton_INCLUDE_DIR
		)
endif()

