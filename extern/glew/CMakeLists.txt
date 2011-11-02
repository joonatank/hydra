# author: Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# date: 2011-10
# 
# This file is part of Hydra build system.
# This file is in public domain, feel free to use it as you see fit
#
# Build the GLEW library using cmake, to better integrate into our build system.

project("GLEW" C)

set(HEADERS GL/glew.h)

include_directories(./)

if(WIN32)
	list(APPEND HEADERS GL/wglew.h)
else()
	list(APPEND HEADERS GL/glxew.h)
endif()

add_library( glew STATIC glew.c ${HEADERS} )

