# - Locate CEGUI library
# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2010-12
# This file is part of eqOgre build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#  CEGUI_INCLUDE_DIR, where to find headers.
#  CEGUI_LIBRARY, the library to link against, both optimized and debug
#  CEGUI_LIBRARY_RELEASE, the library to link against, optimized
#  CEGUI_LIBRARY_DEBUG, the library to link against, debug
#  CEGUI_FOUND, if false, do not try to link to CEGUI
# TODO add CEGUI_VERSION support
# TODO add optimized and release versions
# TODO should be tested on Windows
# TODO before testing the CEGUI lib should be built on windows
# TODO this needs the addition of CMake build system for CEGUI
   
if( CEGUI_LIBRARY AND CEGUI_INCLUDE_DIR )
	# in cache already
	set( CEGUI_FIND_QUIETLY TRUE )
endif( CEGUI_LIBRARY AND CEGUI_INCLUDE_DIR )


find_path(CEGUI_INCLUDE_DIR
	CEGUI
	PATHS
	$ENV{CEGUI_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	#PATH_SUFFIXES cegui CEGUI
)

find_library(CEGUI_LIBRARY
	NAMES CEGUIBase
	PATHS
	$ENV{CEGUI_DIR}/lib
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
   
IF(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
	SET(CEGUI_FOUND "YES")
	SET(CEGUI_INCLUDE_DIRS "${CEGUI_INCLUDE_DIRS}/CEGUI")
IF(NOT CEGUI_FIND_QUIETLY)
	MESSAGE(STATUS "Found CEGUI: ${CEGUI_LIBRARY}")
	ENDIF(NOT CEGUI_FIND_QUIETLY)
ELSE(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
IF(NOT CEGUI_FIND_QUIETLY)
	MESSAGE(STATUS "Warning: Unable to find CEGUI!")
	ENDIF(NOT CEGUI_FIND_QUIETLY)
ENDIF(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
