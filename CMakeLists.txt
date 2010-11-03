project("vl" CXX C)

cmake_minimum_required(VERSION 2.8)

enable_testing()

if( UNIX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
endif( UNIX )

if( WIN32 )
	add_definitions( -DVL_WIN32		# Custom value used throughout the VL project
					 -DNOMINMAX		# Don't pollute the global namespace
					 -DNTDDI_WINXP
					 -DNOCOMM
					 -DNOGDI
					 -DNOSOUND
					 )
elseif( UNIX )
	add_definitions( -DVL_UNIX -DLinux )
endif( WIN32 )

# Set unix debug flag
if( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
	add_definitions( -DDEBUG )
endif()

# Add our local scripts to CMake path
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/CMake )

######## Find Equalizer ########
# We should expose these parameters for the user even in Linux
# We should use find_file and file_lib functions to find the files first though
find_library( EQUALIZER_LIBRARY
	NAMES Equalizer
	PATHS $ENV{EQUALIZER_HOME} $ENV{EQUALIZER_ROOT} $ENV{EQ_ROOT} $ENV{EQ_HOME}
	)

# TODO this needs to be tested in Windows
# The path is really $ENV{EQUALIZER_HOME}/include/eq/eq.h
find_path( EQUALIZER_INCLUDE_DIR
	NAMES eq/eq.h
	PATHS $ENV{EQUALIZER_HOME} $ENV{EQUALIZER_ROOT} $ENV{EQ_ROOT} $ENV{EQ_HOME}
	)

######### Find Turtle ########
# We should expose these parameters for the user even in Linux
# We should use find_file and file_lib functions to find the files first though

find_path( TURTLE_INCLUDE_DIR
	NAMES turtle/mock.hpp
	PATHS $ENV{TURTLE_HOME} $ENV{TURTLE_ROOT} $ENV{TURTLE_ROOT} $ENV{TURTLE_HOME}
	)


###### Find VRPN ########
# TODO
# We should use enviroment variables for detecting VRPN dir
# Also remove separate lib and include dir variables using one and using find_lib
# function should be enough.
# TODO
# VRPN should not be required but if it's not available vrpn_Tracker should not be
# included and build into the eqOgre library
# Also requirement for pthread would drop if VRPN was not available

find_package( Threads REQUIRED )

find_library( VRPN_LIBRARY
	NAMES vrpn
	)

# Include dir
find_path( VPRN_INCLUDE_DIR
	NAMES vrpn_Tracker.h
	)

######### Find OGRE ##########
# We should use find_file to find the Ogre cmake helpers
# TODO this needs a bit more work
find_path( OGRE_CMAKE_DIR "FindOGRE.cmake"
	HINTS /usr/lib/OGRE/cmake/;/usr/local/lib/OGRE/cmake/;$ENV{OGRE_HOME}/CMake )

set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${OGRE_CMAKE_DIR}" )

# TODO should expose Ogre and OpenGL packages to the user
find_package(OpenGL REQUIRED)
find_package(OGRE REQUIRED)

FIND_PACKAGE(OIS REQUIRED )

# TODO test on a system that does not have CEGUI
FIND_PACKAGE( CEGUI QUIET )

if( ${CEGUI_FOUND} )
	message( "CEGUI found : ${CEGUI_LIBRARY} " )
elseif()
	message( "CEGUI not found, will not build a GUI" )
endif()

####### Find Boost ########
# TODO
# Add selectable components
# Unit tests are only necessary for the test modules (not for lib or spikes)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS python REQUIRED )

find_package(PythonLibs REQUIRED )

find_package(PythonInterp REQUIRED )

message( "Boost Libraries = ${Boost_PYTHON_LIBRARIES}" )

message( "Python = ${PYTHON_EXECUTABLE}" )

message( "Python libraries = ${PYTHON_LIBRARIES}" )

include_directories( ${PYTHON_INCLUDE_DIRS} )

######### Add include and link directories ###########
include_directories( SYSTEM 
 	${OpenGL_INCLUDE_DIR}
 	${OGRE_INCLUDE_DIR}
	${EQUALIZER_INCLUDE_DIR}
	${TURTLE_INCLUDE_DIR}
	${VPRN_INCLUDE_DIR}
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	)

# Libraries that eqOgre library depends on
set( EQOGRE_DEB_LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${EQUALIZER_LIBRARY}
	${VRPN_LIBRARY}
	${OIS_LIBRARIES}
	${PYTHON_LIBRARIES}
	${Boost_PYTHON_LIBRARIES}
	)

# Libraries used to build project binaries
set( LIBS eqOgre )

# Add Boost library dir because Equalizer uses relative paths to boost libs
# Linking to Equalizer will fail without it.
link_directories( ${vl_BINARY_DIR}/vl 
	${Boost_LIBRARY_DIRS} )

############ Config Copy MACRO ##############
# TODO macro should be moved to separate file
MACRO( copy_config )
# Config file for spikes
message( "Copying config file for project = ${PROJECT_NAME}"  )

set( DATA_DIR ${vl_SOURCE_DIR}/data )

set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )
set( VL_MEDIA_PATH ${vl_SOURCE_DIR}/../eqOgre-media )

if( WIN32 )
	configure_file( ${DATA_DIR}/test_conf.xml
		${PROJECT_BINARY_DIR}/Release/test_conf.xml )
	configure_file( ${DATA_DIR}/test_conf.xml
		${PROJECT_BINARY_DIR}/Debug/test_conf.xml )
else()
	configure_file( ${DATA_DIR}/test_conf.xml
		${PROJECT_BINARY_DIR}/test_conf.xml )
endif()


# Plugins file
# Copyed to the vl root dir so that the plugins path are correct
if( WIN32 )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_REL} )
	configure_file( ${DATA_DIR}/plugins.cfg
		${PROJECT_BINARY_DIR}/Release/plugins.cfg )
	set( PLUGINS_DIR_DBG ${OGRE_PLUGIN_DIR_DBG} )
	configure_file( ${DATA_DIR}/plugins_d.cfg
		${PROJECT_BINARY_DIR}/Debug/plugins.cfg )
else( )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_TMP} )
	configure_file( ${DATA_DIR}/plugins.cfg
		${PROJECT_BINARY_DIR}/plugins.cfg )
endif()

ENDMACRO()

# include rendering engine, eq ogre directories
include_directories(vl)

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )

