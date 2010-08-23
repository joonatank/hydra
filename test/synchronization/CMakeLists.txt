project( "synchronization_test" CXX )

set( HEADERS
	sync_fixture.hpp
	equalizer_fixture.hpp
	threaded_test_runner.hpp
	)

add_executable( sync_eq_scene_node
	sync_eq_scene_node.cpp
	${HEADERS}
	)

set( LIBS
	${Boost_LIBRARIES}
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${EQ_LIBRARIES}
	)

target_link_libraries( sync_eq_scene_node ${LIBS} )
add_test( sync_eq_scene_node ${PROJECT_BINARY_DIR}/sync_eq_scene_node )
