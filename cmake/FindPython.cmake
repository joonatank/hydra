#-------------------------------------------------------------------
# Joonatan Kuosa <joonatan.kuosa@tut.fi>
# 2011-02
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------
#
# - Find python libraries
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHON_FOUND		- have the Python libs been found
#  PYTHON_LIBRARY		- path to the python library, both debug and optimized
#  PYTHON_INCLUDE_DIR		- path to where Python.h is found
#  PYTHON_LIBRARY_DEBUG		- path to the debug library
#  PYTHON_LIBRARY_RELEASE	- path to the release library
#  PYTHON_EXE_FOUND		- Was the Python executable found
#  PYTHON_EXECUTABLE		- path to the Python interpreter
#
#  Parameters
#  PYTHON_VERSION
# - Version of python to find, if empty any version of python is accepted
# If 2, or 3 is provided as version then any minor version is accepted
# This variable will be set when the python is found
#
# TODO Test on Linux
# TODO test Finding version 2 and 3, separately
# TODO add setting of PYTHON_VERSION when it's found
# TODO add both VERSION and additional versions, or should we?

if( PYTHON_INCLUDE_DIR AND PYTHON_LIBRARY )
	set( PYTHON_FIND_QUIETLY TRUE )
endif()

set( VERSIONS_2 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 )
set( VERSIONS_3 3.2 3.1 3.0 )

if( NOT PYTHON_VERSION )
	set( PYTHON_VERSIONS ${VERSIONS_3} ${VERSIONS_2} )
elseif( PYTHON_VERSION EQUAL "2" )
	set( PYTHON_VERSIONS ${VERSIONS_2} )
elseif( PYTHON_VERSION EQUAL "3" )
	set( PYTHON_VERSIONS ${VERSIONS_3} )
else()
	set( PYTHON_VERSIONS ${PYTHON_VERSION} )
endif()

foreach( _CURRENT_VERSION ${PYTHON_VERSIONS} )
	# Linux does use dots
	# Windows doesn't use dots
	string(REPLACE "." "" _CURRENT_VERSION_NO_DOTS ${_CURRENT_VERSION})

	find_library( PYTHON_LIBRARY_DEBUG
		NAMES python${_CURRENT_VERSION_NO_DOTS}_d
		PATHS
		[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
		[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs/Debug
		/usr/lib
		/usr/local/lib
		)

	find_library( PYTHON_LIBRARY_RELEASE
		NAMES python${_CURRENT_VERSION_NO_DOTS} python${_CURRENT_VERSION}
		PATHS
		[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
		/usr/lib
		/usr/local/lib
		# Avoid finding the .dll in the PATH.  We want the .lib.
		NO_SYSTEM_ENVIRONMENT_PATH
		)

	# TODO should check if the PYTHON_INCLUDE_DIR is valid or not
	find_path( PYTHON_INCLUDE_DIR
		NAMES Python.h
		PATHS
		[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/include
		/usr/include/python${_CURRENT_VERSION}
		/usr/local/include/python${_CURRENT_VERSION}
		)

	if( PYTHON_LIBRARY_DEBUG AND PYTHON_LIBRARY_RELEASE )
		set( PYTHON_LIBRARY debug ${PYTHON_LIBRARY_DEBUG}
			optimized ${PYTHON_LIBRARY_RELEASE}
			CACHE FILEPATH "Python library" FORCE )
	elseif( PYTHON_LIBRARY_RELEASE )
		set( PYTHON_LIBRARY ${PYTHON_LIBRARY_RELEASE}
			CACHE FILEPATH "Python library" FORCE )
	elseif( PYTHON_LIBRARY_DEBUG )
		set( PYTHON_LIBRARY ${PYTHON_LIBRARY_DEBUG}
			CACHE FILEPATH "Python library" FORCE )
	endif( PYTHON_LIBRARY_DEBUG AND PYTHON_LIBRARY_RELEASE )

	if( PYTHON_INCLUDE_DIR AND PYTHON_LIBRARY )
		message(STATUS "Setting python to found" )
		set( PYTHON_FOUND TRUE )
	else()
		message(STATUS "Setting python to NOT found : because include dir = ${PYTHON_INCLUDE_DIR}"
			" : library = ${PYTHON_LIBRARY}" )
		set( PYTHON_FOUND FALSE )
	endif()

	# TODO find the python interpreter
	find_program( PYTHON_EXECUTABLE python NAMES python${_CURRENT_VERSION}
		PATHS
		[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]
		)

	if( PYTHON_EXECUTABLE )
		set( PYTHON_EXE_FOUND TRUE )
	endif()

	# Take the first found Python version
	if( PYTHON_FOUND AND PYTHON_EXE_FOUND )
		message(STATUS "Found python lib and python executable" )
		break()
	endif()

endforeach( _CURRENT_VERSION )


if( PYTHON_FOUND OR PYTHON_FIND_QUIETLY )
	mark_as_advanced(
		PYTHON_LIBRARY_DEBUG
		PYTHON_LIBRARY_RELEASE
		PYTHON_INCLUDE_DIR
		)
endif()

if( PYTHON_EXE_FOUND OR PYTHON_FIND_QUIETLY )
	mark_as_advanced( PYTHON_EXECUTABLE )
endif()

