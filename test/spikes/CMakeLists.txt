project( "spikes" CXX )

cmake_minimum_required(VERSION 2.8)

copy_config()

set( HEADERS
	../debug.hpp
	)

add_executable( spike_listening_client
	listening_client.cpp
	client_fixtures.hpp
	${HEADERS}
	)

add_executable( spike_udp_renderer
	udp_renderer.cpp
	client_fixtures.hpp
	${HEADERS}
	)

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	../tracking_fixture.hpp
	${HEADERS}
	)

target_link_libraries( spike_listening_client ${LIBS} )
target_link_libraries( spike_udp_renderer ${LIBS} )
target_link_libraries( fake_tracking_server ${VRPN_LIBRARY} )

