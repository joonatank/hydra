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

# Set the default install directory to be the build dir since we are using
# install just to gather all the necessary files in one place.
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)

# Add our local scripts to CMake path
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake )

# set the bin dir variable
# TODO these should be auto resolved
set(HYDRA_DEPENDENCIES_LIB_DIR "" CACHE PATH "Dependency library directory")

if(NOT HYDRA_DEPENDENCIES_LIB_DIR)
	message(FATAL_ERROR "Hydra dependencies lib dir needs to be defined.")
elseif(NOT EXISTS ${HYDRA_DEPENDENCIES_LIB_DIR})
	message(FATAL_ERROR "Hydra dependencies lib dir doesn't exist.")
endif()

set(ENV{VRPN_DIR} ${HYDRA_DEPENDENCIES_LIB_DIR}/VRPN)
set(ENV{Ogre_DIR} "${HYDRA_DEPENDENCIES_LIB_DIR}/Ogre/")
set(ENV{OIS_DIR} "${HYDRA_DEPENDENCIES_LIB_DIR}/OIS/")
set(BOOST_ROOT ${HYDRA_DEPENDENCIES_LIB_DIR})
set(BULLET_ROOT "${HYDRA_DEPENDENCIES_LIB_DIR}/Bullet/")
set(ENV{OgreProcedural_DIR} "${HYDRA_DEPENDENCIES_LIB_DIR}/OgreProcedural/")
set(ENV{OPENCOLLADA_DIR} "${HYDRA_DEPENDENCIES_LIB_DIR}/OpenCollada/")
set(ENV{TURTLE_DIR} "${HYDRA_DEPENDENCIES_LIB_DIR}/")

# TODO add Runtime lib definition to any find_package that has dlls.
# <module>_RUNTIME_LIBRARY_DIRS 

###### Find VRPN ########
find_package(VRPN REQUIRED)


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

# Python version 3.3 for Windows
# Linux has some problems with multiple pythons and boost installed from
# package manager, so we can not compile boost python to use Python 3
if(WIN32)
	set( PYTHON_VERSION 3.3 CACHE STRING "Python version" )
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

find_package(Bullet REQUIRED)

find_package(OgreProcedural REQUIRED)
find_package(OpenCollada REQUIRED)

# Oculus SDK should not be required but for the moment
find_package(OVR REQUIRED)

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
	${OPENCOLLADA_INCLUDE_DIR}
	${OPENCOLLADA_INCLUDE_DIR}/COLLADASaxFrameworkLoader
	${OPENCOLLADA_INCLUDE_DIR}/COLLADAFramework
	${OPENCOLLADA_INCLUDE_DIR}/COLLADABaseUtils
	${BULLET_INCLUDE_DIRS}
	${OVR_INCLUDE_DIR}
	# Necessary for some Ogre OpenGL hacks
	${CMAKE_SOURCE_DIR}/extern/glew
	${CMAKE_SOURCE_DIR}/rendering_system
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
	${BULLET_LIBRARIES}
	${OVR_LIBRARIES}
	# For OpenGL hacks needed for Oculus
	HydraGL
	)

# Find the Eye tracking package which is not required
# For now because I'm too lazy to implement non required, lets make it required
find_package(Eyes REQUIRED)
find_package(GLM REQUIRED)

include_directories(SYSTEM ${EYES_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
list(APPEND HYDRA_DEP_LIBS ${EYES_LIBRARIES})
# Add define so we can build the support for Eyes
add_definitions(-DUSE_EYES)

# Libraries used to build project binaries
set( HYDRA_LIBRARIES HydraMain )

# Add Boost library dir because some of our dependencies need it
link_directories( ${vl_BINARY_DIR}/vl ${Boost_LIBRARY_DIRS} )


include( CopyScripts )

set(HYDRA_BUILD_TESTS TRUE CACHE BOOL "Build the unit tests.")

# Find PCAN and add use flag if found
find_package(PCANBasic)
if(PCANBasic_FOUND)
	add_definitions(-DUSE_PCAN)
	include_directories(SYSTEM ${PCANBasic_INCLUDE_DIR})
	list(APPEND HYDRA_DEP_LIBS ${PCANBasic_LIBRARY})
endif()

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/extern )

# Adding include directory for Toast
include_directories(extern)

# include rendering engine, eq ogre directories
include_directories(vl)

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/vl )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/rendering_system )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/demos )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/daemon )
# Needs to be last since we need demos binary path for this
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/pyogre )

if(HYDRA_BUILD_TESTS)
	add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/test )
endif()

include( Packaging )

include( InstallResources )

include( InstallDependencies )

