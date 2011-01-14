#-------------------------------------------------------------------
# Joonatan Kuosa
# 2010-11
#
# This file is part of the CMake build system for eqOgre
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

# TODO should move to using QUIET package searching
# and add checking only after all packages are searched

project("vl" CXX C)

cmake_minimum_required(VERSION 2.6)

if( UNIX )
	add_definitions( -Wall )
	add_definitions( -DVL_UNIX -DLinux )
	# Set unix debug flag
	#if( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
	#	add_definitions( -DDEBUG )
	#endif()
	#	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
elseif( WIN32 )
	add_definitions( -DVL_WIN32		# Custom value used throughout the VL project
					 -DNOMINMAX		# Don't pollute the global namespace
					 -D_WIN32_WINNT=0x0501	# Windows XP target
					 -DNOCOMM
					 -DNOSOUND
					 #-DEQ_RELEASE_ASSERT
					 # Should not need release asserts anymore
					 )
endif( UNIX )

if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES Debug Release)
endif(CMAKE_CONFIGURATION_TYPES)


# Add our local scripts to CMake path
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake )

######## Find Equalizer ########
find_package( Equalizer REQUIRED )

###### Find VRPN ########
find_package( VRPN REQUIRED )


# Way to test if the platform is 64 bit or 32 bit
# Useful for creating different enviroment variables for 32-bit and 64-bit
# platforms
# if(CMAKE_SIZEOF_VOID_P EQUAL 8)


######### Find OGRE ##########
find_package(OpenGL REQUIRED)
find_package(Ogre REQUIRED)

######### Find OIS ###########
find_package(OIS REQUIRED )

######### Find CEGUI #########
find_package( CEGUI QUIET )

if( CEGUI_FOUND )
	message( STATUS "CEGUI found : ${CEGUI_LIBRARY} " )
elseif()
	message( STATUS "CEGUI not found, will not build a GUI" )
endif()


####### Find cAudio
# Dependencies for cAudio
find_package( Ogg REQUIRED )
find_package( Vorbis REQUIRED )
find_package( OpenAL REQUIRED )

find_package( cAudio REQUIRED )


####### Find Boost ########
# TODO
# Add selectable components

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   OFF)

find_package(Boost COMPONENTS python REQUIRED )

find_package(PythonLibs REQUIRED )

find_package(PythonInterp REQUIRED )

include_directories( ${PYTHON_INCLUDE_DIRS} )

if( WIN32 )
	set(Boost_USE_STATIC_LIBS   ON)
endif()

find_package( Boost COMPONENTS system filesystem program_options REQUIRED )

find_package( Bullet REQUIRED )

######### Add include and link directories ###########
include_directories( SYSTEM
 	${OpenGL_INCLUDE_DIR}
 	${Ogre_INCLUDE_DIR}
	${Equalizer_INCLUDE_DIR}
	${VRPN_INCLUDE_DIR}
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${cAudio_INCLUDE_DIR}
	${OGG_INCLUDE_DIR}
	${VORBIS_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
	)

include_directories(
	${BULLET_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIRS}/bullet
	)

# Libraries that eqOgre library depends on
set( EQOGRE_DEB_LIBS
	${OPENGL_LIBRARIES}
	${Ogre_LIBRARY}
	${Equalizer_LIBRARY}
	${VRPN_LIBRARY}
	${OIS_LIBRARY}
	${PYTHON_LIBRARIES}
	${Boost_PYTHON_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARIES}
	${cAudio_LIBRARY}
	${Ogg_LIBRARY}
	${Vorbis_LIBRARY}
	${Vorbis_FILE_LIBRARY}
	${OPENAL_LIBRARY}
	${BULLET_LIBRARIES}
	)

# Libraries used to build project binaries
set( eqOgre_LIBRARY eqOgre )

# Add Boost library dir because Equalizer uses relative paths to boost libs
# Linking to Equalizer will fail without it.
link_directories( ${vl_BINARY_DIR}/vl ${Boost_LIBRARY_DIRS} )


include( CopyScripts )


# include rendering engine, eq ogre directories
include_directories(vl)

set( EQOGRE_BUILD_QTLAUNCHER TRUE CACHE BOOL
	"Build the GUI used for editing settings and launching." )
set( EQOGRE_BUILD_TESTS FALSE CACHE BOOL "Build the unit tests." )
set( EQOGRE_BUILD_DEMOS TRUE CACHE BOOL "Build the demos." )

# TODO should be configurable
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )

if( EQOGRE_BUILD_QTLAUNCHER )
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/qtlauncher )
endif()

if( EQOGRE_BUILD_TESTS )
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )
endif()

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/demos )

include( Packaging )

# TODO this doesn't work yet
#include( InstallResources )

# TODO this doesn't work yet
#include( InstallDependencies )
