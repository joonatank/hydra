project("vl" CXX)

cmake_minimum_required(VERSION 2.8)

# TODO add define rendering engine (Ogre/OpenSG)
# include vl lib
# Using include instead of subdirectory because with include we can just
# add to source/headers list and directories without building anything.
#include(vl/CMakeList.txt)

# TODO move to vl/CMakeList.txt)
include_directories(SYSTEM /usr/local/include/OGRE /usr/local/include/raknet)
link_directories(/usr/local/lib)

# COMPILE_FLAGS += 

# include rendering engine, eq ogre directories
include_directories(vl)

# include cnc lib inclde directories
include_directories(cnc)

# TODO add tests
SET( EQOGRE_SOURCES test/eq_cluster/main.cpp
	vl/eq_cluster/channel.cpp vl/eq_cluster/config.cpp
	vl/eq_cluster/client.cpp vl/eq_cluster/node.cpp
	vl/eq_cluster/pipe.cpp vl/eq_cluster/window.cpp
	vl/eq_ogre/ogre_scene_manager.cpp
	vl/eq_ogre/ogre_scene_node.cpp vl/eq_ogre/ogre_entity.cpp
	vl/eq_graph/eq_scene_manager.cpp
	vl/eq_graph/eq_scene_node.cpp
	vl/eq_graph/eq_root.cpp
	)
	
add_executable( eqOgre ${EQOGRE_SOURCES} )
target_link_libraries( eqOgre OgreMain eq eqserver GLEW pthread )

