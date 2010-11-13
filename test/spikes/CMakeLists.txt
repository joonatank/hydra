project( "spikes" CXX )

cmake_minimum_required(VERSION 2.8)

copy_config()

set( HEADERS
	../test_helpers.hpp
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
	../debug.hpp
	)

add_executable( test_arguments
	test_arguments.cpp
	${eqOgre_SOURCE_DIR}/arguments.hpp
	)

add_executable( test_env
	test_env.cpp
	)

# Compile the python embedding example

add_executable( python_embedded python_embedded.cpp )

target_link_libraries( python_embedded ${Boost_PYTHON_LIBRARIES} ${PYTHON_LIBRARIES} )

target_link_libraries( spike_listening_client ${LIBS} ${Boost_PYTHON_LIBRARIES} ${PYTHON_LIBRARIES}  )
target_link_libraries( spike_udp_renderer ${LIBS} )
target_link_libraries( fake_tracking_server ${VRPN_LIBRARY} )

