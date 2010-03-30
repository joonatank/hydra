project("vl" CXX)

cmake_minimum_required(VERSION 2.6)

enable_testing()

if( UNIX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic" )
endif( UNIX )

if( WIN32 )
	add_definitions( -DVL_WIN32 )
elseif( UNIX )
	add_definitions( -DVL_UNIX )
endif( WIN32 )

if( WIN32 )
	set( EQUALIZER "c:/libs/equalizer" )
	set( TURTLE "C:/jotu/libs/turtle/src/libraries" )
elseif( UNIX )
	set( EQUALIZER "/usr/local" )
	set( TURTLE "/usr/local/" )
endif( WIN32 )

set(EQ_INCLUDE_DIR ${EQUALIZER}/include CACHE PATH "Equalizer include dir")
set(EQ_LIB_DIR ${EQUALIZER}/lib CACHE PATH "Equalizer library dir")
set(TURTLE_INCLUDE_DIR ${TURTLE} CACHE PATH "Turtle include dir")

if( WIN32 )
	set(CMAKE_MODULE_PATH "$ENV{OGRE_SDK}/CMake")
elseif( UNIX )
	set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake")
endif( WIN32 )

find_package(OpenGL)
find_package(OGRE)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

if( WIN32 )
	set( BOOST_INCLUDEDIR "C:/libs/boost/include/boost-1_42/"
		CACHE PATH "Boost include dir" )
elseif( UNIX )
	set( BOOST_INCLUDEDIR "/usr/include/boost-1_42/boost"
		CACHE PATH "Boost include dir" )
endif( WIN32 )


find_package(Boost 1.42.0 COMPONENTS unit_test_framework thread REQUIRED)
# TODO add error checking and expose Boost_ROOT to user if it's not env variable

message( "Boost version = ${Boost_VERSION}. Boost libraries = ${Boost_LIBRARIES}")

if( WIN32 )
	set( EQ_LIBRARIES Equalizer )
elseif( UNIX )
	set( EQ_LIBRARIES eq )
endif( WIN32 )

# TODO add define rendering engine (Ogre/OpenSG)
# include vl lib
# Using include instead of subdirectory because with include we can just
# add to source/headers list and directories without building anything.
#include(vl/CMakeList.txt)

include_directories( 
 	${OpenGL_INCLUDE_DIR}
 	${OGRE_INCLUDE_DIRS}
 	${OIS_INCLUDE_DIRS}
	${BOOST_INCLUDE}
	${EQ_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${TURTLE_INCLUDE_DIR}
	)

link_directories( ${Boost_LIBRARY_DIRS} ${EQ_LIB_DIR} )

# include rendering engine, eq ogre directories
include_directories(vl)

# Tests should be in different CMake file
subdirs( ${CMAKE_CURRENT_SOURCE_DIR}/test )

#add_executable( eqOgre ${EQOGRE_SOURCES} )
# TODO libraries can be found using CMake facilities
#target_link_libraries( eqOgre OgreMain eq eqserver GLEW pthread )

