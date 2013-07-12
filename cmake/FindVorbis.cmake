# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fi
#
# Finds Vorbis library
#
#  Vorbis_INCLUDE_DIR 	- where to find vorbisenc.h, etc.
#  Vorbis_LIBRARY   	- The vorbis library. Both debug and optimized.
#  Vorbis_LIBRARY_RELEASE - The vorbis library optimized.
#  Vorbis_LIBRARY_DEBUG   - The vorbis library debug.
#  Vorbis_FILE_LIBRARY	- The vorbis file library. Both debug and optimized.
#  Vorbis_FILE_LIBRARY_RELEASE	- The vorbis file library, optimized version.
#  Vorbis_FILE_LIBRARY_DEBUG	- The vorbis file library, debug version.
#  Vorbis_FOUND       	- True if Vorbis found.
#

if( Vorbis_INCLUDE_DIR )
	# Already in cache, be silent
	set(Vorbis_FIND_QUIETLY TRUE)
endif( Vorbis_INCLUDE_DIR )

find_path(Vorbis_INCLUDE_DIR vorbis/vorbisenc.h
	$ENV{Vorbis_DIR}/include
	/opt/local/include
	/usr/local/include
	/usr/include
	)

set( LIBRARY_SEARCH_PATHS
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	$ENV{Vorbis_DIR}/lib
	${Vorbis_INCLUDE_DIR}/../lib
	)

set(Vorbis_NAMES_RELEASE vorbis libvorbis libvorbis_static)
find_library(Vorbis_LIBRARY_RELEASE
	NAMES ${Vorbis_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Vorbis_INCLUDE_DIR}/../lib/release
	)

set(Vorbis_NAMES_DEBUG vorbis_d libvorbis_d libvorbis_static_d )
find_library(Vorbis_LIBRARY_DEBUG
	NAMES ${Vorbis_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Vorbis_INCLUDE_DIR}/../lib/debug
	)

# if( Vorbis_LIBRARY_DEBUG AND NOT Vorbis_LIBRARY_RELEASE )
# 	set( Vorbis_LIBRARY_RELEASE ${Vorbis_LIBRARY_DEBUG} )
# endif()

if( Vorbis_LIBRARY_DEBUG AND Vorbis_LIBRARY_RELEASE )
	set( Vorbis_LIBRARY debug ${Vorbis_LIBRARY_DEBUG}
		optimized ${Vorbis_LIBRARY_RELEASE}
		CACHE FILEPATH "Vorbis library" FORCE )
elseif( Vorbis_LIBRARY_RELEASE )
	set( Vorbis_LIBRARY ${Vorbis_LIBRARY_RELEASE} CACHE FILEPATH "Vorbis library" FORCE )
elseif( Vorbis_LIBRARY_DEBUG )
	set( Vorbis_LIBRARY ${Vorbis_LIBRARY_DEBUG} CACHE FILEPATH "Vorbis library" FORCE )
else()
	#	set( Vorbis_LIBRARY CACHE FILEPATH "Vorbis library" FORCE )
endif()

set(Vorbis_FILE_NAMES_RELEASE vorbisfile libvorbisfile libvorbisfile_static )
find_library(Vorbis_FILE_LIBRARY_RELEASE
	NAMES ${Vorbis_FILE_NAMES_RELEASE}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Vorbis_INCLUDE_DIR}/../lib/release
	)

set(Vorbis_FILE_NAMES_DEBUG
	vorbisfile_d
	libvorbisfile_d
	libvorbisfile_static_d )
find_library(Vorbis_FILE_LIBRARY_DEBUG
	NAMES ${Vorbis_FILE_NAMES_DEBUG}
	PATHS
	${LIBRARY_SEARCH_PATHS}
	${Vorbis_INCLUDE_DIR}/../lib/debug
	)

if( Vorbis_FILE_LIBRARY_DEBUG AND Vorbis_FILE_LIBRARY_RELEASE )
	set( Vorbis_FILE_LIBRARY debug ${Vorbis_FILE_LIBRARY_DEBUG}
		optimized ${Vorbis_FILE_LIBRARY_RELEASE}
		CACHE FILEPATH "Vorbis file library" FORCE )
elseif( Vorbis_FILE_LIBRARY_DEBUG )
	set( Vorbis_FILE_LIBRARY ${Vorbis_FILE_LIBRARY_DEBUG}
		CACHE FILEPATH "Vorbis file library" FORCE )
elseif( Vorbis_FILE_LIBRARY_RELEASE )
	set( Vorbis_FILE_LIBRARY ${Vorbis_FILE_LIBRARY_RELEASE}
		CACHE FILEPATH "Vorbis file library" FORCE )
else()
	#set( Vorbis_FILE_LIBRARY CACHE FILEPATH "Vorbis file library" FORCE )
endif()

if( Vorbis_INCLUDE_DIR AND Vorbis_LIBRARY )
	set( Vorbis_FOUND TRUE )
else()
	set( Vorbis_FOUND FALSE )
endif()

if( Vorbis_FOUND )
	if( NOT Vorbis_FIND_QUIETLY )
		message( STATUS "Found Vorbis: ${Vorbis_LIBRARY}" )
		if( Vorbis_FILE_LIBRARY )
			message( STATUS "Found Vorbisfile: ${Vorbis_FILE_LIBRARY}" )
		endif()
	endif( NOT Vorbis_FIND_QUIETLY )
else( Vorbis_FOUND )
	if( Vorbis_FIND_REQUIRED )
		message( STATUS "Looked for Vorbis libraries named ${Vorbis_NAMES}." )
		message(STATUS "Include file detected: [${Vorbis_INCLUDE_DIR}].")
		message(STATUS "Lib file detected: [${Vorbis_LIBRARY}].")
	endif (Vorbis_FIND_REQUIRED)
endif( Vorbis_FOUND )

mark_as_advanced(
	Vorbis_LIBRARY
	Vorbis_LIBRARY_RELEASE
	Vorbis_LIBRARY_DEBUG
	Vorbis_FILE_LIBRARY
	Vorbis_FILE_LIBRARY_RELEASE
	Vorbis_FILE_LIBRARY_DEBUG
	Vorbis_INCLUDE_DIR
	)

