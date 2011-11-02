# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# - Locate VRPN library
#
# This module defines
#  VRPN_INCLUDE_DIR - where to find headers.
#  VRPN_FOUND, if false, do not try to link to VRPN
#  VRPN_LIBRARY - VRPN client library vrpn, both debug and optimized versions
#  VRPN_LIBRARY_RELEASE - VRPN client library vrpn, optimized version
#  VRPN_LIBRARY_DEBUG - VRPN client library vrpn, debug version

#  VRPN_SERVER_FOUND, if false, do not try to link to VRPN server
#  VRPN_SERVER_LIBRARY - VRPN server library vrpnserver, both debug and optimized versions
#  VRPN_SERVER_LIBRARY_RELEASE - VRPN client library vrpn, optimized version
#  VRPN_SERVER_LIBRARY_DEBUG - VRPN client library vrpn, debug version

IF( VRPN_INCLUDE_DIR )
	# in cache already
	set(VRPN_FIND_QUIETLY TRUE)
ENDIF( VRPN_INCLUDE_DIR )

# VRPN depends on threads
find_package( Threads QUIET )
if( NOT Threads_FOUND AND NOT VRPN_FIND_QUIETLY )
	# TODO this should refuse to find VRPN
	message( FATAL_ERROR "Problem finding Threads. VRPN will not work" )
endif()

FIND_PATH(VRPN_INCLUDE_DIR
	vrpn_Tracker.h
	PATHS
	$ENV{VRPN_DIR}/include
	/usr/local/include
	/usr/include
	PATH_SUFFIXES
	)

# Common library search paths, for both Windows and Linux
set( LIB_SEARCH_PATHS
	$ENV{VRPN_DIR}/lib
	/usr/lib
	/usr/local/lib
	${VRPN_INCLUDE_DIR}/../lib
	)

# Find the client library, release
find_library(VRPN_LIBRARY_RELEASE
	NAMES vrpn libvrpn
	PATHS
	${LIB_SEARCH_PATHS}
	${VRPN_INCLUDE_DIR}/../lib/Release
	)


find_library(VRPN_LIBRARY_DEBUG
	NAMES vrpn_d vrpn libvrpn libvrpn_d
	PATHS
	${LIB_SEARCH_PATHS}
	${VRPN_INCLUDE_DIR}/../lib/Debug
	)

if( VRPN_LIBRARY_DEBUG AND VRPN_LIBRARY_RELEASE )
	set( VRPN_LIBRARY debug ${VRPN_LIBRARY_DEBUG}
		optimized ${VRPN_LIBRARY_RELEASE}
		CACHE FILEPATH "VRPN Client library" FORCE)
elseif( VRPN_LIBRARY_DEBUG )
	set( VRPN_LIBRARY ${VRPN_LIBRARY_DEBUG}
		CACHE FILEPATH "VRPN Client library" FORCE)
elseif( VRPN_LIBRARY_RELEASE )
	set( VRPN_LIBRARY ${VRPN_LIBRARY_RELEASE}
		CACHE FILEPATH "VRPN Client library" FORCE)
else()
	#set( VRPN_LIBRARY CACHE FILEPATH "VRPN Client library" FORCE)
endif()

IF(VRPN_LIBRARY AND VRPN_INCLUDE_DIR)
	SET(VRPN_FOUND "YES")
	IF(NOT VRPN_FIND_QUIETLY)
		MESSAGE(STATUS "Found VRPN: ${VRPN_LIBRARY}")
	ENDIF(NOT VRPN_FIND_QUIETLY)
	# TODO this should link the ${VRPN_LIBRARY} to the threads lib
#	target_link_libraries( ${VRPN_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} )
ELSE(VRPN_LIBRARY AND VRPN_INCLUDE_DIR)
	IF(NOT VRPN_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find VRPN!")
	ENDIF(NOT VRPN_FIND_QUIETLY)
ENDIF(VRPN_LIBRARY AND VRPN_INCLUDE_DIR)

if( VRPN_FOUND OR VRPN_FIND_QUIETLY )
	mark_as_advanced(
		VRPN_INCLUDE_DIR
		VRPN_LIBRARY
		VRPN_LIBRARY_RELEASE
		VRPN_LIBRARY_DEBUG
		)
endif()

# Find the server library
FIND_LIBRARY(VRPN_SERVER_LIBRARY_RELEASE
	NAMES vrpnserver
	PATHS
	${LIB_SEARCH_PATHS}
	${VRPN_INCLUDE_DIR}/../lib/Release
	)

FIND_LIBRARY(VRPN_SERVER_LIBRARY_DEBUG
	NAMES vrpnserver_d vrpnserver
	PATHS
	${LIB_SEARCH_PATHS}
	${VRPN_INCLUDE_DIR}/../lib/Debug
	)

if( VRPN_SERVER_LIBRARY_DEBUG AND VRPN_SERVER_LIBRARY_RELEASE )
	set( VRPN_SERVER_LIBRARY debug ${VRPN_SERVER_LIBRARY_DEBUG}
		optimized ${VRPN_SERVER_LIBRARY_RELEASE}
		CACHE FILEPATH "VRPN Server library" FORCE)
elseif( VRPN_SERVER_LIBRARY_DEBUG )
	set( VRPN_SERVER_LIBRARY ${VRPN_SERVER_LIBRARY_DEBUG}
		CACHE FILEPATH "VRPN Server library debug" FORCE)
elseif( VRPN_SERVER_LIBRARY_RELEASE )
	set( VRPN_SERVER_LIBRARY ${VRPN_SERVER_LIBRARY_RELEASE}
		CACHE FILEPATH "VRPN Server library release" FORCE)
else()
	#set( VRPN_SERVER_LIBRARY CACHE FILEPATH "VRPN Client library" FORCE)
endif()


IF(VRPN_SERVER_LIBRARY AND VRPN_INCLUDE_DIR)
	SET(VRPN_SERVER_FOUND "YES")
	IF(NOT VRPN_FIND_QUIETLY)
		message(STATUS "Found VRPN Server: ${VRPN_SERVER_LIBRARY}")
	ENDIF(NOT VRPN_FIND_QUIETLY)
	# TODO this should link the ${VRPN_LIBRARY} to the threads lib
ELSE(VRPN_SERVER_LIBRARY AND VRPN_INCLUDE_DIR)
	IF(NOT VRPN_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find VRPN Server!")
	ENDIF(NOT VRPN_FIND_QUIETLY)
ENDIF(VRPN_SERVER_LIBRARY AND VRPN_INCLUDE_DIR)

if( VRPN_FOUND OR VRPN_FIND_QUIETLY )
	mark_as_advanced(
		VRPN_SERVER_LIBRARY
		VRPN_SERVER_LIBRARY_RELEASE
		VRPN_SERVER_LIBRARY_DEBUG
		)
endif()

