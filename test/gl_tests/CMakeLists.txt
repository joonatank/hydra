# author: Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# date: 2011-08
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project("gl_tests" CXX)

cmake_minimum_required(VERSION 2.6)

if( WIN32 )
	include_directories(${GLEW_SOURCE_DIR})
	set(HEADERS gl_common.hpp)

	add_definitions(-DGLEW_STATIC)

	link_libraries(${OPENGL_LIBRARIES} glew)

	add_executable(fbo_rendering WIN32 fbo_rendering.cpp
		${HEADERS} gl_fbo.hpp)

	add_executable(nv_swap_sync WIN32 nv_swap_sync.cpp 
		${HEADERS}) 

	add_executable(nv_copy_image WIN32 nv_copy_image.cpp
		${HEADERS} gl_fbo.hpp)

	add_executable(nv_gpu_affinity WIN32 nv_gpu_affinity.cpp
		${HEADERS} gl_fbo.hpp)
endif()

