#-------------------------------------------------------------------
# Joonatan Kuosa
# 2010-03
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "pyogre" CXX )

cmake_minimum_required(VERSION 2.6)

set( SRC
	mesh_writer.cpp
	ogre_python.cpp
	)

set( HEADERS
	mesh_writer.hpp
	)

add_library( pyogre MODULE ${SRC} ${HEADERS} )

set( LIBS 
	${PYTHON_LIBRARY}
	${Boost_PYTHON_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Ogre_LIBRARY}
	)

target_link_libraries( pyogre ${LIBS} )

set_target_properties(pyogre PROPERTIES SUFFIX ".pyd")

if( EQOGRE_BUILD_DEMOS )
	foreach(i ${CMAKE_CONFIGURATION_TYPES} )
		file(TO_NATIVE_PATH ${pyogre_BINARY_DIR}/${i}/pyogre.pyd cp_src)
		file(TO_NATIVE_PATH ${demos_BINARY_DIR}/${i} cp_dest)
		
		add_custom_command(TARGET pyogre POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			ARGS ${cp_src} ${cp_dest}
			DEPENDS ${cp_src}
			COMMENT "copying pyogre to demos"
			VERBATIM
			)
	endforeach()

endif()
