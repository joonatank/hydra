# - Locate OpenCollda library
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2011-10
# 
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# This module defines
#
# OPENCOLLADA_INCLUDE_DIR, where to find headers.
# OPENCOLLADA_LIBRARIES, all the OPENCOLLADA libraries and dependencies,
#	both debug and optimized
# OPENCOLLADA_FOUND, if false, do not try to link to OPENCOLLADA
#
# Which one of these we need?
#OpenCOLLADABaseUtils
#OpenCOLLADAFramework
#OpenCOLLADASaxFrameworkLoader
#OpenCOLLADAStreamWriter
# Do we need these?
#MathMLSolver
#GeneratedSaxParser
#buffer
#ftoa
#UTF
# OPENCOLLADA_BASE_UTILS_LIBRARY, the base library, both optimized and debug
# OPENCOLLADA_BASE_UTILS_LIBRARY_RELEASE, the base library, optimized
# OPENCOLLADA_BASE_UTILS_LIBRARY_DEBUG, the base library, debug
#
# OPENCOLLADA_FRAMEWORK_LIBRARY
# OPENCOLLADA_FRAMEWORK_LIBRARY_RELEASE
# OPENCOLLADA_FRAMEWORK_LIBRARY_DEBUG
#

if( OPENCOLLADA_LIBRARIES AND OPENCOLLADA_INCLUDE_DIR )
	# in cache already
	set( OPENCOLLADA_FIND_QUIETLY TRUE )
endif( OPENCOLLADA_LIBRARIES AND OPENCOLLADA_INCLUDE_DIR )

include( ${vl_SOURCE_DIR}/cmake/LibFinder.cmake )

set_include_paths(INCLUDE_PATHS OPENCOLLADA)

set_lib_paths(LIBRARY_PATHS OPENCOLLADA)

find_path( OPENCOLLADA_INCLUDE_DIR
	NAMES COLLADAFramework/COLLADAFW.h
	PATHS ${INCLUDE_PATHS}
	#PATH_SUFFIXES OPENCOLLADA OpenCollada
	)

set( INCLUDE_DIR ${OPENCOLLADA_INCLUDE_DIR} )
# Find the base library
set( LIB_HELP_STRING "OPENCOLLADA Base utils library" )
lib_finder("OPENCOLLADA_BASE_UTILS" OpenCOLLADABaseUtils)
lib_finder("OPENCOLLADA_FRAMEWORK" OpenCOLLADAFramework)
lib_finder("OPENCOLLADA_SAX_FRAMEWORK_LOADER" OpenCOLLADASaxFrameworkLoader)
lib_finder("OPENCOLLADA_STREAM_WRITER" OpenCOLLADAStreamWriter)
lib_finder("OPENCOLLADA_MATH_ML_SOLVER" MathMLSolver)
lib_finder("OPENCOLLADA_UTF" UTF)
lib_finder("OPENCOLLADA_GENERATED_SAX_PARSER" GeneratedSaxParser)
lib_finder("OPENCOLLADA_PCRE" pcre)
lib_finder("OPENCOLLADA_LIBXML" xml)

message(STATUS "${LIB_HELP_STRING} = ${OPENCOLLADA_BASE_UTILS_LIBRARY}" )

# TODO should check all the libraries

set(OPENCOLLADA_LIBRARIES
	${OPENCOLLADA_BASE_UTILS_LIBRARY}
	${OPENCOLLADA_FRAMEWORK_LIBRARY}
	${OPENCOLLADA_SAX_FRAMEWORK_LOADER_LIBRARY}
	${OPENCOLLADA_STREAM_WRITER_LIBRARY}
	${OPENCOLLADA_MATH_ML_SOLVER_LIBRARY}
	${OPENCOLLADA_UTF_LIBRARY}
	${OPENCOLLADA_GENERATED_SAX_PARSER_LIBRARY}
	${OPENCOLLADA_PCRE_LIBRARY}
	${OPENCOLLADA_LIBXML_LIBRARY}
	CACHE INTERNAL "" FORCE
	)

message(STATUS "OpenCollada libraries = ${OPENCOLLADA_LIBRARIES}")

if( OPENCOLLADA_LIBRARIES AND OPENCOLLADA_INCLUDE_DIR )
	set( OPENCOLLADA_FOUND "YES" )
	if( NOT OPENCOLLADA_FIND_QUIETLY )
		message( STATUS "Found OPENCOLLADA: ${OPENCOLLADA_LIBRARIES}" )
	endif( NOT OPENCOLLADA_FIND_QUIETLY )
else()
	if( NOT OPENCOLLADA_FIND_QUIETLY )
		message( STATUS "Warning: Unable to find OPENCOLLADA!" )
	endif( NOT OPENCOLLADA_FIND_QUIETLY )
endif()

