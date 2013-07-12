# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fi
#
# Finds Ogre library Module
#
# This module defines:
#  Ogre_INCLUDE_DIR	- where to find ogg.h
#  Ogre_LIBRARY		- Ogre library both debug and optimized versions.
#  Ogre_LIBRARY_RELEASE	- Ogre library, optimized version.
#  Ogre_LIBRARY_DEBUG	- Ogre library, debug version.
#  Ogre_BINARY_RELEASE	- Ogre dll
#  Ogre_FOUND		- True if Ogre found.
# TODO should define OGRE_LIBRARIES containing all of the Ogre libs

if( Ogre_INCLUDE_DIR AND Ogre_LIBRARY )
	# Already in cache, be silent
	set(Ogre_FIND_QUIETLY TRUE)
endif( Ogre_INCLUDE_DIR AND Ogre_LIBRARY )

find_path(Ogre_INCLUDE_DIR OGRE/Ogre.h
	$ENV{Ogre_DIR}/include
	/opt/local/include
	/usr/local/include
	/usr/include
	)

# Common library search paths for both debug and release versions
set( LIBRARY_SEARCH_PATHS
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	$ENV{Ogre_DIR}/lib
	${Ogre_INCLUDE_DIR}/../lib
	)

set( Ogre_NAMES_RELEASE OgreMain libOgreMain )
find_library(Ogre_LIBRARY_RELEASE
	NAMES ${Ogre_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Ogre_INCLUDE_DIR}/../lib/release
	)

set( Ogre_NAMES_DEBUG OgreMain_d libOgreMain_d )
find_library(Ogre_LIBRARY_DEBUG
	NAMES ${Ogre_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Ogre_INCLUDE_DIR}/../lib/debug
	)

# TODO this only works on Windows
find_file(Ogre_BINARY_RELEASE
	NAMES OgreMain.dll
	PATHS
	${Ogre_INCLUDE_DIR}/../bin/release
	NO_DEFAULT_PATH
	)

if( Ogre_LIBRARY_DEBUG AND Ogre_LIBRARY_RELEASE )
	set( Ogre_LIBRARY debug ${Ogre_LIBRARY_DEBUG}
		optimized ${Ogre_LIBRARY_RELEASE}
		CACHE FILEPATH "Ogre Main library" FORCE )
# Workaround for Linux lacking debug builds, also allows us to only build the
# release version if we want
elseif( Ogre_LIBRARY_RELEASE )
	set( Ogre_LIBRARY ${Ogre_LIBRARY_RELEASE} CACHE FILEPATH "Ogre Main library" FORCE )
elseif( Ogre_LIBRARY_DEBUG )
	set( Ogre_LIBRARY ${Ogre_LIBRARY_DEBUG} CACHE FILEPATH "Ogre Main library" FORCE )
endif()

if( Ogre_INCLUDE_DIR AND Ogre_LIBRARY )
	set(Ogre_FOUND TRUE)
else( Ogre_INCLUDE_DIR AND Ogre_LIBRARY )
	set(Ogre_FOUND FALSE)
endif( Ogre_INCLUDE_DIR AND Ogre_LIBRARY )

if( Ogre_FOUND )
	if (NOT Ogre_FIND_QUIETLY)
		message(STATUS "Found Ogre: ${Ogre_LIBRARY}")
	endif (NOT Ogre_FIND_QUIETLY)
else( Ogre_FOUND )
	if (Ogre_FIND_REQUIRED)
		message(STATUS "Looked for Ogre libraries named ${Ogre_NAMES_RELEASE} and ${Ogre_NAMES_DEBUG}.")
		message(STATUS "Include file detected: [${Ogre_INCLUDE_DIR}].")
		message(STATUS "Lib file detected: [${Ogre_LIBRARY}].")
	endif (Ogre_FIND_REQUIRED)
endif( Ogre_FOUND )

if( Ogre_FOUND OR Ogre_FIND_QUIETLY )
	mark_as_advanced(
		Ogre_INCLUDE_DIR
		Ogre_LIBRARY
		Ogre_LIBRARY_RELEASE
		Ogre_LIBRARY_DEBUG
		)
endif()

