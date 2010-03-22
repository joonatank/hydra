project( "eq_graph_test" CXX )

SET( TEST_SOURCES eq_scene_node eq_scene_manager eq_root )

add_executable( test_eq_scene_node
	test_eq_scene_node.cpp
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node )

add_executable( test_eq_scene_manager
	test_eq_scene_manager.cpp
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node
	)

add_executable( test_eq_root
	test_eq_root.cpp
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager
	${vl_SOURCE_DIR}/vl/eq_graph/eq_root
	)

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( test_${source} ${Boost_LIBRARIES}
		${EQ_LIBRARIES} )
	add_test( ${source} ${PROJECT_BINARY_DIR}/test_${source} )
ENDFOREACH( source )


