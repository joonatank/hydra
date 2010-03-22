project( "eq_ogre_test" CXX )

SET( TEST_SOURCES ogre_root ogre_entity ogre_scene_node  )

add_executable( test_ogre_root
	test_ogre_root.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_root
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_manager
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_node
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_entity
	${vl_SOURCE_DIR}/vl/eq_graph/eq_root
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node
	)

add_executable( test_ogre_entity
	test_ogre_entity.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_entity.cpp )

add_executable( test_ogre_scene_node
	test_ogre_scene_node.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_node.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_scene_manager.cpp
	${vl_SOURCE_DIR}/vl/eq_ogre/ogre_entity.cpp
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_manager.cpp
	${vl_SOURCE_DIR}/vl/eq_graph/eq_scene_node.cpp
	)

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( test_${source} ${Boost_LIBRARIES} 
		 ${OpenGL_LIBRARIES} ${OGRE_LIBRARIES} ${EQ_LIBRARIES} )
ENDFOREACH( source )


