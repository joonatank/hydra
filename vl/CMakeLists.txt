project( "eqOgre" CXX )

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
	base/args.cpp
	base/filesystem.cpp
	base/envsettings.cpp
	base/projsettings.cpp
	eq_cluster/python.cpp
	eq_cluster/eq_resource_manager.cpp
	eq_cluster/dotscene_loader.cpp
	eq_cluster/channel.cpp
	eq_cluster/config.cpp
	eq_cluster/window.cpp
	eq_cluster/pipe.cpp
	eq_cluster/trigger.cpp
	eq_cluster/event_manager.cpp
	eq_cluster/transform_event.cpp
	eq_cluster/config_events.cpp
	eq_cluster/eq_settings.cpp
	eq_cluster/scene_manager.cpp
	eq_cluster/scene_node.cpp
	eq_cluster/client.cpp
	eq_ogre/ogre_root.cpp
	cluster/server.cpp
	cluster/client.cpp
	cluster/message.cpp
	udp/client.cpp
	udp/server.cpp
	udp/server_config.cpp
	udp/ogre_command.cpp
	udp/print_command.cpp
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
	base/system_util.hpp
	base/args.hpp
	base/envsettings.hpp
	base/projsettings.hpp
	base/exceptions.hpp
	base/filesystem.hpp
	base/print.hpp
	base/rapidxml.hpp
	base/rapidxml_print.hpp
	base/sleep.hpp
	base/string_utils.hpp
	base/typedefs.hpp
	resource.hpp
	resource_manager.hpp
	eq_cluster/python.hpp
	eq_cluster/eq_resource_manager.hpp
	eq_cluster/dotscene_loader.hpp
	eq_cluster/event_manager.hpp
	eq_cluster/trigger.hpp
	eq_cluster/action.hpp
	eq_cluster/channel.hpp
	eq_cluster/config.hpp
	eq_cluster/config_python.hpp
	eq_cluster/python.hpp
	eq_cluster/window.hpp
	eq_cluster/pipe.hpp
	eq_cluster/transform_event.hpp
	eq_cluster/config_events.hpp
	eq_cluster/eq_settings.hpp
	eq_cluster/scene_manager.hpp
	eq_cluster/scene_node.hpp
	eq_cluster/client.hpp
	eq_cluster/nodeFactory.hpp
	eq_ogre/ogre_root.hpp
	math/conversion.hpp
	math/math.hpp
	cluster/server.hpp
	cluster/client.hpp
	cluster/message.hpp
	distributed.hpp
	udp/client.hpp
	udp/server.hpp
	udp/server_config.hpp
	udp/command.hpp
	udp/ogre_command.hpp
	udp/print_command.hpp
	)

# This purposefully adds the glx window support for APPLE and CYGWIN also
if( UNIX )
	list( APPEND HEADERS
		eq_cluster/glxWindow.hpp
		)
	list( APPEND SOURCES
		eq_cluster/glxWindow.cpp
		)
elseif( WIN32 )
	list( APPEND HEADERS
		eq_cluster/wglWindow.hpp
		)
	list( APPEND SOURCES
		eq_cluster/wglWindow.cpp
		)
endif()

add_library( eqOgre ${SOURCES} ${HEADERS} )

target_link_libraries( eqOgre ${EQOGRE_DEB_LIBS} )

#install
# TODO this is here for test only the filestructure is not preserved by this
#install( FILES ${HEADERS} DESTINATION include )
