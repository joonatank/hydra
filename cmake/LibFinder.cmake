# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-10
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit


# TODO document
# TODO document the variables necessary to be set before running
# TODO move all the finders to use this
MACRO( lib_finder variable_name lib_name )
	find_library( ${variable_name}_LIBRARY_RELEASE
		NAMES ${lib_name}
		PATHS ${LIBRARY_PATHS}
		${INCLUDE_DIR}/../lib/Release
		)

	find_library( ${variable_name}_LIBRARY_DEBUG
		NAMES ${lib_name}d ${lib_name}_d
		PATHS ${LIBRARY_PATHS}
		${INCLUDE_DIR}/../lib/Debug
		)

	# Redo the search with same name as the Release
	# to avoid file collisions between the two we search only the dedicated Debug dir
	if(NOT ${variable_name}_LIBRARY_DEBUG)
		find_library(${variable_name}_LIBRARY_DEBUG
			NAMES ${lib_name}
			PATHS ${INCLUDE_DIR}/../lib/Debug
			)
	endif()

	if( ${variable_name}_LIBRARY_DEBUG AND ${variable_name}_LIBRARY_RELEASE )
		set( ${variable_name}_LIBRARY debug ${${variable_name}_LIBRARY_DEBUG}
			optimized ${${variable_name}_LIBRARY_RELEASE}
			CACHE FILEPATH ${LIB_HELP_STRING} FORCE )
	elseif( ${variable_name}_LIBRARY_RELEASE )
		set( ${variable_name}_LIBRARY ${${variable_name}_LIBRARY_RELEASE} CACHE FILEPATH ${LIB_HELP_STRING} FORCE )
	elseif( ${variable_name}_LIBRARY_DEBUG )
		set( ${variable_name}_LIBRARY ${${variable_name}_LIBRARY_DEBUG} CACHE FILEPATH ${LIB_HELP_STRING} FORCE )
	endif()

ENDMACRO()

MACRO(set_include_paths variable lib_name)
	list(APPEND ${variable}
		$ENV{${lib_name}_DIR}/include
		$ENV{${lib_name}_HOME}/include
		/usr/local/include
		/usr/include
		/sw/include
		/opt/local/include
		/opt/csw/include
		/opt/include
		)
ENDMACRO()

MACRO(set_lib_paths variable lib_name)
	list(APPEND ${variable}
		$ENV{${lib_name}_DIR}/lib
		$ENV{${lib_name}_HOME}/lib
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
ENDMACRO()

