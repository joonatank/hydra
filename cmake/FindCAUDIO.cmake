# - Locate CAUDIO library
# This module defines
#  CAUDIO_LIBRARY, the library to link against
#  CAUDIO_FOUND, if false, do not try to link to CAUDIO
#  CAUDIO_INCLUDE_DIR, where to find headers.

IF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
	# in cache already
	SET(CAUDIO_FIND_QUIETLY TRUE)
ENDIF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)


FIND_PATH(CAUDIO_INCLUDE_DIR
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

FIND_LIBRARY(CAUDIO_LIBRARY
	NAMES cAudio cAudio.2.1.0
	PATHS
	$ENV{CAUDIO_DIR}/lib
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
   
IF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
	SET(CAUDIO_FOUND "YES")
	SET(CAUDIO_INCLUDE_DIR "${CAUDIO_INCLUDE_DIR};${CAUDIO_INCLUDE_DIR}/cAudio")
	IF(NOT CAUDIO_FIND_QUIETLY)
		MESSAGE(STATUS "Found CAUDIO: ${CAUDIO_LIBRARY}")
	ENDIF(NOT CAUDIO_FIND_QUIETLY)
ELSE(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
	IF(NOT CAUDIO_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find CAUDIO!")
	ENDIF(NOT CAUDIO_FIND_QUIETLY)
ENDIF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
