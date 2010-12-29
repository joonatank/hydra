# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of eqOgre build system.
# This file is in public domain, feel free to use it as you see fi
#
# Finds Ogg library
#
#  Ogg_INCLUDE_DIR	- where to find ogg.h
#  Ogg_LIBRARY		- Ogg library both debug and optimized versions.
#  Ogg_LIBRARY_RELEASE	- Ogg library, optimized version.
#  Ogg_LIBRARY_DEBUG	- Ogg library, debug version.
#  Ogg_FOUND		- True if Ogg found.
#

if( Ogg_INCLUDE_DIR AND Ogg_LIBRARY )
	# Already in cache, be silent
	set(Ogg_FIND_QUIETLY TRUE)
endif( Ogg_INCLUDE_DIR AND Ogg_LIBRARY )

find_path(Ogg_INCLUDE_DIR ogg/ogg.h
	$ENV{Ogg_HOME}/include
	/opt/local/include
	/usr/local/include
	/usr/include
	)

# Common library search paths for both debug and release versions
set( LIBRARY_SEARCH_PATHS 
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	$ENV{Ogg_HOME}/lib
	$ENV{Ogg_HOME}
	${Ogg_INCLUDE_DIR}/../lib
	)

set( Ogg_NAMES_RELEASE ogg libogg libogg_static )
find_library(Ogg_LIBRARY_RELEASE
	NAMES ${Ogg_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Ogg_INCLUDE_DIR}/../lib/release
	)

set( Ogg_NAMES_DEBUG ogg_d libogg_d libogg_static_d )
find_library(Ogg_LIBRARY_DEBUG
	NAMES ${Ogg_NAMES_DEBUG}
	PATHS 
	${LIBRARY_SEARCH_PATHS}
	${Ogg_INCLUDE_DIR}/../lib/debug
	)

if( Ogg_LIBRARY_DEBUG AND NOT Ogg_LIBRARY_RELEASE )
	set( Ogg_LIBRARY_RELEASE ${Ogg_LIBRARY_DEBUG} )
endif()

if( Ogg_LIBRARY_DEBUG AND Ogg_LIBRARY_RELEASE )
	set( Ogg_LIBRARY debug ${Ogg_LIBRARY_DEBUG}
		optimized ${Ogg_LIBRARY_RELEASE} 
		CACHE FILEPATH "Ogg library" FORCE )
else()
	#	set( Ogg_LIBRARY CACHE FILEPATH "Ogg library" FORCE )
endif()

if( Ogg_INCLUDE_DIR AND Ogg_LIBRARY )
	set(Ogg_FOUND TRUE)
else( Ogg_INCLUDE_DIR AND Ogg_LIBRARY )
	set(Ogg_FOUND FALSE)
endif( Ogg_INCLUDE_DIR AND Ogg_LIBRARY )

if( Ogg_FOUND )
	if (NOT Ogg_FIND_QUIETLY)
		message(STATUS "Found Ogg: ${Ogg_LIBRARY}")
	endif (NOT Ogg_FIND_QUIETLY)
else( Ogg_FOUND )
	if (Ogg_FIND_REQUIRED)
		message(STATUS "Looked for Ogg libraries named ${Ogg_NAMES_RELEASE} and ${Ogg_NAMES_DEBUG}.")
		message(STATUS "Include file detected: [${Ogg_INCLUDE_DIR}].")
		message(STATUS "Lib file detected: [${Ogg_LIBRARY}].")
		#message(FATAL_ERROR "=========> Could NOT find Ogg library")
	endif (Ogg_FIND_REQUIRED)
endif( Ogg_FOUND )

if( Ogg_FOUND OR Ogg_FIND_QUIETLY )
	mark_as_advanced(
		Ogg_LIBRARY
		Ogg_LIBRARY_RELEASE
		Ogg_LIBRARY_DEBUG
		Ogg_INCLUDE_DIR
		)
endif()

