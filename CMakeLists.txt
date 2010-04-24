project("vl" CXX)

cmake_minimum_required(VERSION 2.6)

enable_testing()

if( UNIX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic" )
endif( UNIX )

if( WIN32 )
	add_definitions( -DVL_WIN32 )
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

if( WIN32 )
	set( EQ_LIBRARIES Equalizer )
elseif( UNIX )
	set( EQ_LIBRARIES eq )
endif( WIN32 )

# Physics, Bullet for now
# TODO should be a plugin
set( BULLET_LIBRARIES BulletCollision BulletDynamics )

if( WIN32 )
	set( BULLET_LIB_DIR "" CACHE PATH "Bullet library path" )
	set( BULLET_INCLUDE_DIR "" CACHE PATH "Bullet include path" )
endif()
	
# Audio, cAudio for now
# TODO should be a plugin
set( CAUDIO_LIBRARIES cAudio )

if( WIN32 )
	set( CAUDIO_LIB_DIR "" CACHE PATH "cAudio library path" )
	set( CAUDIO_INCLUDE_DIR "" CACHE PATH "cAudio include path" )
endif()


include_directories( 
 	${OpenGL_INCLUDE_DIR}
 	${OGRE_INCLUDE_DIRS}
 	${OIS_INCLUDE_DIRS}
	${BOOST_INCLUDE}
	${EQ_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${TURTLE_INCLUDE_DIR}
	${BULLET_INCLUDE_DIR}
	${CAUDIO_INCLUDE_DIR}
	)

link_directories( ${Boost_LIBRARY_DIRS} ${EQ_LIB_DIR} ${BULLET_LIB_DIR}
	${CAUDIO_LIB_DIR} )

link_directories( ${vl_BINARY_DIR}/vl )

# include rendering engine, eq ogre directories
include_directories(vl)

# Tests should be in different CMake file
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )

