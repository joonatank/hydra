project( "synchronization_test" CXX )

add_executable( sync_eq_scene_node sync_eq_scene_node.cpp )

set( LIBS
	${Boost_LIBRARIES}
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( sync_eq_scene_node ${LIBS} )
add_test( sync_eq_scene_node ${PROJECT_BINARY_DIR}/sync_eq_scene_node )
