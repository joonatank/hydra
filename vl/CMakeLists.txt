set( SOURCES
	eq_ogre/ogre_scene_node
	eq_ogre/ogre_scene_manager
	eq_ogre/ogre_entity.cpp
	eq_ogre/ogre_camera.cpp
	eq_ogre/ogre_root
	eq_graph/eq_root
	eq_graph/eq_scene_node
	eq_graph/eq_scene_manager
	eq_graph/eq_entity.cpp
	eq_graph/eq_entity.hpp
	eq_graph/eq_camera.cpp
	eq_graph/eq_light.hpp
	eq_graph/eq_light.cpp
	eq_graph/eq_camera.hpp
	base/args.hpp
	base/args.cpp
	settings.cpp
	settings.hpp
	dotscene_loader.cpp
	dotscene_loader.hpp
	)

add_library( eqOgre ${SOURCES} )
