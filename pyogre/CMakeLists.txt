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

# Necessary for the mesh and mesh_serializer headers from HydraMain
include_directories(${HydraMain_SOURCE_DIR})

# Do not link to HydraMain, we have so few dependencies that it's easier to
# compile them separately to objs
set( SRC
	${HydraMain_SOURCE_DIR}/serializer.cpp
	${HydraMain_SOURCE_DIR}/mesh_serializer.cpp
	${HydraMain_SOURCE_DIR}/mesh_serializer_impl.cpp
	${HydraMain_SOURCE_DIR}/resource.cpp
	${HydraMain_SOURCE_DIR}/mesh.cpp
	${HydraMain_SOURCE_DIR}/mesh_ogre.cpp
	${HydraMain_SOURCE_DIR}/base/string_utils.cpp
	ogre_python.cpp
	)

set( HEADERS
	${HydraMain_SOURCE_DIR}/base/string_utils.hpp
	${HydraMain_SOURCE_DIR}/mesh.hpp
	${HydraMain_SOURCE_DIR}/mesh_ogre.hpp
	${HydraMain_SOURCE_DIR}/mesh_serializer.hpp
	${HydraMain_SOURCE_DIR}/mesh_serializer_impl.hpp
	${HydraMain_SOURCE_DIR}/serializer.hpp
	${HydraMain_SOURCE_DIR}/resource.hpp
	)

# Using module type for runtime dynamically loaded libraries
add_library( pyogre MODULE ${SRC} ${HEADERS} )

set( LIBS
	${PYTHON_LIBRARY}
	${Boost_PYTHON_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Ogre_LIBRARY}
	)

target_link_libraries( pyogre ${LIBS} )

# Remove Linux lib prefix because this is not a c++ library
set_target_properties(pyogre PROPERTIES PREFIX "")
# Python shared lib suffix
set(PYOGRE_MODULE_NAME "pyogre")
set(EXTE "so")
if( WIN32 )
	set_target_properties(pyogre PROPERTIES SUFFIX ".pyd")
	set(EXTE "pyd")
endif()


copy_binaries(${PYOGRE_MODULE_NAME} ${pyogre_BINARY_DIR} ${demos_BINARY_DIR} ${EXTE} )

