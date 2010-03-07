SET( TEST_SOURCES eq_scene_node )

FOREACH( source ${TEST_SOURCES} )
	add_executable( test_${source}
		test_${source}.cpp
		../../vl/eq_graph/${source} )
	target_link_libraries( test_${source} boost_unit_test_framework eq )
ENDFOREACH( source )


