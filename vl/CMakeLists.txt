PROJECT( "eqOgre" CXX )

set( SOURCES
	eq_ogre/ogre_root.cpp
	eq_ogre/ogre_root.hpp
	eq_ogre/ogre_scene_manager.cpp
	eq_ogre/ogre_scene_manager.hpp
	eq_ogre/ogre_scene_node.cpp
	eq_ogre/ogre_scene_node.hpp
	eq_ogre/ogre_entity.cpp
	eq_ogre/ogre_entity.hpp
	eq_ogre/ogre_camera.cpp
	eq_ogre/ogre_camera.hpp
	eq_ogre/ogre_light.hpp
	eq_ogre/ogre_light.cpp
	eq_graph/eq_root.cpp
	eq_graph/eq_root.hpp
	eq_graph/eq_scene_node.cpp
	eq_graph/eq_scene_node.hpp
	eq_graph/eq_scene_manager.cpp
	eq_graph/eq_scene_manager.hpp
	eq_graph/eq_scene_node.cpp
	eq_graph/eq_scene_node.hpp
	eq_graph/eq_entity.cpp
	eq_graph/eq_entity.hpp
	eq_graph/eq_camera.cpp
	eq_graph/eq_camera.hpp
	eq_graph/eq_light.hpp
	eq_graph/eq_light.cpp
	base/args.hpp
	base/args.cpp
	settings.cpp
	settings.hpp
	tracker.hpp
	vrpn_tracker.cpp
	vrpn_tracker.hpp
	base/exceptions.hpp
	dotscene_loader.cpp
	dotscene_loader.hpp
	)

add_library( eqOgre ${SOURCES} )
