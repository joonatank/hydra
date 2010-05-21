project( "eq_ogre_test" CXX )

add_executable( test_ogre_root 
	test_ogre_root.cpp 
	../fixtures.hpp 
	)

add_executable( test_ogre_scene_node 
	test_ogre_scene_node.cpp 
	../fixtures.hpp 
	)

set( LIBS
	${Boost_LIBRARIES}
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre )

# Plugins file
if( WIN32 )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_REL} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${PROJECT_BINARY_DIR}/plugins.cfg )
	set( PLUGINS_DIR_DBG ${OGRE_PLUGIN_DIR_DBG} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins_d.cfg
		${PROJECT_BINARY_DIR}/plugins_d.cfg )
elseif( UNIX )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_TMP} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${PROJECT_BINARY_DIR}/plugins.cfg )
endif()

target_link_libraries( test_ogre_root ${LIBS} )
add_test( ogre_root ${PROJECT_BINARY_DIR}/test_ogre_root )

target_link_libraries( test_ogre_scene_node ${LIBS} )
add_test( ogre_scene_node ${PROJECT_BINARY_DIR}/test_ogre_scene_node )


