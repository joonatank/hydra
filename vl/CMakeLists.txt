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

project( "eqOgre" CXX )

cmake_minimum_required(VERSION 2.6)

set( SOURCES
	eq_ogre/ogre_dotscene_loader.cpp
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
	distrib_resource_manager.cpp
	distrib_settings.cpp
	program_options.cpp
	stats.cpp
	base/filesystem.cpp
	base/envsettings.cpp
	base/projsettings.cpp
	eq_cluster/window.cpp
	eq_cluster/channel.cpp
	eq_cluster/config.cpp
	eq_cluster/pipe.cpp
	eq_cluster/application.cpp
	eq_ogre/ogre_root.cpp
	cluster/server.cpp
	cluster/client.cpp
	cluster/message.cpp
	)

set( HEADERS
	tracker.hpp
	eq_ogre/ogre_dotscene_loader.hpp
	tracker_serializer.hpp
	fake_tracker.hpp
	vrpn_tracker.hpp
	settings.hpp
	ogre_xml_helpers.hpp
	game_manager.hpp
	player.hpp
	stats.hpp
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
	typedefs.hpp
	resource.hpp
	resource_manager.hpp
	dotscene_loader.hpp
	action.hpp
	actions_misc.hpp
	actions_transform.hpp
	scene_manager.hpp
	scene_node.hpp
	python_module.hpp
	python.hpp
	trigger.hpp
	event_manager.hpp
	keycode.hpp
	session.hpp
	distrib_resource_manager.hpp
	distrib_settings.hpp
	program_options.hpp
	eq_cluster/window.hpp
	eq_cluster/channel.hpp
	eq_cluster/config.hpp
	eq_cluster/pipe.hpp
	eq_cluster/application.hpp
	eq_ogre/ogre_root.hpp
	math/conversion.hpp
	math/math.hpp
	cluster/server.hpp
	cluster/client.hpp
	cluster/message.hpp
	cluster/states.hpp
	distributed.hpp
	)

add_library( eqOgre ${SOURCES} ${HEADERS} )

target_link_libraries( eqOgre ${EQOGRE_DEB_LIBS} )

#install
# TODO this is here for test only the filestructure is not preserved by this
#install( FILES ${HEADERS} DESTINATION include )
