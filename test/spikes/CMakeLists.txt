project( "eq_graph_test" CXX )

SET( TEST_SOURCES stereo_render ogre_render )

add_executable( stereo_render 
	stereo_render.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_node
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_manager
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_entity
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager
	)

add_executable( ogre_render 
	ogre_render.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_node
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_manager
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_entity
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager
	)

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( ${source} boost_unit_test_framework eq
		OgreMain )
	message( "${PROJECT_BINARY_DIR}" )
ENDFOREACH( source )

#add_test( ${source} ${PROJECT_BINARY_DIR}/test_${source} )

