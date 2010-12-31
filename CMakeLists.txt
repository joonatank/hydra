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

project("vl" CXX C)

cmake_minimum_required(VERSION 2.8)

if( UNIX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
endif( UNIX )

if( WIN32 )
	add_definitions( -DVL_WIN32		# Custom value used throughout the VL project
					 -DNOMINMAX		# Don't pollute the global namespace
					 -D_WIN32_WINNT=0x0501	# Windows XP target
					 -DNOCOMM
					 -DNOGDI
					 -DNOSOUND
					 #-DEQ_RELEASE_ASSERT
					 # Should not need release asserts anymore
					 )
elseif( UNIX )
	add_definitions( -DVL_UNIX -DLinux )
	# Set unix debug flag
	if( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
		add_definitions( -DDEBUG )
	endif()
endif( WIN32 )



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

if( ${CEGUI_FOUND} )
	message( "CEGUI found : ${CEGUI_LIBRARY} " )
elseif()
	message( "CEGUI not found, will not build a GUI" )
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

find_package(Boost COMPONENTS python REQUIRED )

find_package(PythonLibs REQUIRED )

find_package(PythonInterp REQUIRED )

include_directories( ${PYTHON_INCLUDE_DIRS} )

set(Boost_USE_STATIC_LIBS   ON)

find_package(Boost COMPONENTS system filesystem program_options REQUIRED )

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
	)

# Libraries used to build project binaries
set( eqOgre_LIBRARY eqOgre )

# Add Boost library dir because Equalizer uses relative paths to boost libs
# Linking to Equalizer will fail without it.
link_directories( ${vl_BINARY_DIR}/vl
	${Boost_LIBRARY_DIRS} )

## Copy start script macro ##
MACRO( copy_start_script program_name project_path start_suffix )
message( "Copying start scripts for project = ${PROJECT_NAME}
	: exe name = ${program_name}" )

# Copy the single Node start script
set( PROGRAM_NAME ${program_name} )
set( DEMO_DATA_PATH ${vl_SOURCE_DIR}/data )
set( PROJ_PATH_RELATIVE ${project_path} )

if( WIN32 )
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/start.bat.in
		${PROJECT_BINARY_DIR}/Release/start_${start_suffix}.bat )
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/start.bat.in
		${PROJECT_BINARY_DIR}/RelWithDebInfo/start_${start_suffix}.bat )
else()
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/start.sh.in
		${PROJECT_BINARY_DIR}/start_${start_suffix}.sh )
endif()

# Copy the multi node listening start script

# TODO this needs to be automatically configured to match the host name of this computer
site_name( HOSTNAME )
set( HOSTNAME_TO_LISTEN ${HOSTNAME} )
message( "Hostname to listen = ${HOSTNAME_TO_LISTEN}" )

set( LISTENING_PORT 4242 )

if( WIN32 )
	# TODO these should be configured automatically
	# TODO not tested in Windows
	set( EQUALIZER_SERVER_BIN "c:/libs/equalizer/bin/eqServer.exe" )
	# TODO the config should be in the repository
	#set( EQUALIZER_CONFIG "c:\libs\equalizer\2-node.layout.eqc" )
	set( EQUALIZER_CONFIG ${DEMO_DATA_PATH}/eqc/2-node.layout.eqc )
	# Copy Release
	# TODO this really needs some automation/MACRO
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/start_multi_node.bat.in
		${PROJECT_BINARY_DIR}/Release/start_multi_${start_suffix}.bat )
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/kill_nodes.bat.in
		${PROJECT_BINARY_DIR}/Release/kill_nodes.bat )
	# Copy RelWithDebugInfo
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/start_multi_node.bat.in
		${PROJECT_BINARY_DIR}/RelWithDebInfo/start_multi_${start_suffix}.bat )
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/kill_nodes.bat.in
		${PROJECT_BINARY_DIR}/RelWithDebInfo/kill_nodes.bat )
else()
	set( EQUALIZER_SERVER_BIN "eqServer" )
	set( EQUALIZER_CONFIG ${DEMO_DATA_PATH}/eqc/2-node.layout.eqc )
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/start_multi_node.sh.in
		${PROJECT_BINARY_DIR}/start_multi_${start_suffix}.sh )
	configure_file( ${DEMO_DATA_PATH}/shell_scripts/kill_nodes.sh.in
		${PROJECT_BINARY_DIR}/kill_nodes.sh )
endif()

ENDMACRO()


# include rendering engine, eq ogre directories
include_directories(vl)

set( EQOGRE_BUILD_QTLAUNCHER TRUE CACHE BOOL
	"Build the GUI used for editing settings and launching." )
set( EQOGRE_BUILD_TESTS FALSE CACHE BOOL "Build the unit tests." )
set( EQOGRE_BUILD_DEMOS TRUE CACHE BOOL "Build the demos." )

# TODO should be configurable
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )

if( ${EQOGRE_BUILD_QTLAUNCHER} )
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/qtlauncher )
endif()

if( ${EQOGRE_BUILD_TESTS} )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )
endif()

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/demos )

include( Packaging )

include( InstallResources )

# TODO this doesn't work yet
#include( InstallDependencies )
