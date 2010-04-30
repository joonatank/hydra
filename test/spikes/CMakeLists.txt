project( "spikes" CXX )

#SET( TEST_SOURCES stereo_render ogre_render pipes_test )

add_executable( ogre_render ogre_render.cpp )
add_executable( stereo_render stereo_render.cpp )
add_executable( pipes_test pipes_test.cpp )
add_executable( node_test node_test.cpp )
add_executable( tracking tracking.cpp )

set( LIBS
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( ogre_render ${LIBS} )
target_link_libraries( stereo_render ${LIBS} )
target_link_libraries( pipes_test ${LIBS} )
target_link_libraries( node_test ${LIBS} )
target_link_libraries( tracking ${LIBS} vrpn )
