project("GLEW" C)

set(HEADERS GL/glew.h)

include_directories(SYSTEM ./)

if(WIN32)
	list(APPEND HEADERS GL/wglew.h)
else()
	list(APPEND HEADERS GL/glxew.h)
endif()

add_library( glew STATIC glew.c ${HEADERS} )

