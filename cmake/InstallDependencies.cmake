#-------------------------------------------------------------------
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------
# TODO this should be tested on Windows
# FIXME will not work on Windows because Boost library will have both
# optimized and debug in it in Windows
# TODO this might need an option for enabling/disabling

# Copy Release versions of libs, only runtime libs are copied

# For now installing dependencies is only supported on Windows
# Shouldn't be a problem
if (NOT WIN32)
	message("Installing Dependencies is not supported on Linux")
else()
	message(STATUS "Install dependencies.")
endif()

# TODO clean the filepath and check it
set(Boost_BIN_DIR ${Boost_LIBRARY_DIRS}/../bin)
# We need Boost python from it nothing else

# Do not use the string utils for figuring out the dll from the lib
# because the cmake string tools suck really hard.

# sample: boost_python-vc100-mt-1_46_1.lib
# Construct the boost python library name
# MSVC version string for boost
if(NOT MSVC)
	message(FATAL_ERROR "Only MSVC compilers are supported for now")
elseif(MSVC_VERSION LESS 1400)
	message(FATAL_ERROR "Need at least MSVC 8")
elseif(MSVC_VERSION EQUAL 1400)
	set(VC_VERSION_STR "vc80")
elseif(MSVC_VERSION EQUAL 1500)
	set(VC_VERSION_STR "vc90")
elseif(MSVC_VERSION EQUAL 1600)
	set(VC_VERSION_STR "vc100")
elseif(MSVC_VERSION EQUAL 1700)
	set(VC_VERSION_STR "vc110")
endif()

set(Boost_PYTHON_BIN_NAME
	"boost_python-${VC_VERSION_STR}-mt-${Boost_LIB_VERSION}.dll")
set(Boost_PYTHON_BIN "${Boost_BIN_DIR}/${Boost_PYTHON_BIN_NAME}")

# Expat hack
set(EXPAT_BIN_RELEASE ${Boost_LIBRARY_DIRS}/../expat/bin/release/expat.dll)

set(LIBS
	# These doesn't work on Linux
	${Ogre_BINARY_RELEASE}
	${SkyX_BINARY_RELEASE}
	# Don't need this since we have HydraGL
	#${OGRE_RenderSystem_GL_BINARY_REL}
	${OIS_BINARY_RELEASE}
	${EXPAT_BIN_RELEASE}
	# TODO how to copy python?
	# ${PYTHON_LIBRARIES}
	${Boost_PYTHON_BIN}
	)

install( FILES ${LIBS} DESTINATION bin CONFIGURATIONS Release )

