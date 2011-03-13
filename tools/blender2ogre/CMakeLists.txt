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

project( "blender2ogre" CXX )

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
