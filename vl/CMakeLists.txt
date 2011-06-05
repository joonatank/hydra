#-------------------------------------------------------------------
# Joonatan Kuosa
# 2010-11
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "HydraMain" CXX )

cmake_minimum_required(VERSION 2.6)

set( SOURCES
	tracker.cpp
	vrpn_tracker.cpp
	tracker_serializer.cpp
	settings.cpp
	ogre_xml_helpers.cpp
	resource.cpp
	game_manager.cpp
	player.cpp
	dotscene_loader.cpp
	action.cpp
	actions_misc.cpp
	actions_transform.cpp
	scene_manager.cpp
	scene_node.cpp
	python.cpp
	trigger.cpp
	event_manager.cpp
	resource_manager.cpp
	distrib_settings.cpp
	program_options.cpp
	stats.cpp
	logger.cpp
	math/math.cpp
	math/transform.cpp
	base/filesystem.cpp
	base/envsettings.cpp
	base/projsettings.cpp
	base/timer.cpp
	eq_cluster/window.cpp
	eq_cluster/config.cpp
	renderer.cpp
	eq_cluster/application.cpp
	eq_ogre/ogre_root.cpp
	cluster/server.cpp
	cluster/client.cpp
	cluster/message.cpp
	physics/physics_events.cpp
	physics/physics_world.cpp
	physics/shapes.cpp
	physics/rigid_body.cpp
	entity.cpp
	light.cpp
	camera.cpp
	movable_object.cpp
	serializer.cpp
	mesh_serializer.cpp
	mesh_serializer_impl.cpp
	mesh_manager.cpp
	mesh.cpp
	mesh_ogre.cpp
	mesh_bullet.cpp
	gui/gui.cpp
	gui/window.cpp
	)

set( HEADERS
	system_tools/message_box.hpp
	tracker.hpp
	entity.hpp
	light.hpp
	movable_object.hpp
	serializer.hpp
	mesh_serializer.hpp
	mesh_serializer_impl.hpp
	mesh.hpp
	mesh_ogre.hpp
	mesh_bullet.hpp
	mesh_manager.hpp
	camera.hpp
	tracker_serializer.hpp
	vrpn_tracker.hpp
	settings.hpp
	ogre_xml_helpers.hpp
	game_manager.hpp
	player.hpp
	stats.hpp
	base/thread.hpp
	base/system_util.hpp
	base/envsettings.hpp
	base/projsettings.hpp
	base/exceptions.hpp
	base/filesystem.hpp
	base/print.hpp
	base/rapidxml.hpp
	base/rapidxml_print.hpp
	base/sleep.hpp
	base/string_utils.hpp
	base/timer.hpp
	base/report.hpp
	typedefs.hpp
	resource.hpp
	resource_manager.hpp
	dotscene_loader.hpp
	action.hpp
	actions_misc.hpp
	actions_transform.hpp
	scene_manager.hpp
	scene_node.hpp
	python.hpp
	python_module.hpp
	trigger.hpp
	event_manager.hpp
	keycode.hpp
	session.hpp
	logger.hpp
	distrib_settings.hpp
	program_options.hpp
	eq_cluster/window.hpp
	eq_cluster/config.hpp
	renderer.hpp
	renderer_interface.hpp
	eq_cluster/application.hpp
	eq_ogre/ogre_root.hpp
	math/conversion.hpp
	math/math.hpp
	math/transform.hpp
	cluster/server.hpp
	cluster/client.hpp
	cluster/message.hpp
	cluster/states.hpp
	distributed.hpp
	physics/motion_state.hpp
	physics/rigid_body.hpp
	physics/shapes.hpp
	physics/constraints.hpp
	physics/physics_events.hpp
	physics/physics_world.hpp
	gui/gui.hpp
	gui/window.hpp
	gui/gui_actions.hpp
	)

add_library(HydraMain ${SOURCES} ${HEADERS} )

target_link_libraries(HydraMain ${HYDRA_DEB_LIBS})

set_target_properties(HydraMain PROPERTIES DEBUG_POSTFIX _d)

#install
# TODO this is here for test only the filestructure is not preserved by this
#install( FILES ${HEADERS} DESTINATION include )
