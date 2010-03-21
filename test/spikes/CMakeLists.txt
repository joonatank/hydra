project( "spikes" CXX )

SET( TEST_SOURCES stereo_render ogre_render pipes_test )

SET( COMMON_SOURCES 
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_node
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_manager
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_entity
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager
	)

add_executable( stereo_render 
	stereo_render.cpp
	${COMMON_SOURCES}
	)

add_executable( ogre_render 
	ogre_render.cpp
	${COMMON_SOURCES}
	)

add_executable( pipes_test
	pipes_test.cpp
	${COMMON_SOURCES}
	)

add_executable( node_test
	node_test.cpp
	${COMMON_SOURCES}
	)

target_link_libraries( node_test eq OgreMain )

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( ${source} boost_unit_test_framework eq
		OgreMain boost_thread )
ENDFOREACH( source )

