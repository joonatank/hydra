project("vl" CXX)

cmake_minimum_required(VERSION 2.6)

enable_testing()

if( UNIX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
endif( UNIX )

if( WIN32 )
	add_definitions( -DVL_WIN32 -DNOMINMAX )
elseif( UNIX )
	add_definitions( -DVL_UNIX -DLinux )
endif( WIN32 )

# Set unix debug flag
if( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
	add_definitions( -DDEBUG )
endif()

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
	set(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake")
elseif( UNIX )
	set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake")
endif( WIN32 )

find_package(OpenGL REQUIRED)
find_package(OGRE REQUIRED)

message( "OpenGL libs = ${OPENGL_LIBRARIES}" )

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS unit_test_framework thread filesystem REQUIRED)
# TODO add error checking and expose Boost_ROOT to user if it's not env variable

#if( WIN32 )
	set( EQ_LIBRARIES Equalizer )
#elseif( UNIX )
#	set( EQ_LIBRARIES eq )
#endif( WIN32 )

if( WIN32 )
	set( VRPN_INCLUDE_DIR "" CACHE PATH "VRPN include path" )
	set( VRPN_LIB_DIR "" CACHE PATH "VRPN lib path" )
endif()
	

include_directories( 
 	${OpenGL_INCLUDE_DIR}
 	${OGRE_INCLUDE_DIRS}
 	${OIS_INCLUDE_DIRS}
	${BOOST_INCLUDE}
	${EQ_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${TURTLE_INCLUDE_DIR}
	${VRPN_INCLUDE_DIR}
	)

link_directories( ${Boost_LIBRARY_DIRS} ${EQ_LIB_DIR} ${VRPN_LIB_DIR} )

link_directories( ${vl_BINARY_DIR}/vl )

# include rendering engine, eq ogre directories
include_directories(vl)

# Tests should be in different CMake file
#add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )

