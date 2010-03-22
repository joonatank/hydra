project("vl" CXX)

cmake_minimum_required(VERSION 2.6)

enable_testing()

#set(OGRE_INCLUDE /usr/local/include/OGRE CACHE PATH "Ogre include dir")
#set(BOOST_INCLUDE /usr/local/include CACHE PATH "Boost include dir")
#set(BOOST_LIB	/usr/local/lib	CACHE PATH "Boost library dir")

if( WIN32 )
	set( EQUALIZER "c:/libs/equalizer" )
	set( TURTLE "C:/jotu/libs/turtle/src/libraries" )
elseif( UNIX )
	set( EQUALIZER "/usr/local" )
endif( WIN32 )

set(EQ_INCLUDE_DIR ${EQUALIZER}/include CACHE PATH "Equalizer include dir")
set(EQ_LIB_DIR ${EQUALIZER}/lib CACHE PATH "Equalizer library dir")
set(TURTLE_INCLUDE_DIR ${TURTLE} CACHE PATH "Turtle include dir")

#set(OGRE_BUILD "c:/libs/OGRE")
#set(OGRE_DEPENDENCIES_DIR "c:/libs/ogre_dependencies")

#set(ENV{OGRE_SDK} "c:/libs/OGRE")

set(CMAKE_MODULE_PATH "$ENV{OGRE_SDK}/CMake")
#set(CMAKE_MODULE_PATH "C:/libs/OGRE/CMake")

find_package(OpenGL)
find_package(OGRE)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

set( BOOST_INCLUDEDIR "C:/libs/boost/include/boost-1_42/"
	CACHE PATH "Boost include dir" )

find_package(Boost 1.42.0 COMPONENTS unit_test_framework thread REQUIRED)
# TODO add error checking and expose Boost_ROOT to user if it's not env variable

message( "Boost version = ${Boost_VERSION}. Boost libraries = ${Boost_LIBRARIES}")

if( WIN32 )
	set( EQ_LIBRARIES Equalizer )
elseif( UNIX )
	set( EQ_LIBRARIES eq )
endif( WIN32 )

#message( "Building vl" )
# TODO add define rendering engine (Ogre/OpenSG)
# include vl lib
# Using include instead of subdirectory because with include we can just
# add to source/headers list and directories without building anything.
#include(vl/CMakeList.txt)

# TODO move to vl/CMakeList.txt)
#include_directories(SYSTEM ${OGRE_INCLUDE} ${BOOST_INCLUDE})

include_directories( SYSTEM
 	${OpenGL_INCLUDE_DIR}
 	${OGRE_INCLUDE_DIRS}
 	${OIS_INCLUDE_DIRS}
	${BOOST_INCLUDE}
	${EQ_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${TURTLE_INCLUDE_DIR}
	)

link_directories( SYSTEM ${Boost_LIBRARY_DIRS} ${EQ_LIB_DIR} )

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

