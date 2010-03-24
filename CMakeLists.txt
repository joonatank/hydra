project("vl" CXX)

cmake_minimum_required(VERSION 2.6)

enable_testing()

message( "Building vl" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic" )
message( "CXX_FLAGS = ${CMAKE_CXX_FLAGS}" )
message( "CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}" )
message( "CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}" )
# TODO add define rendering engine (Ogre/OpenSG)
# include vl lib
# Using include instead of subdirectory because with include we can just
# add to source/headers list and directories without building anything.
#include(vl/CMakeList.txt)

# TODO move to vl/CMakeList.txt)
include_directories(SYSTEM /usr/local/include/OGRE /usr/local/include/raknet)
link_directories(/usr/local/lib)

# COMPILE_FLAGS += 

# include rendering engine, eq ogre directories
include_directories(vl)

# include cnc lib inclde directories
#include_directories(cnc)

# TODO add tests
# Tests should be in different CMake file
subdirs( ${CMAKE_CURRENT_SOURCE_DIR}/test )

#add_executable( eqOgre ${EQOGRE_SOURCES} )
# TODO libraries can be found using CMake facilities
#target_link_libraries( eqOgre OgreMain eq eqserver GLEW pthread )

