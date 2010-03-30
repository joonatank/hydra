project( "scene_loader" CXX )

FIND_PACKAGE(OIS REQUIRED)

IF(NOT OIS_FOUND)
    MESSAGE(SEND_ERROR "Failed to find OIS.")
ENDIF(NOT OIS_FOUND)

#message( "OIS include = ${OIS_INCLUDE_DIRS} " )
#message( "OIS libs = ${OIS_LIBRARIES} " )

SET(SRCS
	BaseApplication.cpp
	SampleApp.cpp
	DotSceneLoader.cpp
	BaseApplication.h
	SampleApp.h
	DotSceneLoader.h
	rapidxml.hpp
)

include_directories( SYSTEM ${OIS_INCLUDE_DIRS}
	)
#${OGRE_INCLUDE_DIRS}
#	${OGRE_Terrain_INCLUDE_DIR}
#	${Boost_INCLUDE_DIR}
#	${OGRE_SAMPLES_INCLUDEPATH}

if(WIN32)
	file(GLOB SampleLoadApp_res ./res/*.rc)
endif(WIN32)

if( WIN32 )
add_executable( sceneloader WIN32 ${SRCS} ${SampleLoadApp_res} )
else( WIN32 )
add_executable( sceneloader ${SRCS} )
endif( WIN32 )

target_link_libraries( sceneloader
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${OIS_LIBRARIES}
	${Boost_THREAD_LIBRARY}
	${OGRE_Terrain_LIBRARY}
	)

