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
# TODO
# We should use enviroment variables for detecting VRPN dir
# Also remove separate lib and include dir variables using one and using find_lib
# function should be enough.
# TODO
# VRPN should not be required but if it's not available vrpn_Tracker should not be
# included and build into the eqOgre library
# Also requirement for pthread would drop if VRPN was not available

find_package( VRPN REQUIRED )



######### Find OGRE ##########
# We should use find_file to find the Ogre cmake helpers
# TODO this needs a bit more work
find_path( OGRE_CMAKE_DIR "FindOGRE.cmake"
	HINTS /usr/lib/OGRE/cmake/;/usr/local/lib/OGRE/cmake/;$ENV{OGRE_HOME}/CMake
	)

set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${OGRE_CMAKE_DIR}" )

# TODO should expose Ogre and OpenGL packages to the user
find_package(OpenGL REQUIRED)
find_package(OGRE REQUIRED)

find_package(OIS REQUIRED )

find_package( CEGUI QUIET )



####### Find cAudio
find_package( CAUDIO REQUIRED )


####### Find Boost ########
# TODO
# Add selectable components
# Unit tests are only necessary for the test modules (not for lib or spikes)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS python system filesystem REQUIRED )

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
	${Equalizer_INCLUDE_DIRS}
	${VPRN_INCLUDE_DIR}
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${CAUDIO_INCLUDE_DIR}
	)

# Libraries that eqOgre library depends on
set( EQOGRE_DEB_LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Equalizer_LIBRARY}
	${VRPN_LIBRARY}
	${OIS_LIBRARIES}
	${PYTHON_LIBRARIES}
	${Boost_PYTHON_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	${CAUDIO_LIBRARY}
	)

# Libraries used to build project binaries
set( LIBS eqOgre )

# Add Boost library dir because Equalizer uses relative paths to boost libs
# Linking to Equalizer will fail without it.
link_directories( ${vl_BINARY_DIR}/vl 
	${Boost_LIBRARY_DIRS} )

## Copy start script macro ##
MACRO( copy_start_script program_name )
message( "Copying start scripts for project = ${PROJECT_NAME} : exe name = ${program_name}" )

set( PROGRAM_NAME ${program_name} )
set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )

if( WIN32 )
	configure_file( ${VL_DATA_PATH}/start.bat
		${PROJECT_BINARY_DIR}/start.bat )
else()
	configure_file( ${VL_DATA_PATH}/start.sh
		${PROJECT_BINARY_DIR}/start.sh )
endif()

# Create log directory
file( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs )

ENDMACRO()


###### Copy Extra files MACRO #######
# TODO this can be removed when ProjectSettings scripts are in use
# and when we load audio files using Ogre resource system
MACRO( copy_extras )

set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )

file( COPY ${VL_DATA_PATH}/The_Dummy_Song.ogg DESTINATION ${PROJECT_BINARY_DIR}/ )
file( COPY ${VL_DATA_PATH}/script.py DESTINATION ${PROJECT_BINARY_DIR}/ )

ENDMACRO()



# include rendering engine, eq ogre directories
include_directories(vl)

# TODO should be configurable
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )
# TODO should be configurable
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/qtlauncher )
# TODO should be configurable
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )
# TODO should be configurable
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/demos )

