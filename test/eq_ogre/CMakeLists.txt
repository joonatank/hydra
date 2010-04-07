project( "eq_ogre_test" CXX )

SET( TEST_SOURCES ogre_root ogre_scene_node  )

add_executable( test_ogre_root
	test_ogre_root.cpp
	)

add_executable( test_ogre_scene_node
	test_ogre_scene_node.cpp
	)

FOREACH( source ${TEST_SOURCES} )
	target_link_libraries( test_${source} ${Boost_LIBRARIES} 
		 ${OpenGL_LIBRARIES} ${OGRE_LIBRARIES} ${EQ_LIBRARIES} eqOgre )
ENDFOREACH( source )


