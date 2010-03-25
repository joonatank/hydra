project( "synchronization_test" CXX )

#SET( TEST_SOURCES eq_scene_node eq_scene_manager eq_root )

add_executable( sync_eq_scene_node
	sync_eq_scene_node.cpp
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node )

target_link_libraries( sync_eq_scene_node boost_unit_test_framework eq )
add_test( sync_eq_scene_node ${PROJECT_BINARY_DIR}/sync_eq_scene_node )
