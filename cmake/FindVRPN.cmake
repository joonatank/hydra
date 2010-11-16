# - Locate VRPN library
# This module defines
#  VRPN_LIBRARY, the library to link against
#  VRPN_FOUND, if false, do not try to link to VRPN
#  VRPN_INCLUDE_DIRS, where to find headers.

IF(VRPN_LIBRARY AND VRPN_INCLUDE_DIRS)
	# in cache already
	SET(VRPN_FIND_QUIETLY TRUE)
ENDIF(VRPN_LIBRARY AND VRPN_INCLUDE_DIRS)

# VRPN depends on threads
find_package( Threads REQUIRED )
if( NOT ${Threads_FOUND} )
	# TODO this should refuse to find VRPN
	message( "Problem finding Threads. VRPN will not work" )
endif()

FIND_PATH(VRPN_INCLUDE_DIRS
	vrpn_Tracker.h
	PATHS
	$ENV{VRPN_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	PATH_SUFFIXES
)

FIND_LIBRARY(VRPN_LIBRARY
	NAMES vrpn
	PATHS
	$ENV{VRPN_DIR}/lib
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
   
IF(VRPN_LIBRARY AND VRPN_INCLUDE_DIRS)
	SET(VRPN_FOUND "YES")
#	SET(VRPN_INCLUDE_DIRS "${VRPN_INCLUDE_DIRS};${VRPN_INCLUDE_DIRS}/cAudio")
	IF(NOT VRPN_FIND_QUIETLY)
		MESSAGE(STATUS "Found VRPN: ${VRPN_LIBRARY}")
	ENDIF(NOT VRPN_FIND_QUIETLY)
	# TODO this should link the ${VRPN_LIBRARY} to the threads lib
#	target_link_libraries( ${VRPN_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} )
ELSE(VRPN_LIBRARY AND VRPN_INCLUDE_DIRS)
	IF(NOT VRPN_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find VRPN!")
	ENDIF(NOT VRPN_FIND_QUIETLY)
ENDIF(VRPN_LIBRARY AND VRPN_INCLUDE_DIRS)
