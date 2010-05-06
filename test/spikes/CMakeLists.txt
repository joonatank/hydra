project( "spikes" CXX )

#SET( TEST_SOURCES stereo_render ogre_render pipes_test )

SET( HEADERS
	../fixtures.hpp 
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_scene_manager.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_scene_node.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_camera.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_entity.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_viewport.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_render_window.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_root.hpp
	${eqOgre_SOURCE_DIR}/eq_graph/eq_scene_manager.hpp
	${eqOgre_SOURCE_DIR}/eq_graph/eq_scene_node.hpp
	${eqOgre_SOURCE_DIR}/eq_graph/eq_root.hpp
	${eqOgre_SOURCE_DIR}/eq_graph/eq_entity.hpp
	${eqOgre_SOURCE_DIR}/eq_graph/eq_camera.hpp
	)
add_executable( ogre_render 
	ogre_render.cpp 
	${HEADERS}
	)

add_executable( stereo_render 
	stereo_render.cpp 
	${HEADERS}
	)

add_executable( pipes_test 
	pipes_test.cpp
	${HEADERS}
	)

add_executable( node_test 
	node_test.cpp 
	${HEADERS}
	)

add_executable( tracking 
	tracking.cpp 
	)

set( LIBS
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( ogre_render ${LIBS} )
target_link_libraries( stereo_render ${LIBS} )
target_link_libraries( pipes_test ${LIBS} )
target_link_libraries( node_test ${LIBS} )
target_link_libraries( tracking ${LIBS} vrpn )
