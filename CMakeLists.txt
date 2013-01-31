#-------------------------------------------------------------------
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2010-11
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

# TODO should move to using QUIET package searching
# and add checking only after all packages are searched
# TODO static boost::python doesn't work

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
					 -bigobj		# We have lots of inlines for python
					 -DWIN_ZERO_SLEEP	# Rather do context switching than busy-wait
					 )
	# For debugging we want to enable symbols for both Debug and Release
	add_definitions(-Zi)
	# Add to linker /INCREMENTAL:NO /Zi /DEBUG /OPT:REF /OPT:ICF)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")

endif( UNIX )

# For Visual Studio
if(DEFINED CMAKE_CONFIGURATION_TYPES)
	# Don't compile debug version for 32-bit Windows
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
	else()
		set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
	endif()
endif()


# Add our local scripts to CMake path
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake )

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
find_package(OIS REQUIRED)


####### Find Boost ########

set(Boost_USE_MULTITHREADED ON)
if( WIN32 )
	# FIXME Linking statically causes runtime errors when mixing
	# pyogre module (which extends python) and vl module which is embedded
	#set(Boost_USE_STATIC_LIBS   ON)
	#add_definitions(-DBOOST_PYTHON_STATIC_LIB)
	set(Boost_USE_STATIC_LIBS   OFF)
else()
	set(Boost_USE_STATIC_LIBS   OFF)
endif()

find_package(Boost COMPONENTS python REQUIRED )

# Python version 3.2 for Windows
# Linux has some problems with multiple pythons and boost installed from
# package manager, so we can not compile boost python to use Python 3
if(WIN32)
	set( PYTHON_VERSION 3.2 CACHE STRING "Python version" )
else()
	set( PYTHON_VERSION 2.7 CACHE STRING "Python version" )
endif()

find_package(Python REQUIRED )

message(STATUS "Python library ${PYTHON_LIBRARY} : Python version ${PYTHON_VERSION} : "
	"Python include dir = ${PYTHON_INCLUDE_DIR}" )

if( WIN32 )
	set(Boost_USE_STATIC_LIBS   ON)
endif()

find_package( Boost COMPONENTS system filesystem program_options signals REQUIRED )

find_package(Bullet)
find_package(Newton)

find_package(OgreProcedural REQUIRED)
find_package(OpenCollada REQUIRED)

# TODO Sky plugisn should be optional and only one of them should be used
find_package(SkyX REQUIRED)
find_package(Caelum REQUIRED)

######### Add include and link directories ###########
include_directories( SYSTEM
 	${OpenGL_INCLUDE_DIR}
 	${Ogre_INCLUDE_DIR}
	# Necessary for OgreProcedurial
 	${Ogre_INCLUDE_DIR}/Ogre
	${OIS_INCLUDE_DIR}
	${VRPN_INCLUDE_DIR}
	${PYTHON_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${OgreProcedural_INCLUDE_DIR}
	${SkyX_INCLUDE_DIR}
	${Caelum_INCLUDE_DIR}
	${OPENCOLLADA_INCLUDE_DIR}
	${OPENCOLLADA_INCLUDE_DIR}/COLLADASaxFrameworkLoader
	${OPENCOLLADA_INCLUDE_DIR}/COLLADAFramework
	${OPENCOLLADA_INCLUDE_DIR}/COLLADABaseUtils
	)

# Libraries that Hydra library depends on
set( HYDRA_DEP_LIBS
	${OPENGL_LIBRARIES}
	${Ogre_LIBRARY}
	${VRPN_LIBRARY}
	${OIS_LIBRARY}
	${PYTHON_LIBRARY}
	${Boost_PYTHON_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARIES}
	${OgreProcedural_LIBRARIES}
	${OPENCOLLADA_LIBRARIES}
	${SkyX_LIBRARIES}
	${Caelum_LIBRARIES}
	)

# Find the Eye tracking package which is not required
find_package(Eyes QUIET)
find_package(GLM QUIET)
# Link if we found it
if(EYES_FOUND AND GLM_FOUND)
	message("Linking to eyes")
	include_directories(SYSTEM ${EYES_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
	list(APPEND HYDRA_DEP_LIBS ${EYES_LIBRARIES})
	# Add define so we can build the support for Eyes
	add_definitions(-DUSE_EYES)
endif()

# Libraries used to build project binaries
set( HYDRA_LIBRARIES HydraMain )

# Add Boost library dir because some of our dependencies need it
link_directories( ${vl_BINARY_DIR}/vl ${Boost_LIBRARY_DIRS} )


include( CopyScripts )

set(HYDRA_BUILD_TESTS TRUE CACHE BOOL "Build the unit tests.")
set(HYDRA_BUILD_DEMOS TRUE CACHE BOOL "Build the demos.")

set(HYDRA_PHYSICS "Bullet" CACHE STRING
	"Physics library to use: Bullet, Newton or None")


# TODO check that the physics library has been found
if(HYDRA_PHYSICS STREQUAL "Bullet")
	if(BULLET_FOUND)
		add_definitions(-DUSE_BULLET)
		include_directories(SYSTEM ${BULLET_INCLUDE_DIRS})
		list(APPEND HYDRA_DEP_LIBS ${BULLET_LIBRARIES})
	else()
		message(FATAL_ERROR "Requested to use Bullet for physics but didn't find the library.")
	endif()
elseif(HYDRA_PHYSICS STREQUAL "Newton")
	if(Newton_FOUND)
		add_definitions(-DUSE_NEWTON)
		list(APPEND HYDRA_DEP_LIBS ${Newton_LIBRARY})
		include_directories(SYSTEM ${Newton_INCLUDE_DIRS})
	else()
		message(FATAL_ERROR "Requested to use Newton for physics but didn't find the library.")
	endif()
endif()

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/extern )

# Adding include directory for Toast
include_directories(extern)

# include rendering engine, eq ogre directories
include_directories(vl)

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/pyogre )

if(HYDRA_BUILD_TESTS)
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )
endif()

if(HYDRA_BUILD_DEMOS)
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/demos )
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/daemon )
endif()

include( Packaging )

# TODO this doesn't work yet
#include( InstallResources )

# TODO this doesn't work yet
#include( InstallDependencies )
