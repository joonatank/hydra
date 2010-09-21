project( "spikes" CXX )

cmake_minimum_required(VERSION 2.6)

copy_config()

set( HEADERS
	../test_helpers.hpp
	../debug.hpp
	)

add_executable( spike_listening_client
	listening_client.cpp
	${HEADERS}
	)

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	../tracking_fixture.hpp
	../debug.hpp
	)

set( LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	vrpn
	eqOgre
	)

target_link_libraries( spike_listening_client ${LIBS} )
target_link_libraries( fake_tracking_server vrpn )
