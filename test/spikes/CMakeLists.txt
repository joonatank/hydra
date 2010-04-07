project( "spikes" CXX )

SET( TEST_SOURCES stereo_render ogre_render pipes_test )

add_executable( stereo_render 
	stereo_render.cpp
	)

add_executable( ogre_render 
	ogre_render.cpp
	)

add_executable( pipes_test
	pipes_test.cpp
	)

add_executable( node_test
	node_test.cpp
	)

set( LIBS ${OpenGL_LIBRARIES} ${OGRE_LIBRARIES} ${Boost_LIBRARIES}
	${EQ_LIBRARIES} )

message( "vl binary dir = ${vl_BINARY_DIR}" )

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( ${source} ${LIBS} eqOgre )
ENDFOREACH( source )

target_link_libraries( node_test ${OGRE_LIBRARIES} ${EQ_LIBRARIES} eqOgre )

