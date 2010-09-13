project( "eqOgre" CXX )

set( SOURCES
	dotscene_loader.cpp
	fake_tracker.cpp
	vrpn_tracker.cpp
	settings.cpp
	base/args.cpp
	eq_cluster/channel.cpp
	eq_cluster/config.cpp
	eq_cluster/window.cpp
	eq_cluster/init_data.cpp
	eq_ogre/ogre_root.cpp
	udp/client.cpp
	udp/server.cpp
	udp/server_config.cpp
	udp/ogre_command.cpp
	udp/print_command.cpp
	)

set( HEADERS
	tracker.hpp
	dotscene_loader.hpp
	fake_tracker.hpp
	vrpn_tracker.hpp
	settings.hpp

	base/helpers.hpp
	base/args.hpp
	base/exceptions.hpp
	base/filesystem.hpp
	base/print.hpp
	base/rapidxml.hpp
	base/rapidxml_print.hpp
	base/sleep.hpp
	base/string_utils.hpp
	base/typedefs.hpp
	
	eq_cluster/channel.hpp
	eq_cluster/config.hpp
	eq_cluster/window.hpp
	eq_cluster/init_data.hpp

	eq_ogre/ogre_root.hpp

	math/conversion.hpp
	math/math.hpp

	udp/client.hpp
	udp/server.hpp
	udp/server_config.hpp
	udp/command.hpp
	udp/ogre_command.hpp
	udp/print_command.hpp
	)

add_library( eqOgre ${SOURCES} ${HEADERS} )

