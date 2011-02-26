
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