project("eq_cluster_test" CXX)

message("building eq_cluster_test")

SET( EQOGRE_SOURCES 
	vl/eq_cluster/channel.cpp vl/eq_cluster/config.cpp
	vl/eq_cluster/client.cpp vl/eq_cluster/node.cpp
	vl/eq_cluster/pipe.cpp vl/eq_cluster/window.cpp
	vl/eq_ogre/ogre_scene_manager.cpp
	vl/eq_ogre/ogre_scene_node.cpp vl/eq_ogre/ogre_entity.cpp
	vl/eq_graph/eq_scene_manager.cpp
	vl/eq_graph/eq_scene_node.cpp
	vl/eq_graph/eq_root.cpp
	)

FOREACH( source ${EQOGRE_SOURCES} )
	#set( SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/../../${source} )
	set( SOURCES ${SOURCES} ../../${source} )
ENDFOREACH( source )
	
add_executable( eqOgre main.cpp ${SOURCES} )
target_link_libraries( eqOgre OgreMain eq eqserver GLEW pthread )

