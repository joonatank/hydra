project( "scene_loader" CXX )

SET( SRCS
	BaseApplication.cpp
	SampleApp.cpp
	DotSceneLoader.cpp
	BaseApplication.h
	SampleApp.h
	DotSceneLoader.h
	)

include_directories( SYSTEM ${OIS_INCLUDE_DIRS}
	)

if(WIN32)
	file(GLOB SampleLoadApp_res ./res/*.rc)
endif(WIN32)

if( WIN32 )
add_executable( ogre_sceneloader WIN32 ${SRCS} ${SampleLoadApp_res} )
else( WIN32 )
add_executable( ogre_sceneloader ${SRCS} )
endif( WIN32 )

target_link_libraries( ogre_sceneloader
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${OIS_LIBRARIES}
	${Boost_THREAD_LIBRARY}
	${OGRE_Terrain_LIBRARY}
	)

