# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2013-01
# This file is part of Eyes build system.
# This file is in public domain, feel free to use it as you see fit
#
# - Locate GLM library
#
# This module defines
#  GLM_INCLUDE_DIR - where to find headers.
#  GLM_FOUND, if false, do not try to link to GLM

# TODO we should support both GLM_INCLUDE_DIR finding based on global include
# dir. Similar to unix /usr/include.

# include already in cache
if( GLM_INCLUDE_DIR )
	if(EXISTS "${GLM_INCLUDE_DIR}/glm.hpp")
		set(GLM_FIND_QUIETLY TRUE)
	endif()
endif() 

find_path(GLM_INCLUDE_DIR
	glm.hpp
	PATHS
	$ENV{GLM_DIR}/include
	/usr/local/include
	/usr/include
	PATH_SUFFIXES
	)

# Common library search paths, for both Windows and Linux
set( LIB_SEARCH_PATHS
	$ENV{GLM_DIR}/lib
	/usr/lib
	/usr/local/lib
	${GLM_INCLUDE_DIR}/../lib
	)

if(GLM_INCLUDE_DIR AND EXISTS "${GLM_INCLUDE_DIR}/glm.hpp")
	set(GLM_FOUND "YES")
	if(NOT GLM_FIND_QUIETLY)
		message(STATUS "Found GLM: ${GLM_LIBRARY}")
	endif()
else()
	unset(GLM_FOUND)
	if(NOT GLM_FIND_QUIETLY)
		message(STATUS "Warning: Unable to find GLM!")
	endif()
endif()

if( GLM_FOUND OR GLM_FIND_QUIETLY )
	mark_as_advanced(
		GLM_INCLUDE_DIR
		GLM_LIBRARY
		GLM_LIBRARY_RELEASE
		GLM_LIBRARY_DEBUG
		)
endif()

