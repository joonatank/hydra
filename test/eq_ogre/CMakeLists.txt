project( "eq_ogre_test" CXX )

add_executable( test_ogre_root test_ogre_root.cpp )

add_executable( test_ogre_scene_node test_ogre_scene_node.cpp )

set( LIBS
	${Boost_LIBRARIES}
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre )

target_link_libraries( test_ogre_root ${LIBS} )
add_test( ogre_root ${PROJECT_BINARY_DIR}/test_ogre_root )

target_link_libraries( test_ogre_scene_node ${LIBS} )
add_test( ogre_scene_node ${PROJECT_BINARY_DIR}/test_ogre_scene_node )


