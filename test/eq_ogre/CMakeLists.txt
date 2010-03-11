SET( TEST_SOURCES ogre_root ogre_entity  )

add_executable( test_ogre_root
	test_ogre_root.cpp
	../../vl/eq_graph/eq_root
	../../vl/eq_ogre/ogre_root
	../../vl/eq_ogre/ogre_scene_manager
	../../vl/eq_ogre/ogre_scene_node
	../../vl/eq_ogre/ogre_entity
	../../vl/eq_graph/eq_scene_manager
	../../vl/eq_graph/eq_scene_node
	)

add_executable( test_ogre_entity
	test_ogre_entity
	../../vl/eq_ogre/ogre_entity.cpp )

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( test_${source} boost_unit_test_framework eq
		OgreMain )
ENDFOREACH( source )


