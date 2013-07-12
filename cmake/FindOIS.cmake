# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fi
#
# Finds OIS library
#
#  OIS_INCLUDE_DIR	- where to find ogg.h
#  OIS_LIBRARY		- OIS library both debug and optimized versions.
#  OIS_LIBRARY_RELEASE	- OIS library, optimized version.
#  OIS_LIBRARY_DEBUG	- OIS library, debug version.
#  OIS_FOUND		- True if OIS found.
#

if( OIS_INCLUDE_DIR )
	# Already in cache, be silent
	set(OIS_FIND_QUIETLY TRUE)
endif( OIS_INCLUDE_DIR )

find_path(OIS_INCLUDE_DIR ois/ois.h
	$ENV{OIS_DIR}/include
	/opt/local/include
	/usr/local/include
	/usr/include
	)

# Common library search paths for both debug and release versions
set( LIBRARY_SEARCH_PATHS
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	$ENV{OIS_DIR}/lib
	${OIS_INCLUDE_DIR}/../lib
	)

set( OIS_NAMES_RELEASE OIS libOIS )
find_library(OIS_LIBRARY_RELEASE
	NAMES ${OIS_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${OIS_INCLUDE_DIR}/../lib/release
	)

set( OIS_NAMES_DEBUG OIS_d libOIS_d )
find_library(OIS_LIBRARY_DEBUG
	NAMES ${OIS_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${OIS_INCLUDE_DIR}/../lib/debug
	)

# if( OIS_LIBRARY_DEBUG AND NOT OIS_LIBRARY_RELEASE )
# 	set( OIS_LIBRARY_RELEASE ${OIS_LIBRARY_DEBUG} )
# endif()

if( OIS_LIBRARY_DEBUG AND OIS_LIBRARY_RELEASE )
	set( OIS_LIBRARY debug ${OIS_LIBRARY_DEBUG}
		optimized ${OIS_LIBRARY_RELEASE}
		CACHE FILEPATH "OIS library" FORCE )
elseif( OIS_LIBRARY_RELEASE )
	set( OIS_LIBRARY ${OIS_LIBRARY_RELEASE} CACHE FILEPATH "OIS library" FORCE )
elseif( OIS_LIBRARY_DEBUG )
	set( OIS_LIBRARY ${OIS_LIBRARY_DEBUG} CACHE FILEPATH "OIS library" FORCE )
else()
	#set( OIS_LIBRARY CACHE FILEPATH "OIS library" FORCE )
endif()

if( OIS_INCLUDE_DIR AND OIS_LIBRARY )
	set(OIS_FOUND TRUE)
else( OIS_INCLUDE_DIR AND OIS_LIBRARY )
	set(OIS_FOUND FALSE)
endif( OIS_INCLUDE_DIR AND OIS_LIBRARY )

if( OIS_FOUND )
	if (NOT OIS_FIND_QUIETLY)
		message(STATUS "Found OIS: ${OIS_LIBRARY}")
	endif (NOT OIS_FIND_QUIETLY)
else( OIS_FOUND )
	if( OIS_FIND_REQUIRED )
		message(STATUS "Looked for OIS libraries named ${OIS_NAMES_RELEASE} and ${OIS_NAMES_DEBUG}.")
		message(STATUS "Include file detected: [${OIS_INCLUDE_DIR}].")
		message(STATUS "Lib file detected: [${OIS_LIBRARY}].")
	endif( OIS_FIND_REQUIRED )
endif( OIS_FOUND )

if( OIS_FOUND OR OIS_FIND_QUIETLY )
	mark_as_advanced(
		OIS_LIBRARY
		OIS_LIBRARY_RELEASE
		OIS_LIBRARY_DEBUG
		OIS_INCLUDE_DIR
		)
endif()

